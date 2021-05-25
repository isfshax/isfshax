/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef _SHA_H
#define _SHA_H

#include "types.h"

#define SHA_BLOCK_BITS    (0x200) // 512
#define SHA_BLOCK_SIZE    (SHA_BLOCK_BITS / 8) // 64
#define SHA_BLOCK_WORDS   (SHA_BLOCK_SIZE / sizeof(u32)) // 16

#define SHA_HASH_BITS     (0xA0) // 160
#define SHA_HASH_SIZE     (SHA_HASH_BITS / 8) // 20 (0x14)
#define SHA_HASH_WORDS    (SHA_HASH_SIZE / sizeof(u32)) // 5

typedef struct {
    u32 state[SHA_HASH_WORDS];
    u32 count[2];
    u8 buffer[SHA_BLOCK_SIZE];
} sha_ctx;

void sha_init(sha_ctx* ctx);
void sha_update(sha_ctx* ctx, const void* inbuf, size_t size);
void sha_final(sha_ctx* ctx, void* outbuf);

void sha_hash(const void* inbuf, void* outbuf, size_t size);

#endif
