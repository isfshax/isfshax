/*
 * isfshax.h
 *
 * Copyright (C) 2021          rw-r-r-0644 <rwrr0644@gmail.com>
 *
 * This code is licensed to you under the terms of the GNU GPL, version 2;
 * see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */
#pragma once
#include "types.h"
#include "nand.h"
#include "isfs/isfs.h"

#define ISFSHAX_MAGIC               0x48415858

#define ISFSHAX_REDUNDANCY          (1 << 2)

#define ISFSHAX_GENERATION_FIRST    0xffff7fff
#define ISFSHAX_GENERATION_RANGE    0x100

#define ISFSHAX_BAD_SLOT            0x80

typedef struct isfshax_info
{
    u32 magic;
    u8 slots[ISFSHAX_REDUNDANCY];
    u32 generation;
    u32 generationbase;
    u32 index;
} isfshax_info;
_Static_assert(sizeof(isfshax_info) == 0x14, "isfshax_info must be 0x14");

typedef struct isfshax_super
{
    char magic[4];
    u32 generation;
    u32 x1;
    u16 fat[CLUSTER_COUNT];
    isfs_fst fst[6143];
    isfshax_info isfshax;
} PACKED ALIGNED(64) isfshax_super;

_Static_assert(sizeof(isfshax_super) == ISFSSUPER_SIZE, "isfshax_super must be 0x40000");


int isfshax_refresh();

