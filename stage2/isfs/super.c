/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "types.h"
#include "utils.h"
#include "nand.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "debug.h"

#include "isfs/isfs.h"
#include "isfs/volume.h"
#include "isfs/hmac_seed.h"
#include "isfs/super.h"
#include "isfs/isfshax.h"

int isfs_get_super_version(void* buffer)
{
    if(!memcmp(buffer, "SFFS", 4)) return 0;
    if(!memcmp(buffer, "SFS!", 4)) return 1;

    return -1;
}

u32 isfs_get_super_generation(void* buffer)
{
    return read32((u32)buffer + 4);
}

isfs_hdr* isfs_get_hdr(isfs_ctx* ctx)
{
    return (isfs_hdr*)&ctx->super[0];
}

u16* isfs_get_fat(isfs_ctx* ctx)
{
    return (u16*)&ctx->super[0x0C];
}

isfs_fst* isfs_get_fst(isfs_ctx* ctx)
{
    return (isfs_fst*)&ctx->super[0x10000 + 0x0C];
}

static isfs_fst* isfs_check_file(isfs_ctx* ctx, isfs_fst* fst, const char* path)
{
    char fst_name[sizeof(fst->name) + 1] = {0};
    memcpy(fst_name, fst->name, sizeof(fst->name));

    //ISFS_debug("file: %s vs %s\n", path, fst_name);

    if(!strcmp(fst_name, path))
        return fst;

    return NULL;
}

static isfs_fst* isfs_check_dir(isfs_ctx* ctx, isfs_fst* fst, const char* path)
{
    isfs_fst* root = isfs_get_fst(ctx);

    size_t size = strlen(path);
    const char* remaining = strchr(path, '/');
    if(remaining) size = remaining - path;

    if(size > sizeof(fst->name)) return NULL;

    char name[sizeof(fst->name) + 1] = {0};
    memcpy(name, path, size);

    char fst_name[sizeof(fst->name) + 1] = {0};
    memcpy(fst_name, fst->name, sizeof(fst->name));

    if(size == 0 || !strcmp(name, fst_name))
    {
        if(fst->sub != 0xFFFF && remaining != NULL && remaining[1] != '\0')
        {
            while(*remaining == '/') remaining++;
            return isfs_find_fst(ctx, &root[fst->sub], remaining);
        }

        return fst;
    }

    return NULL;
}

int isfs_fst_get_type(const isfs_fst* fst)
{
    return fst->mode & 3;
}

bool isfs_fst_is_file(const isfs_fst* fst)
{
    return isfs_fst_get_type(fst) == 1;
}

bool isfs_fst_is_dir(const isfs_fst* fst)
{
    return isfs_fst_get_type(fst) == 2;
}

isfs_fst* isfs_find_fst(isfs_ctx* ctx, isfs_fst* fst, const char* path)
{
    isfs_fst* root = isfs_get_fst(ctx);
    if(!fst) fst = root;

    if(fst->sib != 0xFFFF) {
        isfs_fst* result = isfs_find_fst(ctx, &root[fst->sib], path);
        if(result) return result;
    }

    switch(isfs_fst_get_type(fst)) {
        case 1:
            return isfs_check_file(ctx, fst, path);
        case 2:
            return isfs_check_dir(ctx, fst, path);
        default:
            ISFS_debug("Unknown mode! (%d)\n", isfs_fst_get_type(fst));
            break;
    }

    return NULL;
}

int isfs_super_check_slot(isfs_ctx *ctx, u32 index)
{
    u32 offs, cluster = CLUSTER_COUNT - (ctx->super_count - index) * ISFSSUPER_CLUSTERS;
    u16* fat = isfs_get_fat(ctx);

    for (offs = 0; offs < ISFSSUPER_CLUSTERS; offs++)
        if (fat[cluster + offs] != FAT_CLUSTER_RESERVED)
            return -1;

    return 0;
}

int isfs_super_mark_bad_slot(isfs_ctx *ctx, u32 index)
{
    u32 offs, cluster = CLUSTER_COUNT - (ctx->super_count - index) * ISFSSUPER_CLUSTERS;
    u16* fat = isfs_get_fat(ctx);

    for (offs = 0; offs < ISFSSUPER_CLUSTERS; offs++)
        fat[cluster + offs] = FAT_CLUSTER_BAD;

    return 0;
}

int isfs_read_super(isfs_ctx *ctx, void *super, int index)
{
    u32 cluster = CLUSTER_COUNT - (ctx->super_count - index) * ISFSSUPER_CLUSTERS;
    isfs_hmac_meta seed = { .cluster = cluster };
    return isfs_read_volume(ctx, cluster, ISFSSUPER_CLUSTERS, ISFSVOL_FLAG_HMAC, &seed, super);
}

int isfs_write_super(isfs_ctx *ctx, void *super, int index)
{
    u32 cluster = CLUSTER_COUNT - (ctx->super_count - index) * ISFSSUPER_CLUSTERS;
    isfs_hmac_meta seed = { .cluster = cluster };
    return isfs_write_volume(ctx, cluster, ISFSSUPER_CLUSTERS, ISFSVOL_FLAG_HMAC | ISFSVOL_FLAG_READBACK, &seed, super);
}

int isfs_find_super(isfs_ctx* ctx, u32 min_generation, u32 max_generation, u32 *generation, u32 *version)
{
    struct {
        int index;
        u32 generation;
        u8 version;
    } newest = {-1, 0, 0};

    for(int i = 0; i < ctx->super_count; i++)
    {
        u32 cluster = CLUSTER_COUNT - (ctx->super_count - i) * ISFSSUPER_CLUSTERS;

        if(isfs_read_volume(ctx, cluster, 1, 0, NULL, ctx->clbuf))
            continue;

        int cur_version = isfs_get_super_version(ctx->clbuf);
        if(cur_version < 0) continue;

        u32 cur_generation = isfs_get_super_generation(ctx->clbuf);
        if((cur_generation < newest.generation) ||
           (cur_generation < min_generation) ||
           (cur_generation >= max_generation))
            continue;

        newest.index = i;
        newest.generation = cur_generation;
        newest.version = cur_version;
    }

    if(newest.index == -1)
    {
        ISFS_debug("Failed to find super block.\n");
        return -3;
    }

    ISFS_debug("Found super block (device=%s, version=%u, index=%d, generation=0x%lX)\n",
            ctx->name, newest.version, newest.index, newest.generation);

    if(generation) *generation = newest.generation;
    if(version) *version = newest.version;
    return newest.index;
}

int isfs_load_super(isfs_ctx* ctx, u32 min_generation, u32 max_generation)
{
    ctx->generation = max_generation;

    while((ctx->index = isfs_find_super(ctx, min_generation, ctx->generation, &ctx->generation, &ctx->version)) >= 0)
        if(isfs_read_super(ctx, ctx->super, ctx->index) >= 0)
            break;

    return (ctx->index >= 0) ? 0 : -1;
}

int isfs_commit_super(isfs_ctx* ctx)
{
    isfs_get_hdr(ctx)->generation++;

    for(int i = 1; i <= ctx->super_count; i++)
    {
        u32 index = (ctx->index + i) & (ctx->super_count - 1);

        if (isfs_super_check_slot(ctx, index) < 0)
            continue;

        if (isfs_write_super(ctx, ctx->super, index) >= 0)
            return 0;

        isfs_super_mark_bad_slot(ctx, index);
        isfs_get_hdr(ctx)->generation++;
    }

    return -1;
}
