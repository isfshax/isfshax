/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef _ANCAST_H
#define _ANCAST_H

#include "types.h"
#include "sha.h"

#define ANCAST_MAGIC (0xEFA282D9l)
#define ANCAST_TARGET_IOP (0x02)
#define ANCAST_ADDRESS_IOP (0x01000000)

typedef struct {
    u16 unk1;
    u8 unk2;
    u8 unk3;
    u32 device;
    u32 type;
    u32 body_size;
    u32 body_hash[SHA_HASH_WORDS];
    u32 version;
    u8 padding[0x38];
} ancast_header;

typedef struct {
    u32 header_size;
    u32 loader_size;
    u32 elf_size;
    u32 ddr_init;
} ios_header;

u32 ancast_iop_load(u8* buffer, size_t size);

#endif
