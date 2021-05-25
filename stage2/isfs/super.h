#pragma once
#include "isfs/isfs.h"
#include "nand.h"

#define ISFSSUPER_CLUSTERS  0x10
#define ISFSSUPER_SIZE      (ISFSSUPER_CLUSTERS * CLUSTER_SIZE)

typedef struct isfs_fst {
    char name[12];
    u8 mode;
    u8 attr;
    u16 sub;
    u16 sib;
    u32 size;
    u16 x1;
    u16 uid;
    u16 gid;
    u32 x3;
} PACKED isfs_fst;
_Static_assert(sizeof(isfs_fst) == 0x20, "isfs_fst size must be 0x20!");

typedef struct isfs_hdr {
    char magic[4];
    u32 generation;
    u32 x1;
} PACKED isfs_hdr;
_Static_assert(sizeof(isfs_hdr) == 0xC, "isfs_hdr size must be 0xC!");

typedef struct isfs_super {
    isfs_hdr hdr;
    u16 fat[CLUSTER_COUNT];
    isfs_fst fst[6143];
    u8 pad[20];
} PACKED ALIGNED(64) isfs_super;
_Static_assert(sizeof(isfs_super) == ISFSSUPER_SIZE, "isfs_super must be 0x40000");

#define FAT_CLUSTER_LAST        0xFFFB // last cluster within a chain
#define FAT_CLUSTER_RESERVED    0xFFFC // reserved cluster
#define FAT_CLUSTER_BAD         0xFFFD // bad block (marked at factory)
#define FAT_CLUSTER_EMPTY       0xFFFE // empty (unused / available) space

typedef struct isfs_ctx isfs_ctx;

int isfs_get_super_version(void* buffer);
u32 isfs_get_super_generation(void* buffer);

isfs_hdr* isfs_get_hdr(isfs_ctx* ctx);
u16* isfs_get_fat(isfs_ctx* ctx);
isfs_fst* isfs_get_fst(isfs_ctx* ctx);

void isfs_print_fst(isfs_fst* fst);

int isfs_fst_get_type(const isfs_fst* fst);
bool isfs_fst_is_file(const isfs_fst* fst);
bool isfs_fst_is_dir(const isfs_fst* fst);
isfs_fst* isfs_find_fst(isfs_ctx* ctx, isfs_fst* fst, const char* path);

int isfs_super_check_slot(isfs_ctx *ctx, u32 index);
int isfs_super_mark_bad_slot(isfs_ctx *ctx, u32 index);

int isfs_read_super(isfs_ctx *ctx, void *super, int index);
int isfs_write_super(isfs_ctx *ctx, void *super, int index);

int isfs_find_super(isfs_ctx* ctx, u32 min_generation, u32 max_generation, u32 *generation, u32 *version);
int isfs_load_super(isfs_ctx* ctx, u32 min_generation, u32 max_generation);
int isfs_commit_super(isfs_ctx* ctx);
