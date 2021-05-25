/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

.arm

.globl _dc_inval_entries
.globl _dc_flush_entries
.globl _dc_flush
.globl _dc_inval
.globl _ic_inval
.globl _drain_write_buffer
.globl _tlb_inval

.section .sram.text

_dc_inval_entries:
    mcr     p15, 0, r0, c7, c6, 1
    add     r0, #0x20
    subs    r1, #1
    bne     _dc_inval_entries
    bx      lr

_dc_flush_entries:
    mcr     p15, 0, r0, c7, c10, 1
    add     r0, #0x20
    subs    r1, #1
    bne     _dc_flush_entries
    bx      lr

_dc_flush:
    mrc     p15, 0, pc, c7, c10, 3
    bne     _dc_flush
    bx      lr

_dc_inval:
    mov     r0, #0
    mcr     p15, 0, r0, c7, c6, 0
    bx      lr

_ic_inval:
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 0
    bx      lr

_drain_write_buffer:
    mov     r0, #0
    mcr     p15, 0, r0, c7, c10, 4
    bx      lr

_tlb_inval:
    mov     r0, #0
    mcr     p15, 0, r0, c8, c7
    bx      lr
