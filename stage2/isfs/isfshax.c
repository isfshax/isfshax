/*
 * isfshax.c
 *
 * Copyright (C) 2021          rw-r-r-0644 <rwrr0644@gmail.com>
 *
 * This code is licensed to you under the terms of the GNU GPL, version 2;
 * see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 *
 * isfshax is installed to 4 superblock slots for redundancy
 * against ecc errors and nand blocks wear out.
 *
 * when a boot1 superblock recommit attempt (due to ecc errors)
 * is detected, the superblock will be rewritten to the next
 * isfshax slot after correcting ecc errors.
 *
 * the generation number is incremented after each correction;
 * once the maximum allowed genertion number is reached, all
 * isfshax superblocks are rewritten with a lower generation number.
 *
 * if one of the blocks becomes bad during the rewrite,
 * the range of used generations numbers is incremented to ensure
 * the old superblock will not be used:
 * 0 BAD BLOCKS -> generation no. 0xfffffaff-0xfffffbff
 * ...
 * 3 BAD BLOCKS -> generation no. 0xfffffdff-0xfffffeff
 *
 * bad block information is stored along with other informations in a
 * separate isfshax info structure inside of the superblock, since all
 * isfshax slots are already marked as bad to guard against IOSU intervention
 * inside of the normal ISFS cluster allocation table.
 */
#include "types.h"
#include "isfs/isfs.h"
#include "isfs/super.h"
#include "isfs/volume.h"
#include "isfs/isfshax.h"
#include "malloc.h"



/* boot1 superblock loading address.
 * this can be used to read the current generation
 * and the list of isfshax superblock slots, but it
 * can't be rewritten directly to nand as it has been
 * modified by the stage1 payload. */
static const isfshax_super *
boot1_superblock = (const isfshax_super *)(0x01f80000);

static isfshax_super
superblock;


int isfshax_refresh()
{
    isfs_ctx *slc = isfs_get_volume(ISFSVOL_SLC);
    u32 curindex, offs, count = 1, written = 0;
    u32 generation;

    /* detect if the superblock contains ecc errors and boot1
     * attempted to recommit the superblock */
    if (boot1_superblock->generation == boot1_superblock->isfshax.generation)
        return 0;

    /* load the newest valid isfshax superblock slot */
    curindex = boot1_superblock->isfshax.index;
    for (offs = 0; offs < ISFSHAX_REDUNDANCY; offs++) {
        u32 index = (curindex + offs) & (ISFSHAX_REDUNDANCY - 1);
        u32 slot = boot1_superblock->isfshax.slots[index] & ~ISFSHAX_BAD_SLOT;

        if (isfs_read_super(slc, &superblock, slot) >= 0) {
            curindex = index;
            break;
        }
    }
    if (offs == ISFSHAX_REDUNDANCY)
        return -2;

    /* if the last valid generation is reached, rewrite/erase all
     * isfshax superblocks with a lower generation number */
    generation = superblock.generation + 1;
    if (generation >= (superblock.isfshax.generationbase + ISFSHAX_GENERATION_RANGE)) {
        generation = superblock.isfshax.generationbase;
        count = ISFSHAX_REDUNDANCY;
    }

    for (offs = 1; (offs <= ISFSHAX_REDUNDANCY) && (written < count); offs++) {
        u32 index = (curindex + offs) & (ISFSHAX_REDUNDANCY - 1);
        u32 slot = superblock.isfshax.slots[index] & ~ISFSHAX_BAD_SLOT;

        /* skip slots that became bad after a superblock rewrite */
        if (superblock.isfshax.slots[index] & ISFSHAX_BAD_SLOT)
            continue;

        /* if the slot currently in use is being rewritten, ensure
         * at least another slot was already successfully written */
        if ((index == curindex) && !written)
            continue;

        /* update superblock informations */
        superblock.isfshax.index = index;
        superblock.isfshax.generation = generation;
        superblock.generation = generation;

        /* rewrite and verify the superblock */
        if (isfs_write_super(slc, &superblock, slot) >= 0)
        {
            generation++;
            written++;
            continue;
        }

        /* block became bad during write operation, mark
         * the block as bad and go to next generation range */
        superblock.isfshax.slots[index] |= ISFSHAX_BAD_SLOT;
        superblock.isfshax.generationbase += ISFSHAX_GENERATION_RANGE;
        generation = superblock.isfshax.generationbase;

        /* the current superblock became bad. ensure the other
         * isfshax superblock are updated with the new generation range
         * and bad slot information. */
        if (index == curindex)
        {
            offs = 1;
            written = 0;
        }
    }

    /* all isfshax superblocks became bad, or the nand writing code stopped
     * working correctly. either way the user should probably be informed. */
    if (!written)
        return -3;

    return 0;
}
