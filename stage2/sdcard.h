/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  Copyright (C) 2008, 2009    Sven Peter <svenpeter@gmail.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef __SDCARD_H__
#define __SDCARD_H__

#include "bsdtypes.h"
#include "sdmmc.h"

void sdcard_init(void);
void sdcard_exit(void);
void sdcard_irq(void);

void sdcard_attach(sdmmc_chipset_handle_t handle);
void sdcard_needs_discover(void);
int sdcard_wait_data(void);

int sdcard_select(void);
int sdcard_check_card(void);
int sdcard_ack_card(void);
int sdcard_get_sectors(void);

int sdcard_read(u32 blk_start, u32 blk_count, void *data);
int sdcard_write(u32 blk_start, u32 blk_count, void *data);

int sdcard_start_read(u32 blk_start, u32 blk_count, void *data, struct sdmmc_command* cmdbuf);
int sdcard_end_read(struct sdmmc_command* cmdbuf);

int sdcard_start_write(u32 blk_start, u32 blk_count, void *data, struct sdmmc_command* cmdbuf);
int sdcard_end_write(struct sdmmc_command* cmdbuf);

#endif
