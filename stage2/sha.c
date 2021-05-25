/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain
Test Vectors (from FIPS PUB 180-1)
"abc"
  A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
A million repetitions of "a"
  34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

#include "types.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "sha.h"
#include "irq.h"
#include "memory.h"
#include "latte.h"

//should be divisible by four
#define BLOCKSIZE 32

#define SHA_CMD_FLAG_EXEC (1<<31)
#define SHA_CMD_FLAG_IRQ  (1<<30)
#define SHA_CMD_FLAG_ERR  (1<<29)
#define SHA_CMD_AREA_BLOCK ((1<<10) - 1)

static void sha_transform(u32 state[SHA_HASH_WORDS], u8 buffer[SHA_BLOCK_SIZE], u32 blocks)
{
    if(blocks == 0) return;

    /* Copy ctx->state[] to working vars */
    write32(SHA_H0, state[0]);
    write32(SHA_H1, state[1]);
    write32(SHA_H2, state[2]);
    write32(SHA_H3, state[3]);
    write32(SHA_H4, state[4]);

    // assign block to local copy which is 64-byte aligned
    u8 *block = memalign(64, SHA_BLOCK_SIZE * blocks);
    memcpy(block, buffer, SHA_BLOCK_SIZE * blocks);

    // royal flush :)
    dc_flushrange(block, SHA_BLOCK_SIZE * blocks);
    ahb_flush_to(RB_SHA);

    // tell sha1 controller the block source address
    write32(SHA_SRC, dma_addr(block));

    // tell sha1 controller number of blocks
    write32(SHA_CTRL, (read32(SHA_CTRL) & ~(SHA_CMD_AREA_BLOCK)) | (blocks - 1));

    // fire up hashing and wait till its finished
    write32(SHA_CTRL, read32(SHA_CTRL) | SHA_CMD_FLAG_EXEC);
    while (read32(SHA_CTRL) & SHA_CMD_FLAG_EXEC);

    // free the aligned data
    free(block);

    /* Add the working vars back into ctx.state[] */
    state[0] = read32(SHA_H0);
    state[1] = read32(SHA_H1);
    state[2] = read32(SHA_H2);
    state[3] = read32(SHA_H3);
    state[4] = read32(SHA_H4);
}

void sha_init(sha_ctx* ctx)
{
    memset(ctx, 0, sizeof(sha_ctx));

    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
}

void sha_update(sha_ctx* ctx, const void* inbuf, size_t size)
{
    unsigned int i, j;
    u8* data = (u8*)inbuf;

    j = (ctx->count[0] >> 3) & 63;
    if ((ctx->count[0] += size << 3) < (size << 3))
        ctx->count[1]++;
    ctx->count[1] += (size >> 29);
    if ((j + size) > 63) {
        memcpy(&ctx->buffer[j], data, (i = 64-j));
        sha_transform(ctx->state, ctx->buffer, 1);
        // try bigger blocks at once
        for ( ; i + 63 + ((BLOCKSIZE-1)*64) < size; i += (64 + (BLOCKSIZE-1)*64)) {
            sha_transform(ctx->state, &data[i], BLOCKSIZE);
        }
        for ( ; i + 63 + (((BLOCKSIZE/2)-1)*64) < size; i += (64 + ((BLOCKSIZE/2)-1)*64)) {
            sha_transform(ctx->state, &data[i], BLOCKSIZE/2);
        }
        for ( ; i + 63 + (((BLOCKSIZE/4)-1)*64) < size; i += (64 + ((BLOCKSIZE/4)-1)*64)) {
            sha_transform(ctx->state, &data[i], BLOCKSIZE/4);
        }
        for ( ; i + 63 < size; i += 64) {
            sha_transform(ctx->state, &data[i], 1);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&ctx->buffer[j], &data[i], size - i);
}

void sha_final(sha_ctx* ctx, void* outbuf)
{
    u8 final_count[8];
    u8* digest = outbuf;

    for (int i = 0; i < 8; i++) {
        final_count[i] = ((ctx->count[(i >= 4 ? 0 : 1)] >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
    }

    sha_update(ctx, "\200", 1);
    while ((ctx->count[0] & 504) != 448) {
        sha_update(ctx, "\0", 1);
    }
    sha_update(ctx, final_count, sizeof(final_count));  /* Should cause a sha_transform() */
    for (int i = 0; i < SHA_HASH_SIZE; i++) {
        digest[i] = ((ctx->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
    }

    /* Wipe variables */
    memset(ctx->buffer, 0, sizeof(ctx->buffer));
    memset(ctx->state, 0, sizeof(ctx->state));
    memset(ctx->count, 0, sizeof(ctx->count));
    memset(final_count, 0, sizeof(final_count));
    sha_transform(ctx->state, ctx->buffer, 1);
}

void sha_hash(const void* inbuf, void* outbuf, size_t size)
{
    sha_ctx ctx;

    sha_init(&ctx);
    sha_update(&ctx, inbuf, size);
    sha_final(&ctx, outbuf);
}
