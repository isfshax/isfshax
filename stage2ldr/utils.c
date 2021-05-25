/*
    mini - a Free Software replacement for the Nintendo/BroadOn IOS.

    ELF loader: random utilities

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
#include "types.h"
#include "utils.h"
#include "hollywood.h"
#include "start.h"

void udelay(u32 d)
{
    // should be good to max .2% error
    u32 ticks = d * 19 / 10;

    write32(HW_TIMER, 0);
    while(read32(HW_TIMER) < ticks);
}

void panic(u8 v)
{
    while(1) {
        // uhh
    }
}

