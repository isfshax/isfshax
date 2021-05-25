/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

.arm

.globl memcpy32
.globl memcpy16
.globl memcpy8
.globl memset32
.globl memset16
.globl memset8

.section .sram.text

memcpy32:
    bics    r2, #3
    bxeq    lr
1:  ldr     r3, [r1],#4
    str     r3, [r0],#4
    subs    r2, #4
    bne     1b
    bx      lr

memset32:
    bics    r2, #3
    bxeq    lr
1:  str     r1, [r0],#4
    subs    r2, #4
    bne     1b
    bx      lr

memcpy16:
    bics    r2, #1
    bxeq    lr
1:  ldrh    r3, [r1],#2
    strh    r3, [r0],#2
    subs    r2, #2
    bne     1b
    bx      lr

memset16:
    bics    r2, #1
    bxeq    lr
1:  strh    r1, [r0],#2
    subs    r2, #2
    bne     1b
    bx      lr

memcpy8:
    cmp     r2, #0
    bxeq    lr
1:  ldrb    r3, [r1],#1
    strb    r3, [r0],#1
    subs    r2, #1
    bne     1b
    bx      lr

memset8:
    cmp     r2, #0
    bxeq    lr
1:  strb    r1, [r0],#1
    subs    r2, #1
    bne     1b
    bx      lr
