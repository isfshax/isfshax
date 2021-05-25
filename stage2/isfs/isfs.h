/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef _ISFS_H
#define _ISFS_H

#include "types.h"
#include <sys/iosupport.h>
#include <stdio.h>
#include "super.h"

typedef struct isfs_ctx {
    int volume;
    const char name[0x10];
    const u32 bank;
    const u32 super_count;
    int index;
    u8* const super;
    u32 generation;
    u32 version;
    bool mounted;
    void* key;
    void* hmac;
    devoptab_t devoptab;
} isfs_ctx;

typedef struct isfs_file {
    int volume;
    isfs_fst* fst;
    size_t offset;
    u16 cluster;
} isfs_file;

typedef struct isfs_dir {
    int volume;
    isfs_fst* dir;
    isfs_fst* child;
} isfs_dir;

int isfs_init(void);
int isfs_fini(void);

isfs_fst* isfs_stat(const char* path);

int isfs_open(isfs_file* file, const char* path);
int isfs_close(isfs_file* file);

int isfs_seek(isfs_file* file, s32 offset, int whence);
int isfs_read(isfs_file* file, void* buffer, size_t size, size_t* bytes_read);

int isfs_diropen(isfs_dir* dir, const char* path);
int isfs_dirread(isfs_dir* dir, isfs_fst** info);
int isfs_dirreset(isfs_dir* dir);
int isfs_dirclose(isfs_dir* dir);

#ifdef ISFS_DEBUG
#   define  ISFS_debug(f, arg...) DEBUG("ISFS: " f, ##arg);
#else
#   define  ISFS_debug(f, arg...)
#endif

#endif
