#pragma once
#include "isfs/isfs.h"

#define ISFSVOL_SLC             0
#define ISFSVOL_SLCCMPT         1

#define ISFSVOL_FLAG_HMAC       1
#define ISFSVOL_FLAG_ENCRYPTED  2
#define ISFSVOL_FLAG_READBACK   4

#define ISFSVOL_OK              0
#define ISFSVOL_ECC_CORRECTED   0x10
#define ISFSVOL_HMAC_PARTIAL    0x20
#define ISFSVOL_ERROR_WRITE     -0x10
#define ISFSVOL_ERROR_READ      -0x20
#define ISFSVOL_ERROR_ERASE     -0x30
#define ISFSVOL_ERROR_HMAC      -0x40
#define ISFSVOL_ERROR_READBACK  -0x50

int isfs_num_volumes(void);
isfs_ctx* isfs_get_volume(int volume);
char* isfs_do_volume(const char* path, isfs_ctx** ctx);

int isfs_read_volume(const isfs_ctx* ctx, u32 start_cluster, u32 cluster_count, u32 flags, void *hmac_seed, void *data);
int isfs_write_volume(const isfs_ctx* ctx, u32 start_cluster, u32 cluster_count, u32 flags, void *hmac_seed, void *data);
