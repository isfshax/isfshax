#include "types.h"
#include "isfs/isfs.h"
#include "isfs/volume.h"
#include "nand.h"

#include "crypto.h"
#include "aes.h"
#include "hmac.h"
#include "string.h"

static u8 slc_super_buf[ISFSSUPER_SIZE] ALIGNED(NAND_DATA_ALIGN), slc_cluster_buf[CLUSTER_SIZE] ALIGNED(NAND_DATA_ALIGN);

isfs_ctx isfs[] = {
    [ISFSVOL_SLC]
    {
        .volume = ISFSVOL_SLC,
        .name = "slc",
        .bank = BANK_SLC,
        .key = &otp.nand_key,
        .hmac = &otp.nand_hmac,
        .super_count = 64,
        .super = slc_super_buf,
        .clbuf = slc_cluster_buf,
    },
};

int isfs_num_volumes(void)
{
    return sizeof(isfs) / sizeof(*isfs);
}

isfs_ctx* isfs_get_volume(int volume)
{
    if(volume < isfs_num_volumes() && volume >= 0)
        return &isfs[volume];

    return NULL;
}

char* isfs_do_volume(const char* path, isfs_ctx** ctx)
{
    isfs_ctx* volume = NULL;

    if(!path) return NULL;
    const char* filename = strchr(path, ':');

    if(!filename) return NULL;
    if(filename[1] != '/') return NULL;

    char mount[sizeof(volume->name)] = {0};
    memcpy(mount, path, filename - path);

    for(int i = 0; i < isfs_num_volumes(); i++)
    {
        volume = &isfs[i];
        if(strcmp(mount, volume->name)) continue;

        if(!volume->mounted) return NULL;

        *ctx = volume;
        return (char*)(filename + 1);
    }

    return NULL;
}

int isfs_read_volume(const isfs_ctx* ctx, u32 start_cluster, u32 cluster_count, u32 flags, void *hmac_seed, void *data)
{
    u8 saved_hmacs[2][20] = {0}, hmac[20] = {0};
    int rc = ISFSVOL_OK;
    u32 i, p;

    /* enable slc or slccmpt bank */
    nand_enable_banks(ctx->bank);

    /* read all requested clusters */
    for (i = 0; i < cluster_count; i++)
    {
        u32 cluster = start_cluster + i;
        u8 *cluster_data = (u8 *)data + i * CLUSTER_SIZE;
        u32 cluster_start = cluster * CLUSTER_PAGES;

        /* read cluster pages */
        for (p = 0; p < CLUSTER_PAGES; p++)
        {
            u8 spare[SPARE_SIZE] = {0};

            /* attempt to read the page (and correct ecc errors) */
            rc = nand_read_page(cluster_start + p, &cluster_data[p * PAGE_SIZE], spare);

            /* uncorrectable ecc error or other issues */
            if (rc < 0)
                return ISFSVOL_ERROR_READ;

            /* ECC errors, a refresh might be needed */
            if (rc > 0)
                rc = ISFSVOL_ECC_CORRECTED;

            /* page 6 and 7 store the hmac */
            if (p == 6)
            {
                memcpy(saved_hmacs[0], &spare[1], 20);
                memcpy(saved_hmacs[1], &spare[21], 12);
            }
            if (p == 7)
                memcpy(&saved_hmacs[1][12], &spare[1], 8);
        }

        /* decrypt cluster */
        if (flags & ISFSVOL_FLAG_ENCRYPTED)
        {
            aes_reset();
            aes_set_key(ctx->key);
            aes_empty_iv();
            aes_decrypt(cluster_data, cluster_data, CLUSTER_SIZE / AES_BLOCK_SIZE, 0);            
        }
    }

    /* verify hmac */
    if (flags & ISFSVOL_FLAG_HMAC)
    {
        hmac_ctx calc_hmac;
        int matched = 0;

        /* compute clusters hmac */
        hmac_init(&calc_hmac, ctx->hmac, 20);
        hmac_update(&calc_hmac, (const u8 *)hmac_seed, SHA_BLOCK_SIZE);
        hmac_update(&calc_hmac, (const u8 *)data, cluster_count * CLUSTER_SIZE);
        hmac_final(&calc_hmac, hmac);

        /* ensure at least one of the saved hmacs matches */
        matched += !memcmp(saved_hmacs[0], hmac, sizeof(hmac));
        matched += !memcmp(saved_hmacs[1], hmac, sizeof(hmac));

        if (matched == 2)
            rc = ISFSVOL_OK;
        else if (matched == 1)
            rc = ISFSVOL_HMAC_PARTIAL;
        else
            rc = ISFSVOL_ERROR_HMAC;
    }

    return rc;
}

int isfs_write_volume(const isfs_ctx* ctx, u32 start_cluster, u32 cluster_count, u32 flags, void *hmac_seed, void *data)
{
    static u8 blockpg[64][PAGE_SIZE] ALIGNED(NAND_DATA_ALIGN), blocksp[64][SPARE_SIZE];
    static u8 pgbuf[PAGE_SIZE] ALIGNED(NAND_DATA_ALIGN), spbuf[SPARE_SIZE];
    u8 hmac[20] = {0};
    int rc = ISFSVOL_OK;
    u32 b, p;

    /* enable slc or slccmpt bank */
    nand_enable_banks(ctx->bank);

    /* compute clusters hmac */
    if (flags & ISFSVOL_FLAG_HMAC)
    {
        hmac_ctx calc_hmac;
        hmac_init(&calc_hmac, ctx->hmac, 20);
        hmac_update(&calc_hmac, (const u8 *)hmac_seed, SHA_BLOCK_SIZE);
        hmac_update(&calc_hmac, (const u8 *)data, cluster_count * CLUSTER_SIZE);
        hmac_final(&calc_hmac, hmac);
    }

    /* setup clusters encryption */
    if (flags & ISFSVOL_FLAG_ENCRYPTED)
    {
        aes_reset();
        aes_set_key(ctx->key);
        aes_empty_iv();
    }

    u32 startpage = start_cluster * CLUSTER_PAGES;
    u32 endpage = (start_cluster + cluster_count) * CLUSTER_PAGES;

    u32 startblock = start_cluster / BLOCK_CLUSTERS;
    u32 endblock = (start_cluster + cluster_count + BLOCK_CLUSTERS - 1) / BLOCK_CLUSTERS;

    /* process data in nand blocks */
    for (b = startblock; (b < endblock) && (rc >= 0); b++)
    {
        u32 firstblockpage = b * BLOCK_PAGES;

        /* prepare block */
        for (p = 0; p < 64; p++)
        {
            u32 curpage = firstblockpage + p;       /* current page */
            u32 clusidx = curpage % CLUSTER_PAGES;  /* index in cluster */

            /* if this page is unmodified, read it from nand */
            if ((curpage < startpage) || (curpage >= endpage))
            {
                if (nand_read_page(curpage, blockpg[p], blocksp[p]) < 0)
                    return ISFSVOL_ERROR_READ;
                continue;
            }

            /* place hmac in page 6 and 7 of a cluster */
            memset(blocksp[p], 0, SPARE_SIZE);
            switch (clusidx)
            {
            case 6:
                memcpy(&blocksp[p][1], hmac, 20);
                memcpy(&blocksp[p][21], hmac, 12);
                break;
            case 7:
                memcpy(&blocksp[p][1], &hmac[12], 8);
                break;
            }

            /* encrypt or copy the data */
            u8 *srcdata = (u8*)data + (curpage - startpage) * PAGE_SIZE;
            if (flags & ISFSVOL_FLAG_ENCRYPTED)
                aes_encrypt(blockpg[p], srcdata, PAGE_SIZE / AES_BLOCK_SIZE, clusidx > 0);
            else
                memcpy(blockpg[p], srcdata, PAGE_SIZE);
        }

        /* erase block */
        if (nand_erase_block(b) < 0)
            return ISFSVOL_ERROR_ERASE;

        /* write block */
        for (p = 0; p < BLOCK_PAGES; p++)
            if (nand_write_page(firstblockpage + p, blockpg[p], blocksp[p]) < 0)
                rc = ISFSVOL_ERROR_WRITE;

        /* check if pages should be verified after writing */
        if (rc || !(flags & ISFSVOL_FLAG_READBACK))
            continue;

        /* read back pages */
        for (p = 0; p < BLOCK_PAGES; p++)
        {
            if (nand_read_page(firstblockpage + p, pgbuf, spbuf) < 0)
                return ISFSVOL_ERROR_READ;

            /* page content doesn't match */
            if (memcmp(blockpg[p], pgbuf, PAGE_SIZE) ||
                memcmp(&blocksp[p][1], &spbuf[1], 0x20))
                return ISFSVOL_ERROR_READBACK;
        }
    }

    return rc;
}
