/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "types.h"

#define ALIGN_FORWARD(x,align) \
    ((__typeof__(x))((((u32)(x)) + (align) - 1) & (~(align-1))))

#define ALIGN_BACKWARD(x,align) \
    ((__typeof__(x))(((u32)(x)) & (~(align-1))))

enum rb_client {
    RB_IOD = 0,
    RB_IOI = 1,
    RB_AIM = 2,
    RB_FLA = 3,
    RB_AES = 4,
    RB_SHA = 5,
    RB_EHCI = 6,
    RB_OHCI0 = 7,
    RB_OHCI1 = 8,
    RB_SD0 = 9,
    RB_SD1 = 10,
    RB_SD2 = 11,
    RB_SD3 = 12,
    RB_EHC1 = 13,
    RB_OHCI10 = 14,
    RB_EHC2 = 15,
    RB_OHCI20 = 16,
    RB_SATA = 17,
    RB_AESS = 18,
    RB_SHAS = 19
};

enum wb_client {
    WB_IOD = 0,
    WB_AIM = 1,
    WB_FLA = 2,
    WB_AES = 3,
    WB_SHA = 4,
    WB_EHCI = 5,
    WB_OHCI0 = 6,
    WB_OHCI1 = 7,
    WB_SD0 = 8,
    WB_SD1 = 9,
    WB_SD2 = 10,
    WB_SD3 = 11,
    WB_EHC1 = 12,
    WB_OHCI10 = 13,
    WB_EHC2 = 14,
    WB_OHCI20 = 15,
    WB_SATA = 16,
    WB_AESS = 17,
    WB_SHAS = 18,
    WB_DMAA = 19,
    WB_DMAB = 20,
    WB_DMAC = 21,
    WB_ALL = 22
};

void dc_flushrange(const void *start, u32 size);
void dc_invalidaterange(void *start, u32 size);
void dc_flushall(void);
void ic_invalidateall(void);
void ahb_flush_from(enum wb_client dev);
void ahb_flush_to(enum rb_client dev);
void mem_protect(int enable, void *start, void *end);
void mem_setswap(int enable);

void mem_initialize(void);
void mem_shutdown(void);

u32 dma_addr(void *);

static inline u32 get_cr(void)
{
    u32 data;
    __asm__ volatile ( "mrc\tp15, 0, %0, c1, c0, 0" : "=r" (data) );
    return data;
}

static inline u32 get_ttbr(void)
{
    u32 data;
    __asm__ volatile ( "mrc\tp15, 0, %0, c2, c0, 0" : "=r" (data) );
    return data;
}

static inline u32 get_dacr(void)
{
    u32 data;
    __asm__ volatile ( "mrc\tp15, 0, %0, c3, c0, 0" : "=r" (data) );
    return data;
}

static inline void set_cr(u32 data)
{
    __asm__ volatile ( "mcr\tp15, 0, %0, c1, c0, 0" :: "r" (data) );
}

static inline void set_ttbr(u32 data)
{
    __asm__ volatile ( "mcr\tp15, 0, %0, c2, c0, 0" :: "r" (data) );
}

static inline void set_dacr(u32 data)
{
    __asm__ volatile ( "mcr\tp15, 0, %0, c3, c0, 0" :: "r" (data) );
}

static inline u32 get_dfsr(void)
{
    u32 data;
    __asm__ volatile ( "mrc\tp15, 0, %0, c5, c0, 0" : "=r" (data) );
    return data;
}

static inline u32 get_ifsr(void)
{
    u32 data;
    __asm__ volatile ( "mrc\tp15, 0, %0, c5, c0, 1" : "=r" (data) );
    return data;
}

static inline u32 get_far(void)
{
    u32 data;
    __asm__ volatile ( "mrc\tp15, 0, %0, c6, c0, 0" : "=r" (data) );
    return data;
}

void _ahb_flush_to(enum rb_client dev);

static inline void dc_inval_block_fast(void *block)
{
    __asm__ volatile ( "mcr\tp15, 0, %0, c7, c6, 1" :: "r" (block) );
    _ahb_flush_to(RB_IOD); //TODO: check if really needed and if not, remove
}

static inline void dc_flush_block_fast(void *block)
{
    __asm__ volatile ( "mcr\tp15, 0, %0, c7, c10, 1" :: "r" (block) );
    __asm__ volatile ( "mcr\tp15, 0, %0, c7, c10, 4" :: "r" (0) );
    ahb_flush_from(WB_AIM); //TODO: check if really needed and if not, remove
}

#endif
