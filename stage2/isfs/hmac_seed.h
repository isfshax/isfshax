#pragma once
#include "types.h"
#include "sha.h"
#include "hmac.h"

typedef struct {
    u16 x1;
    u16 uid;
    char name[0x0C];
    u32 iblk;
    u32 ifst;
    u32 x3;
    u8 pad0[0x24];
} isfs_hmac_data;
_Static_assert(sizeof(isfs_hmac_data) == 0x40, "isfs_hmac_data size must be 0x40!");

typedef struct {
    u8 pad0[0x12];
    u16 cluster;
    u8 pad1[0x2b];
} isfs_hmac_meta;
_Static_assert(sizeof(isfs_hmac_meta) == 0x40, "isfs_hmac_meta size must be 0x40!");
