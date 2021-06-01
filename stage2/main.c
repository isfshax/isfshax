/*
 * ISFSHAX
 * 
 * Copyright (C) 2021          rw-r-r-0644 <rwrr0644@gmail.com>
 *
 * Based on code from Minute and Mini:
 *
 * Copyright (C) 2017          Ash Logan <quarktheawesome@gmail.com>
 * Copyright (C) 2016          SALT
 * Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 * Copyright (C) 2008, 2009    Haxx Enterprises <bushing@gmail.com>
 * Copyright (C) 2008, 2009    Sven Peter <svenpeter@gmail.com>
 * Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 * Copyright (C) 2009          Andre Heider "dhewg" <dhewg@wiibrew.org>
 * Copyright (C) 2009          John Kelley <wiidev@kelley.ca>
 *
 * This code is licensed to you under the terms of the GNU GPL, version 2;
 * see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */
#include <stdlib.h>
#include "memory.h"
#include "irq.h"
#include "crypto.h"
#include "smc.h"
#include "latte.h"
#include "utils.h"
#include "isfs/isfshax.h"
#include "sdcard.h"
#include "fatfs/ff.h"
#include "nand.h"
#include "isfs/isfs.h"
#include "ancast.h"

u32 load_payload_sd(void)
{
    static FATFS fatfs;
    UINT btr, br;
    FRESULT res;
    FIL file;
    u32 vector = 0;

    ancast_iop_clear((u8*)ANCAST_ADDRESS_IOP);

    sdcard_init();

    res = f_mount(&fatfs, "0:", 1);
    if (res)
        goto error_mount;

    res = f_open(&file, "isfshax.bin", FA_READ);
    if (res)
        goto error_open;

    btr = f_size(&file);
    if (!btr)
        goto error_read;

    res = f_read(&file, (u8*)ANCAST_ADDRESS_IOP, btr, &br);
    if (res || (btr != br))
        goto error_read;

    vector = ancast_iop_load((u8*)ANCAST_ADDRESS_IOP, btr);

error_read:
    f_close(&file);
error_open:
    f_mount(0, "0:", 0);
error_mount:
    sdcard_exit();
    return vector;
}

u32 load_payload_nand(void)
{
    isfs_file file;
    size_t btr, br;
    int res;
    u32 vector = 0;

    ancast_iop_clear((u8*)ANCAST_ADDRESS_IOP);

    res = isfs_init();
    if (res)
        return vector;

    res = isfs_open(&file, "slc:/sys/isfshax.bin");
    if (res)
        goto error_open;

    btr = file.fst->size;
    if (!btr)
        goto error_read;

    res = isfs_read(&file, (u8*)ANCAST_ADDRESS_IOP, btr, &br);
    if (res || (btr != br))
        goto error_read;

    vector = ancast_iop_load((u8*)ANCAST_ADDRESS_IOP, btr);

error_read:
    isfs_close(&file);
error_open:
    isfs_fini();
    return vector;
}

u32 _main(void)
{
    u32 vector = 0;
    //mem_initialize();
    irq_initialize();
    crypto_read_otp();
    nand_initialize();

    /* repair isfshax superblocks ecc errors, if present */
    isfshax_refresh();

    /* attempt to load the payload from SD, then NAND */
    vector = load_payload_sd();
    if (!vector)
        vector = load_payload_nand();

    nand_deinitialize();
    irq_shutdown();
    //mem_shutdown();

    /* failed to load the payload from SD or NAND -> shutdown */
    if (!vector)
        smc_shutdown(false);

    return vector;
}
