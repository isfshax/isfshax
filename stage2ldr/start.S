/*
    mini - a Free Software replacement for the Nintendo/BroadOn IOS.

    ELF loader: system startup

Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 2.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
.arm

.extern _main
.extern __got_start
.extern __got_end
.extern __stack_addr
.extern delay
.globl _start

.section .init

_start:
    @ Get real address of _start
    sub r4, pc, #8
    @ Subtract offset to get the address that we were loaded at
    ldr r0, =_start
    sub r4, r4, r0

    @ Set up a stack
    ldr sp, =__stack_addr
    add sp, r4

    @ relocate the GOT entries
    ldr r1, =__got_start
    add r1, r4
    ldr r2, =__got_end
    add r2, r4
got_loop:
    @ check for the end
    cmp r1, r2
    beq done_got
    @ read the GOT entry
    ldr r3, [r1]
    @ add our base address
    add r3, r4
    str r3, [r1]
    @ move on
    add r1, r1, #4
    b   got_loop

done_got:

    @ take the plunge
    mov r0, r4
    bl  _main
    @ _main returned! Go to whatever address it returned...
    mov r1, r0
    mov r0, r4
    mov pc, r1

.pool
