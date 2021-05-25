/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "types.h"
#include "utils.h"
#include "latte.h"

void SRAM_TEXT __attribute__((__noreturn__)) smc_shutdown(bool reset)
{
    write16(MEM_FLUSH_MASK, 0b1111);
    while(read16(MEM_FLUSH_MASK) & 0b1111);

    if(read32(LT_RESETS) & 4) {
        write32(LT_ABIF_CPLTL_OFFSET, 0xC0008020);
        write32(LT_ABIF_CPLTL_DATA, 0xFFFFFFFF);
        write32(LT_ABIF_CPLTL_OFFSET, 0xC0000E60);
        write32(LT_ABIF_CPLTL_DATA, 0xFFFFFFDB);
    }

    write32(LT_RESETS_AHB, 0xFFFFCE71);
    write32(LT_RESETS_AHMN, 0xFFFFCD70);
    write32(LT_RESETS_COMPAT, 0xFF8FCDEF);

    write16(MEM_REFRESH_FLAG, 0);

    write16(MEM_SEQ_REG_ADDR, 0x18);
    write16(MEM_SEQ_REG_VAL, 1);
    write16(MEM_SEQ_REG_ADDR, 0x19);
    write16(MEM_SEQ_REG_VAL, 0);
    write16(MEM_SEQ_REG_ADDR, 0x1A);
    write16(MEM_SEQ_REG_VAL, 1);

    write16(MEM_SEQ0_REG_ADDR, 0x18);
    write16(MEM_SEQ0_REG_VAL, 1);
    write16(MEM_SEQ0_REG_ADDR, 0x19);
    write16(MEM_SEQ0_REG_VAL, 0);
    write16(MEM_SEQ0_REG_ADDR, 0x1A);
    write16(MEM_SEQ0_REG_VAL, 1);

    {
        write32(EXI0_CSR, 0x108);
        write32(EXI0_DATA, 0xA1000100);
        write32(EXI0_CR, 0x35);
        while(!(read32(EXI0_CSR) & 8));

        write32(EXI0_CSR, 0x108);
        write32(EXI0_DATA, 0);
        write32(EXI0_CR, 0x35);
        while(!(read32(EXI0_CSR) & 8));

        write32(EXI0_CSR, 0);
    }

    if(reset) {
        {
            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0xA1000D00);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0x501);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0);
        }

        clear32(LT_RESETS, 1);
    } else {
        {
            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0xA1000D00);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0x101);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0);
        }

        {
            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0xA1000D00);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0x10101);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0);
        }
    }

    while(true);
}

void __attribute__((__noreturn__)) smc_power_off(void)
{
    smc_shutdown(false);
}

void __attribute__((__noreturn__)) smc_reset(void)
{
    smc_shutdown(true);
}
