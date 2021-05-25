/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef __TYPES_H__
#define __TYPES_H__

#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef volatile u8  vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;

typedef volatile s8  vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

#define NULL ((void *)0)

#define ALWAYS_INLINE __attribute__((always_inline))
#define SRAM_TEXT __attribute__((section(".sram.text")))
#define SRAM_DATA __attribute__((section(".sram.data")))
#define NORETURN __attribute__((__noreturn__))

#define ALIGNED(x) __attribute__((aligned(x)))
#define PACKED __attribute__((packed))

#define STACK_ALIGN(type, name, cnt, alignment)         \
    u8 _al__##name[((sizeof(type)*(cnt)) + (alignment) + \
    (((sizeof(type)*(cnt))%(alignment)) > 0 ? ((alignment) - \
    ((sizeof(type)*(cnt))%(alignment))) : 0))]; \
    type *name = (type*)(((u32)(_al__##name)) + ((alignment) - (( \
    (u32)(_al__##name))&((alignment)-1))))


#define INT_MAX ((s32)0x7fffffff)
#define UINT_MAX ((u32)0xffffffff)

//#define LONG_MAX INT_MAX
//#define ULONG_MAX UINT_MAX

//#define LLONG_MAX ((s64)0x7fffffffffffffff)
//#define ULLONG_MAX ((u64)0xffffffffffffffff)

#endif
