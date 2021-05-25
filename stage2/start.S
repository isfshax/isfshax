/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

.arm

.extern _main
.extern __got_start
.extern __got_end
.extern __bss_start
.extern __bss_end
.extern __stack_addr
.globl _start

.extern v_irq

.section .init

_vectors:
_start:
    ldr pc, =v_reset
1:  b 1b
2:  b 2b
3:  b 3b
4:  b 4b
5:  b 5b
    ldr pc, =v_irq
6:  b 6b

.pool

v_reset:
    @ Switch to System mode
    msr cpsr_c, #0xdf

    @ Get loader base from ELF loader
    mov r4, r0

    @ Set up a stack
    ldr sp, =__stack_addr

    @ clear the stack to a marker value
    ldr r1, =__stack_end
    ldr r2, =__stack_addr
    ldr r3, =0xDEADBEEF
stk_loop:
    @ check for the end
    cmp r1, r2
    beq done_stk
    @ clear the word and move on
    str r3, [r1]
    add r1, r1, #4
    b   stk_loop

done_stk:
    @ clear BSS
    ldr r1, =__bss_start
    ldr r2, =__bss_end
    mov r3, #0
bss_loop:
    @ check for the end
    cmp r1, r2
    beq done_bss
    @ clear the word and move on
    str r3, [r1]
    add r1, r1, #4
    b   bss_loop

done_bss:
    @ take the plunge
    mov r0, r4
    ldr r1, =_main
    blx r1
    @ _main returned! Go to whatever address it returned...
    bx r0

.pool

