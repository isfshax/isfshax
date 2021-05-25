/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *  Copyright (C) 2008, 2009    Sven Peter <svenpeter@gmail.com>
 *  Copyright (C) 2009          Andre Heider "dhewg" <dhewg@wiibrew.org>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "irq.h"
#include "latte.h"
#include "utils.h"
#include "crypto.h"
#include "nand.h"
#include "sdcard.h"
#include "debug.h"

static u32 _alarm_frequency = 0;

void irq_setup_stack(void);

void irq_initialize(void)
{
    irq_setup_stack();
    write32(LT_INTMR_AHBALL_ARM, 0);
    write32(LT_INTSR_AHBALL_ARM, 0xffffffff);
    write32(LT_INTMR_AHBLT_ARM, 0);
    write32(LT_INTSR_AHBLT_ARM, 0xffffffff);
    irq_restore(CPSR_FIQDIS);

    //???
    write32(LT_INTMR_AHBALL_ARM2X, 0);
    write32(LT_INTMR_AHBLT_ARM2X, 0);
    write32(LT_ERROR_MASK, 0);

    write32(LT_ALARM, 0);
}

void irq_shutdown(void)
{
    write32(LT_INTMR_AHBALL_ARM, 0);
    write32(LT_INTSR_AHBALL_ARM, 0xffffffff);
    write32(LT_INTMR_AHBLT_ARM, 0);
    write32(LT_INTSR_AHBLT_ARM, 0xffffffff);
    irq_kill();
}

void irq_handler(void)
{
    u32 all_enabled = read32(LT_INTMR_AHBALL_ARM);
    u32 all_flags = read32(LT_INTSR_AHBALL_ARM);
    u32 all_mask = all_enabled & all_flags;

    u32 lt_enabled = read32(LT_INTMR_AHBLT_ARM);
    u32 lt_flags = read32(LT_INTSR_AHBLT_ARM);
    u32 lt_mask = lt_enabled & lt_flags;

    /*DEBUG("In IRQ handler: ALL (0x%08x 0x%08x 0x%08x) LT (0x%08x 0x%08x 0x%08x)\n",
            all_enabled, all_flags, all_mask, lt_enabled, lt_flags, lt_mask);*/

    if(all_mask & IRQF_TIMER) {
        if (_alarm_frequency)
            write32(LT_ALARM, read32(LT_TIMER) + _alarm_frequency);

        write32(LT_INTSR_AHBALL_ARM, IRQF_TIMER);
    }

    if(all_mask & IRQF_NAND) {
//      DEBUG("IRQ: NAND\n");
        write32(NAND_CTRL, 0x7fffffff); // shut it up
        write32(LT_INTSR_AHBALL_ARM, IRQF_NAND);
        nand_irq();
    }
    /*
    if(all_mask & IRQF_GPIO1B) {
//      DEBUG("IRQ: GPIO1B\n");
        write32(HW_GPIO1BINTFLAG, 0xFFFFFF); // shut it up
        write32(LT_INTSR_AHBALL_ARM, IRQF_GPIO1B);
    }
    if(all_mask & IRQF_GPIO1) {
//      DEBUG("IRQ: GPIO1\n");
        write32(HW_GPIO1INTFLAG, 0xFFFFFF); // shut it up
        write32(LT_INTSR_AHBALL_ARM, IRQF_GPIO1);
    }
    if(all_mask & IRQF_RESET) {
//      DEBUG("IRQ: RESET\n");
        write32(LT_INTSR_AHBALL_ARM, IRQF_RESET);
    }*/
    if(all_mask & IRQF_SHA1) {
//        DEBUG("IRQ: SHA1\n");
        write32(LT_INTSR_AHBALL_ARM, IRQF_SHA1);
    }
    if(all_mask & IRQF_AES) {
//      DEBUG("IRQ: AES\n");
        write32(LT_INTSR_AHBALL_ARM, IRQF_AES);
    }
    if(all_mask & IRQF_SD0) {
//      DEBUG("IRQ: SD0\n");
        sdcard_irq();
        write32(LT_INTSR_AHBALL_ARM, IRQF_SD0);
    }

    if(lt_mask & IRQLF_SD2) {
//      DEBUG("IRQL: SD2\n");
        //mlc_irq(); //Ash: don't need MLC support, turning it off
        write32(LT_INTSR_AHBLT_ARM, IRQLF_SD2);
    }

    all_mask &= ~IRQF_ALL;
    if(all_mask) {
        DEBUG("IRQ: ALL unknown 0x%08lx\n", all_mask);
        write32(LT_INTSR_AHBALL_ARM, all_mask);
    }
    lt_mask &= ~IRQLF_ALL;
    if(lt_mask) {
        DEBUG("IRQ: LT unknown 0x%08lx\n", lt_mask);
        write32(LT_INTSR_AHBALL_ARM, lt_mask);
    }
}

void irq_enable(u32 irq)
{
    set32(LT_INTMR_AHBALL_ARM, 1<<irq);
}

void irql_enable(u32 irq)
{
    set32(LT_INTMR_AHBLT_ARM, 1<<irq);
}

void irq_disable(u32 irq)
{
    clear32(LT_INTMR_AHBALL_ARM, 1<<irq);
}

void irql_disable(u32 irq)
{
    clear32(LT_INTMR_AHBLT_ARM, 1<<irq);
}

void irq_set_alarm(u32 ms, u8 enable)
{
    _alarm_frequency = IRQ_ALARM_MS2REG(ms);

    if (enable)
        write32(LT_ALARM, read32(LT_TIMER) + _alarm_frequency);
}

void irq_wait(void)
{
    u32 data = 0;
    __asm__ volatile ( "mcr p15, 0, %0, c7, c0, 4" : : "r" (data) );
}
