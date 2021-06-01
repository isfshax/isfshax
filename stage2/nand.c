/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2021          rw-r-r-0644
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  Copyright (C) 2008, 2009    Haxx Enterprises <bushing@gmail.com>
 *  Copyright (C) 2008, 2009    Sven Peter <svenpeter@gmail.com>
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "nand.h"
#include "utils.h"
#include "types.h"
#include "latte.h"
#include "memory.h"
#include "irq.h"
#include "crypto.h"
#include "debug.h"
#include <string.h>

/* ECC definitions */
#define ECC_SIZE            0x10
#define ECC_STOR_OFFS       0x30
#define ECC_CALC_OFFS       0x40

/* required buffers sizes */
#define SPARE_BUF_SIZE      (SPARE_SIZE + ECC_SIZE + 0x10)
#define STATUS_BUF_SIZE     0x40

/* NAND chip commands */
#define CMD_CHIPID          0x90
#define CMD_RESET           0xff
#define CMD_GET_STATUS      0x70
#define CMD_ERASE_SETUP     0x60
#define CMD_ERASE           0xd0
#define CMD_SERIALDATA_IN   0x80
#define CMD_RANDOMDATA_IN   0x85
#define CMD_PROGRAM         0x10
#define CMD_READ_SETUP      0x00
#define CMD_READ            0x30

/* NAND_CTRL definitions */
#define CTRL_FL_EXEC        (0x80000000)
#define CTRL_FL_ERR         (0x20000000)
#define CTRL_FL_IRQ         (0x40000000)
#define CTRL_FL_WAIT        (0x00008000)
#define CTRL_FL_WR          (0x00004000)
#define CTRL_FL_RD          (0x00002000)
#define CTRL_FL_ECC         (0x00001000)
#define CTRL_CMD(cmd)       (0x00ff0000 & (cmd << 16))
#define CTRL_ADDR(addr)     (0x1f000000 & (addr << 24))
#define CTRL_SIZE(size)     (0x00000fff & (size))

/* NAND_CONF definitions */
#define CONF_FL_WP          (0x80000000) /* bsp:fla clears this flag when writing */
#define CONF_FL_EN          (0x08000000) /* enable nand controller */
#define CONF_ATTR_INIT      (0x743e3eff) /* initial nand config */
#define CONF_ATTR_NORMAL    (0x550f1eff) /* normal nand config */

/* NAND_BANK definitions */
#define BANK_FL_4           (0x00000004) /* set by bsp:fla for revisions after latte A2X */

#if NAND_WRITE_ENABLED
static u8 nand_status_buf[STATUS_BUF_SIZE] ALIGNED(NAND_DATA_ALIGN);
#endif
static u8 nand_spare_buf[SPARE_BUF_SIZE] ALIGNED(NAND_DATA_ALIGN);

static u32 nand_enabled_banks = BANK_SLC;

static int irq_flag = 0;

int nand_error(const char *error)
{
    DEBUG("nand: %s\n", error);
    nand_initialize();
    return -1;
}

void nand_irq(void)
{
    ahb_flush_from(WB_FLA);
    ahb_flush_to(RB_IOD);
    irq_flag = 1;
}

void nand_irq_clear_and_enable(void)
{
    irq_flag = 0;
    irq_enable(IRQ_NAND);
}

void nand_wait_irq(void)
{
    while(!irq_flag) {
        u32 cookie = irq_kill();
        if (!irq_flag) {
            irq_wait();
        }
        irq_restore(cookie);
    }
}

void nand_enable_banks(u32 bank)
{
    nand_enabled_banks = bank & 3;
}

void nand_set_config(int write_enable)
{
    u32 conf, bank;

    write32(NAND_CTRL, 0);
    write32(NAND_CONF, 0);

    /* set nand config */
    conf = (write_enable ? 0 : CONF_FL_WP)
         | (CONF_FL_EN)
         | (CONF_ATTR_NORMAL);
    write32(NAND_CONF, conf);

    /* set nand bank */
    bank = BANK_FL_4
         | nand_enabled_banks; 
    write32(NAND_BANK, bank);
}

#if NAND_WRITE_ENABLED

int nand_erase_block(u32 blockno)
{
    if (blockno > BLOCK_COUNT) {
        return nand_error("invalid block number");
    }

    /* clear write protection */
    nand_set_config(1);

    /* erase setup */
    write32(NAND_CTRL, 0);
    write32(NAND_ADDR0, 0);
    write32(NAND_ADDR1, blockno * BLOCK_PAGES);
    write32(NAND_CTRL,
        CTRL_FL_EXEC |
        CTRL_CMD(CMD_ERASE_SETUP) |
        CTRL_ADDR(0x1c));
    while(read32(NAND_CTRL) & CTRL_FL_EXEC);

    write32(NAND_CTRL, 0);
    nand_irq_clear_and_enable();

    /* erase */
    write32(NAND_CTRL,
        CTRL_FL_EXEC |
        CTRL_FL_IRQ |
        CTRL_CMD(CMD_ERASE) |
        CTRL_FL_WAIT);
    nand_wait_irq();

    /* set write protection */
    nand_set_config(0);

    /* get status */
    *nand_status_buf = 1;
    dc_flushrange(nand_status_buf, STATUS_BUF_SIZE);

    write32(NAND_DATA, dma_addr(nand_status_buf));
    write32(NAND_CTRL,
        CTRL_FL_EXEC |
        CTRL_CMD(CMD_GET_STATUS) |
        CTRL_FL_RD |
        CTRL_SIZE(STATUS_BUF_SIZE));

    while(read32(NAND_CTRL) & CTRL_FL_EXEC);

    ahb_flush_from(WB_FLA);
    dc_invalidaterange(nand_status_buf, STATUS_BUF_SIZE);

    /* check failure */
    if (*nand_status_buf & 1) {
        return nand_error("erase command failed");
    }

    return 0;
}

int nand_write_page(u32 pageno, void *data, void *spare)
{
    if (pageno > PAGE_COUNT) {
        return nand_error("invalid page number");
    }
    if ((u32)data & (NAND_DATA_ALIGN - 1)) {
        return nand_error("unaligned page buffer");
    }

    dc_flushrange(data, PAGE_SIZE);
    ahb_flush_to(RB_FLA);
    dc_invalidaterange(nand_spare_buf + ECC_CALC_OFFS, ECC_SIZE);

    /* clear write protection */
    nand_set_config(1);

    /* send page content and calc ecc */
    write32(NAND_CTRL, 0);
    write32(NAND_ADDR0, 0);
    write32(NAND_ADDR1, pageno);
    write32(NAND_DATA, dma_addr(data));
    write32(NAND_ECC, dma_addr(nand_spare_buf));
    write32(NAND_CTRL,
        CTRL_FL_EXEC | 
        CTRL_ADDR(0x1f) | 
        CTRL_CMD(CMD_SERIALDATA_IN) | 
        CTRL_FL_WR | 
        CTRL_FL_ECC | 
        CTRL_SIZE(PAGE_SIZE));
    while(read32(NAND_CTRL) & CTRL_FL_EXEC);

    if (read32(NAND_CTRL) & CTRL_FL_ERR) {
        nand_error("error executing data input command");
    }

    /* prepare page spare */
    ahb_flush_from(WB_FLA);
    if (spare) {
        memcpy(nand_spare_buf, spare, SPARE_SIZE);
    } else {
        memset(nand_spare_buf, 0, SPARE_SIZE);
    }
    nand_spare_buf[0] = 0xff;
    memcpy(nand_spare_buf + ECC_STOR_OFFS, nand_spare_buf + ECC_CALC_OFFS, ECC_SIZE);
    dc_flushrange(nand_spare_buf, SPARE_SIZE);

    /* setup irq */
    write32(NAND_CTRL, 0);
    nand_irq_clear_and_enable();

    /* send spare content */
    write32(NAND_ADDR0, PAGE_SIZE);
    write32(NAND_ADDR1, 0);
    write32(NAND_DATA, dma_addr(nand_spare_buf));
    write32(NAND_ECC, 0);
    write32(NAND_CTRL,
        CTRL_FL_EXEC |
        CTRL_ADDR(0x3) |
        CTRL_CMD(CMD_RANDOMDATA_IN) |
        CTRL_FL_WR |
        CTRL_SIZE(SPARE_SIZE));
    while(read32(NAND_CTRL) & CTRL_FL_EXEC);

    if (read32(NAND_CTRL) & CTRL_FL_ERR) {
        nand_error("error executing random data input command");
    }

    /* program page */
    write32(NAND_CTRL, 0);
    write32(NAND_CTRL,
        CTRL_FL_EXEC |
        CTRL_FL_IRQ |
        CTRL_CMD(CMD_PROGRAM) |
        CTRL_FL_WAIT);
    nand_wait_irq();

    /* set write protection */
    nand_set_config(0);

    /* get status */
    *nand_status_buf = 1;
    dc_flushrange(nand_status_buf, STATUS_BUF_SIZE);
    
    write32(NAND_DATA, dma_addr(nand_status_buf));
    write32(NAND_CTRL,
            CTRL_FL_EXEC |
            CTRL_CMD(CMD_GET_STATUS) |
            CTRL_FL_RD |
            CTRL_SIZE(STATUS_BUF_SIZE));
    while(read32(NAND_CTRL) & CTRL_FL_EXEC);

    ahb_flush_from(WB_FLA);
    dc_invalidaterange(nand_status_buf, STATUS_BUF_SIZE);

    /* check failure */
    if (*nand_status_buf & 1) {
        return nand_error("page program command failed");
    }
    
    return 0;
}

#endif

int nand_ecc_correct(u8 *data, u32 *ecc_save, u32 *ecc_calc, u32 size)
{
    u32 syndrome;
    u16 odd, even;

    /* check if the page contain ecc errors */
    if (!memcmp(ecc_save, ecc_calc, size)) {
        return 0;
    }

    /* correct ecc errors */
    for (int i = 0; i < (size / 4); i++) {
        if (ecc_save[i] == ecc_calc[i]) {
            continue;
        }

        /* don't try to correct unformatted pages */
        if (ecc_save[i] == 0xffffffff) {
            continue;
        }

        /* calculate ecc syndrome */
        syndrome = (ecc_save[i] ^ ecc_calc[i]) & 0x0fff0fff;
        if ((syndrome & (syndrome - 1)) == 0) {
            continue;
        }

        /* extract odd and even halves */
        odd = syndrome >> 16;
        even = syndrome;

        /* uncorrectable error */
        if ((odd ^ even) != 0xfff) {
            return -1;
        }

        /* fix the bad bit */
        data[i * 0x200 + (odd >> 3)] ^= 1 << (odd & 7);
    }

    return 1;
}

int nand_read_page(u32 pageno, void *data, void *spare)
{
    int res = 0;

    if (pageno > PAGE_COUNT) {
        return nand_error("invalid page number");
    }
    if ((u32)data & (NAND_DATA_ALIGN - 1)) {
        return nand_error("unaligned page buffer");
    }

    /* set nand config */
    nand_set_config(0);

    /* prepare for reading */
    write32(NAND_CTRL, 0);
    write32(NAND_ADDR0, 0);
    write32(NAND_ADDR1, pageno);
    write32(NAND_CTRL,
        CTRL_FL_EXEC |
        CTRL_ADDR(0x1f) |
        CTRL_CMD(CMD_READ_SETUP));
    while(read32(NAND_CTRL) & CTRL_FL_EXEC);

    /* read page and spare */
    dc_invalidaterange(data, PAGE_SIZE);
    dc_invalidaterange(nand_spare_buf, SPARE_BUF_SIZE);
    write32(NAND_CTRL, 0);
    write32(NAND_DATA, dma_addr(data));
    write32(NAND_ECC, dma_addr(nand_spare_buf));
    nand_irq_clear_and_enable();
    write32(NAND_CTRL,
        CTRL_FL_EXEC |
        CTRL_FL_IRQ |
        CTRL_CMD(CMD_READ) |
        CTRL_FL_WAIT |
        CTRL_FL_RD |
        CTRL_FL_ECC |
        CTRL_SIZE(PAGE_SIZE + SPARE_SIZE));
    nand_wait_irq();

    if (read32(NAND_CTRL) & CTRL_FL_ERR) {
        return nand_error("error executing page read command");
    }

    write32(NAND_CTRL, 0);
    ahb_flush_from(WB_FLA);

    /* correct ecc errors */
    res = nand_ecc_correct(data,
                           (u32*)(nand_spare_buf + ECC_STOR_OFFS),
                           (u32*)(nand_spare_buf + ECC_CALC_OFFS),
                           ECC_SIZE);
    if (res < 0) {
        return nand_error("uncorrectable ecc error");
    }

    /* copy spare from internal buffer */
    if (spare) {
        memcpy(spare, nand_spare_buf, SPARE_SIZE);
    }

    return res;
}

void nand_deinitialize(void)
{
    /* shutdown nand banks */
    write32(NAND_BANK_CTRL, 0);
    while(read32(NAND_BANK_CTRL) & (1 << 31));
    write32(NAND_BANK_CTRL, 0);
    
    for (int i = 0; i < 0xc0; i += 0x18) {
        write32(NAND_REG_BASE + 0x40 + i, 0);
        write32(NAND_REG_BASE + 0x44 + i, 0);
        write32(NAND_REG_BASE + 0x48 + i, 0);
        write32(NAND_REG_BASE + 0x4c + i, 0);
        write32(NAND_REG_BASE + 0x50 + i, 0);
        write32(NAND_REG_BASE + 0x54 + i, 0);
    }
    
    /* shutdown main nand bank */
    write32(NAND_CTRL, 0);
    while(read32(NAND_CTRL) & (1 << 31));
    write32(NAND_CTRL, 0);
    
    /* write init config */
    write32(NAND_CONF, CONF_ATTR_INIT);
    write32(NAND_BANK, 1);
}

void nand_initialize(void)
{    
    for (int i = 0; i < 2; i++) {
        /* shutdown nand interface */
        nand_deinitialize();
        
        /* set nand init config and enable */
        write32(NAND_CONF,
                CONF_FL_EN |
                CONF_ATTR_INIT);
        
        /* set nand bank */
        write32(NAND_BANK,
                BANK_FL_4 |   // ???
                (i ? 3 : 1)); // ???
        
        /* reset nand chip */
        write32(NAND_CTRL,
                CTRL_FL_EXEC |
                CTRL_CMD(CMD_RESET) |
                CTRL_FL_WAIT);
        while(read32(NAND_CTRL) & CTRL_FL_EXEC);
        write32(NAND_CTRL, 0);
    }
    
    /* set normal nand config */
    nand_set_config(0);
}
