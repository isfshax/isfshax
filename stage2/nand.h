/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2021          rw-r-r-0644
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *  Copyright (C) 2008, 2009    Sven Peter <svenpeter@gmail.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef __NAND_H__
#define __NAND_H__

#include "types.h"

#define NAND_WRITE_ENABLED  1

/* nand structure definitions */
#define PAGE_SIZE           0x800
#define PAGE_COUNT          0x40000

#define SPARE_SIZE          0x40

#define CLUSTER_PAGES       8
#define CLUSTER_SIZE        (PAGE_SIZE * CLUSTER_PAGES)
#define CLUSTER_COUNT       (PAGE_COUNT / CLUSTER_PAGES)

#define BLOCK_CLUSTERS      8
#define BLOCK_PAGES         0x40
#define BLOCK_SIZE          (CLUSTER_SIZE * BLOCK_CLUSTERS)
#define BLOCK_COUNT         0x1000

/* nand banks */
#define BANK_SLCCMPT        1
#define BANK_SLC            2

/* initialize nand */
void nand_initialize(void);

/* shutdown nand interface */
void nand_deinitialize(void);

/* read page and spare */
int nand_read_page(u32 pageno, void *data, void *spare);

#if NAND_WRITE_ENABLED
/* write page and spare */
int nand_write_page(u32 pageno, void *data, void *spare);

/* erase a block of pages */
int nand_erase_block(u32 blockno);
#endif

/* set enabled nand banks */
void nand_enable_banks(u32 bank);

/* nand irq handler */
void nand_irq(void);

#endif
