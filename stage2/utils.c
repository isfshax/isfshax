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

#include "types.h"
#include "utils.h"
#include "gpio.h"
#include "latte.h"

#include <stdarg.h>

void udelay(u32 d)
{
    // should be good to max .2% error
    u32 ticks = d * 19 / 10;

    if(ticks < 2)
        ticks = 2;

    u32 now = read32(LT_TIMER);

    u32 then = now + ticks;

    if(then < now) {
        while(read32(LT_TIMER) >= now);
        now = read32(LT_TIMER);
    }

    while(now < then) {
        now = read32(LT_TIMER);
    }
}

void panic(u8 v)
{
    while(true) {
        //debug_output(v);
        //set32(HW_GPIO1BOUT, GP_SLOTLED);
        //udelay(500000);
        //debug_output(0);
        //clear32(HW_GPIO1BOUT, GP_SLOTLED);
        //udelay(500000);
    }
}
