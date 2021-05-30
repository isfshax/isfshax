/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
*	Copyright (C) 2021          Roberto Van Eeden <rwrr0644@gmail.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifdef LOLSERIAL_DEBUG

#include "latte.h"

.arm

.globl lolserial_print
.globl lolserial_lprint

.section .text

/*
 * the number of timer ticks to wait
 * each bit; might need manual adjustment
 * for particoular serial adapters, usually
 * the stable value is in the +5/-5 range
 */

.equ LOLSERIAL_WAIT_TICKS, 200
.equ GP_SENSORBAR, 0x00000100

@ arg:r0 string address
lolserial_print:
    mov r1, #-1

@ arg:r0 string address
@ arg:r1 max length
lolserial_lprint:
    push {r4-r5}
    add r1, r0                                          @ end address in r1
    ldr r5, =LT_REG_BASE                                @ latte registers base in r5

    @ clear32(LT_GPIO_OWNER, GP_SENSORBAR)
    ldr r2, [r5, #0x0FC]
    bic r2, r2, #GP_SENSORBAR
    str r2, [r5, #0x0FC]

    @ set32(LT_GPIO_ENABLE, GP_SENSORBAR)
    ldr r2, [r5, #0x0DC]
    orr r2, r2, #GP_SENSORBAR
    str r2, [r5, #0x0DC]

    @ set32(LT_GPIO_DIR, GP_SENSORBAR)
    ldr r2, [r5, #0x0E4]
    orr r2, r2, #GP_SENSORBAR
    str r2, [r5, #0x0E4]

    @ set32(LT_GPIO_OUT, GP_SENSORBAR)
    ldr r2, [r5, #0x0E0]
    orr r2, r2, #GP_SENSORBAR
    str r2, [r5, #0x0E0]

    lolserial_send_string_loop:
        cmp r0, r1                                      @ ensure end address was not reached
        ldrneb r3, [r0], #1                             @   load cur char in r3, increment r0 string ptr
        cmpne r3, #0                                    @   check for string null termination
        beq lolserial_send_string_end

        @ r3: bits to send (serial end bit, data byte, serial start bit)
        @ r3 = (1 << 9) | (r3:chr << 1) | (0 << 0)
        lsl r3, #1
        orr r3, #0x200

        @ loop over bits to send
        lolserial_send_char_loop:
            @ mask32(LT_GPIO_OUT, GP_SENSORBAR, bit ? GP_SENSORBAR : 0)
            tst r3, #1
            ldr r2, [r5, #0x0E0]
            biceq r2, #GP_SENSORBAR
            orrne r2, #GP_SENSORBAR
            str r2, [r5, #0x0E0]

            @ wait for LOLSERIAL_WAIT_TICKS timer ticks
            ldr r2, [r5, #0x010]                        @ r2: current ticks
            adds r4, r2, #LOLSERIAL_WAIT_TICKS          @ r4: wait time end
            timer_wait_overflow_loop:
                cmp r4, r2
                ldr r2, [r5, #0x010]
                blo timer_wait_overflow_loop
            timer_wait_loop:
                cmp r4, r2
                ldr r2, [r5, #0x010]
                bhi timer_wait_loop

            lsrs r3, #1                                 @ go to next bit
            bne lolserial_send_char_loop                @ unless r3 is 0 (end bit sent)
        
        b lolserial_send_string_loop
    lolserial_send_string_end:

    pop {r4-r5}
    bx lr

#endif /* LOLSERIAL_DEBUG */
