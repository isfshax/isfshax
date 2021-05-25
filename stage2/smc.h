/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef _SMC_H
#define _SMC_H

#include "types.h"

void __attribute__((__noreturn__)) smc_shutdown(bool reset);
void __attribute__((__noreturn__)) smc_reset(void);
void __attribute__((__noreturn__)) smc_power_off(void);

#endif
