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
#include "memory.h"

#include "debug.h"
#include <string.h>
#include <sys/errno.h>
#include <stdio.h>

#include "sha.h"
#include "crypto.h"
#include "smc.h"

#include "ancast.h"


u32 ancast_iop_load(u8* buffer, size_t size)
{
    u32 magic = read32((u32) buffer);
    if(magic != ANCAST_MAGIC) {
        DEBUG("ancast_iop_load: not an ancast image (magic is 0x%08lX, expected 0x%08lX).\n", magic, ANCAST_MAGIC);
        return 0;
    }

    u32 sig_offset = read32((u32) &buffer[0x08]);
    u32 sig_type = read32((u32) &buffer[sig_offset]);

    if (sig_type != 0x02) {
        DEBUG("ancast_iop_load: unexpected signature type 0x%02lX.\n", sig_type);
        return 0;
    }

    ancast_header* ancast = (ancast_header*)&buffer[0x1A0];
    u8 target = ancast->device >> 4;

    if(target != ANCAST_TARGET_IOP) {
        DEBUG("ancast: not an IOP image (target is 0x%02X, expected 0x%02X).\n", target, ANCAST_TARGET_IOP);
        return 0;
    }

    u8* body = (u8*)(ancast + 1);

    u32 hash[SHA_HASH_WORDS] = {0};
    sha_hash(body, hash, ancast->body_size);

    u32* h1 = ancast->body_hash;
    u32* h2 = hash;
    if(memcmp(h1, h2, SHA_HASH_SIZE) != 0) {
        DEBUG("ancast: body hash check failed.\n");
        DEBUG("        expected:   %08lX%08lX%08lX%08lX%08lX\n", h1[0], h1[1], h1[2], h1[3], h1[4]);
        DEBUG("        calculated: %08lX%08lX%08lX%08lX%08lX\n", h2[0], h2[1], h2[2], h2[3], h2[4]);
        return 0;
    }

    ios_header* header = (ios_header*)body;
    u32 vector = (u32) &body[header->header_size];

    return vector;
}
