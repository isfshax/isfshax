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

#ifndef __SDMMC_H__
#define __SDMMC_H__

#include "bsdtypes.h"

struct sdmmc_command;

typedef struct sdhc_host * sdmmc_chipset_handle_t;

/* clock frequencies for sdmmc_chip_bus_clock() */
#define SDMMC_SDCLK_OFF     0
#define SDMMC_SDCLK_400KHZ  400
#define SDMMC_SDCLK_25MHZ   25000

#define SDMMC_TIMING_LEGACY 0
#define SDMMC_TIMING_HIGHSPEED  1

struct sdmmc_csd {
    int csdver;     /* CSD structure format */
    int mmcver;     /* MMC version (for CID format) */
    int capacity;   /* total number of sectors */
    int sector_size;    /* sector size in bytes */
    int read_bl_len;    /* block length for reads */
    /* ... */
};

struct sdmmc_cid {
    int mid;        /* manufacturer identification number */
    int oid;        /* OEM/product identification number */
    char    pnm[8];     /* product name (MMC v1 has the longest) */
    int rev;        /* product revision */
    int psn;        /* product serial number */
    int mdt;        /* manufacturing date */
};

typedef u_int32_t sdmmc_response[4];

struct sdmmc_softc;

struct sdmmc_task {
    void (*func)(void *arg);
    void *arg;
    int onqueue;
    struct sdmmc_softc *sc;
};

#define sdmmc_init_task(xtask, xfunc, xarg) do {            \
    (xtask)->func = (xfunc);                    \
    (xtask)->arg = (xarg);                      \
    (xtask)->onqueue = 0;                       \
    (xtask)->sc = NULL;                     \
} while (0)

#define sdmmc_task_pending(xtask) ((xtask)->onqueue)

struct sdmmc_command {
//  struct sdmmc_task c_task;   /* task queue entry */
    u_int16_t    c_opcode;  /* SD or MMC command index */
    u_int32_t    c_arg;     /* SD/MMC command argument */
    sdmmc_response   c_resp;    /* response buffer */
    void        *c_data;    /* buffer to send or read into */
    int      c_datalen; /* length of data buffer */
    int      c_blklen;  /* block length */
    int      c_flags;   /* see below */
#define SCF_ITSDONE  0x0001     /* command is complete */
#define SCF_CMD(flags)   ((flags) & 0x00f0)
#define SCF_CMD_AC   0x0000
#define SCF_CMD_ADTC     0x0010
#define SCF_CMD_BC   0x0020
#define SCF_CMD_BCR  0x0030
#define SCF_CMD_READ     0x0040     /* read command (data expected) */
#define SCF_RSP_BSY  0x0100
#define SCF_RSP_136  0x0200
#define SCF_RSP_CRC  0x0400
#define SCF_RSP_IDX  0x0800
#define SCF_RSP_PRESENT  0x1000
/* response types */
#define SCF_RSP_R0   0 /* none */
#define SCF_RSP_R1   (SCF_RSP_PRESENT|SCF_RSP_CRC|SCF_RSP_IDX)
#define SCF_RSP_R1B  (SCF_RSP_PRESENT|SCF_RSP_CRC|SCF_RSP_IDX|SCF_RSP_BSY)
#define SCF_RSP_R2   (SCF_RSP_PRESENT|SCF_RSP_CRC|SCF_RSP_136)
#define SCF_RSP_R3   (SCF_RSP_PRESENT)
#define SCF_RSP_R4   (SCF_RSP_PRESENT)
#define SCF_RSP_R5   (SCF_RSP_PRESENT|SCF_RSP_CRC|SCF_RSP_IDX)
#define SCF_RSP_R5B  (SCF_RSP_PRESENT|SCF_RSP_CRC|SCF_RSP_IDX|SCF_RSP_BSY)
#define SCF_RSP_R6   (SCF_RSP_PRESENT|SCF_RSP_CRC|SCF_RSP_IDX)
#define SCF_RSP_R7   (SCF_RSP_PRESENT|SCF_RSP_CRC|SCF_RSP_IDX)
    int     c_error;    /* errno value on completion */

    int     c_timeout;

    /* Host controller owned fields for data xfer in progress */
    int c_resid;            /* remaining I/O */
    u_char *c_buf;          /* remaining data */
};

/*
 * Decoded PC Card 16 based Card Information Structure (CIS),
 * per card (function 0) and per function (1 and greater).
 */
struct sdmmc_cis {
    u_int16_t    manufacturer;
#define SDMMC_VENDOR_INVALID    0xffff
    u_int16_t    product;
#define SDMMC_PRODUCT_INVALID   0xffff
    u_int8_t     function;
#define SDMMC_FUNCTION_INVALID  0xff
    u_char       cis1_major;
    u_char       cis1_minor;
    char         cis1_info_buf[256];
    char        *cis1_info[4];
};

/*
 * Structure describing either an SD card I/O function or a SD/MMC
 * memory card from a "stack of cards" that responded to CMD2.  For a
 * combo card with one I/O function and one memory card, there will be
 * two of these structures allocated.  Each card slot has such a list
 * of sdmmc_function structures.
 */
struct sdmmc_function {
    /* common members */
    u_int16_t rca;          /* relative card address */
    int flags;
#define SFF_ERROR       0x0001  /* function is poo; ignore it */
#define SFF_SDHC        0x0002  /* SD High Capacity card */
    /* SD card I/O function members */
    int number;         /* I/O function number or -1 */
    struct sdmmc_cis cis;       /* decoded CIS */
    /* SD/MMC memory card members */
    struct sdmmc_csd csd;       /* decoded CSD value */
    struct sdmmc_cid cid;       /* decoded CID value */
    sdmmc_response raw_cid;     /* temp. storage for decoding */
};

#define SDMMC_LOCK(sc)   lockmgr(&(sc)->sc_lock, LK_EXCLUSIVE, NULL)
#define SDMMC_UNLOCK(sc) lockmgr(&(sc)->sc_lock, LK_RELEASE, NULL)
#define SDMMC_ASSERT_LOCKED(sc) \
    KASSERT(lockstatus(&((sc))->sc_lock) == LK_EXCLUSIVE)

#define SDMMC_DEFAULT_CLOCK     25000
#define SDMMC_DEFAULT_BLOCKLEN        512

#define SDMMC_NO_CARD               1
#define SDMMC_NEW_CARD              2
#define SDMMC_INSERTED              3

/* MMC commands */              /* response type */
#define MMC_GO_IDLE_STATE       0   /* R0 */
#define MMC_SEND_OP_COND        1   /* R3 */
#define MMC_ALL_SEND_CID        2   /* R2 */
#define MMC_SET_RELATIVE_ADDR       3   /* R1 */
#define MMC_SWITCH              6   /* R1B */
#define MMC_SELECT_CARD         7   /* R1 */
#define MMC_SEND_EXT_CSD        8   /* R1 */
#define MMC_SEND_CSD            9   /* R2 */
#define MMC_STOP_TRANSMISSION       12  /* R1B */
#define MMC_SEND_STATUS         13  /* R1 */
#define MMC_SET_BLOCKLEN        16  /* R1 */
#define MMC_READ_BLOCK_SINGLE       17  /* R1 */
#define MMC_READ_BLOCK_MULTIPLE     18  /* R1 */
#define MMC_SET_BLOCK_COUNT     23  /* R1 */
#define MMC_WRITE_BLOCK_SINGLE      24  /* R1 */
#define MMC_WRITE_BLOCK_MULTIPLE    25  /* R1 */
#define MMC_APP_CMD         55  /* R1 */

/* SD commands */               /* response type */
#define SD_SEND_RELATIVE_ADDR       3   /* R6 */
#define SD_SWITCH_FUNC          6   /* R1 */
#define SD_SEND_IF_COND         8   /* R7 */

/* SD application commands */           /* response type */
#define SD_APP_SET_BUS_WIDTH        6   /* R1 */
#define SD_APP_OP_COND          41  /* R3 */

/* OCR bits */
#define MMC_OCR_MEM_READY       (1<<31) /* memory power-up status bit */
#define MMC_OCR_3_5V_3_6V       (1<<23)
#define MMC_OCR_3_4V_3_5V       (1<<22)
#define MMC_OCR_3_3V_3_4V       (1<<21)
#define MMC_OCR_3_2V_3_3V       (1<<20)
#define MMC_OCR_3_1V_3_2V       (1<<19)
#define MMC_OCR_3_0V_3_1V       (1<<18)
#define MMC_OCR_2_9V_3_0V       (1<<17)
#define MMC_OCR_2_8V_2_9V       (1<<16)
#define MMC_OCR_2_7V_2_8V       (1<<15)
#define MMC_OCR_2_6V_2_7V       (1<<14)
#define MMC_OCR_2_5V_2_6V       (1<<13)
#define MMC_OCR_2_4V_2_5V       (1<<12)
#define MMC_OCR_2_3V_2_4V       (1<<11)
#define MMC_OCR_2_2V_2_3V       (1<<10)
#define MMC_OCR_2_1V_2_2V       (1<<9)
#define MMC_OCR_2_0V_2_1V       (1<<8)
#define MMC_OCR_1_9V_2_0V       (1<<7)
#define MMC_OCR_1_8V_1_9V       (1<<6)
#define MMC_OCR_1_7V_1_8V       (1<<5)
#define MMC_OCR_1_6V_1_7V       (1<<4)

#define SD_OCR_SDHC_CAP         (1<<30)
#define SD_OCR_VOL_MASK         0xFF8000 /* bits 23:15 */

/* R1 response type bits */
#define MMC_R1_READY_FOR_DATA       (1<<8)  /* ready for next transfer */
#define MMC_R1_APP_CMD          (1<<5)  /* app. commands supported */

/* 48-bit response decoding (32 bits w/o CRC) */
#define MMC_R1(resp)            ((resp)[0])
#define MMC_R3(resp)            ((resp)[0])
#define SD_R6(resp)         ((resp)[0])

/* RCA argument and response */
#define MMC_ARG_RCA(rca)        ((rca) << 16)
#define SD_R6_RCA(resp)         (SD_R6((resp)) >> 16)

/* bus width argument */
#define SD_ARG_BUS_WIDTH_1      0
#define SD_ARG_BUS_WIDTH_4      2

/* MMC R2 response (CSD) */
#define MMC_CSD_CSDVER(resp)        MMC_RSP_BITS((resp), 126, 2)
#define  MMC_CSD_CSDVER_1_0     1
#define  MMC_CSD_CSDVER_2_0     2
#define MMC_CSD_MMCVER(resp)        MMC_RSP_BITS((resp), 122, 4)
#define  MMC_CSD_MMCVER_1_0     0 /* MMC 1.0 - 1.2 */
#define  MMC_CSD_MMCVER_1_4     1 /* MMC 1.4 */
#define  MMC_CSD_MMCVER_2_0     2 /* MMC 2.0 - 2.2 */
#define  MMC_CSD_MMCVER_3_1     3 /* MMC 3.1 - 3.3 */
#define  MMC_CSD_MMCVER_4_0     4 /* MMC 4 */
#define MMC_CSD_READ_BL_LEN(resp)   MMC_RSP_BITS((resp), 80, 4)
#define MMC_CSD_C_SIZE(resp)        MMC_RSP_BITS((resp), 62, 12)
#define MMC_CSD_CAPACITY(resp)      ((MMC_CSD_C_SIZE((resp))+1) << \
                     (MMC_CSD_C_SIZE_MULT((resp))+2))
#define MMC_CSD_C_SIZE_MULT(resp)   MMC_RSP_BITS((resp), 47, 3)

/* MMC v1 R2 response (CID) */
#define MMC_CID_MID_V1(resp)        MMC_RSP_BITS((resp), 104, 24)
#define MMC_CID_PNM_V1_CPY(resp, pnm)                   \
    do {                                \
        (pnm)[0] = MMC_RSP_BITS((resp), 96, 8);         \
        (pnm)[1] = MMC_RSP_BITS((resp), 88, 8);         \
        (pnm)[2] = MMC_RSP_BITS((resp), 80, 8);         \
        (pnm)[3] = MMC_RSP_BITS((resp), 72, 8);         \
        (pnm)[4] = MMC_RSP_BITS((resp), 64, 8);         \
        (pnm)[5] = MMC_RSP_BITS((resp), 56, 8);         \
        (pnm)[6] = MMC_RSP_BITS((resp), 48, 8);         \
        (pnm)[7] = '\0';                    \
    } while (0)
#define MMC_CID_REV_V1(resp)        MMC_RSP_BITS((resp), 40, 8)
#define MMC_CID_PSN_V1(resp)        MMC_RSP_BITS((resp), 16, 24)
#define MMC_CID_MDT_V1(resp)        MMC_RSP_BITS((resp), 8, 8)

/* MMC v2 R2 response (CID) */
#define MMC_CID_MID_V2(resp)        MMC_RSP_BITS((resp), 120, 8)
#define MMC_CID_OID_V2(resp)        MMC_RSP_BITS((resp), 104, 16)
#define MMC_CID_PNM_V2_CPY(resp, pnm)                   \
    do {                                \
        (pnm)[0] = MMC_RSP_BITS((resp), 96, 8);         \
        (pnm)[1] = MMC_RSP_BITS((resp), 88, 8);         \
        (pnm)[2] = MMC_RSP_BITS((resp), 80, 8);         \
        (pnm)[3] = MMC_RSP_BITS((resp), 72, 8);         \
        (pnm)[4] = MMC_RSP_BITS((resp), 64, 8);         \
        (pnm)[5] = MMC_RSP_BITS((resp), 56, 8);         \
        (pnm)[6] = '\0';                    \
    } while (0)
#define MMC_CID_PSN_V2(resp)        MMC_RSP_BITS((resp), 16, 32)

/* SD R2 response (CSD) */
#define SD_CSD_CSDVER(resp)     MMC_RSP_BITS((resp), 126, 2)
#define  SD_CSD_CSDVER_1_0      0
#define  SD_CSD_CSDVER_2_0      1
#define SD_CSD_TAAC(resp)       MMC_RSP_BITS((resp), 112, 8)
#define  SD_CSD_TAAC_1_5_MSEC       0x26
#define SD_CSD_NSAC(resp)       MMC_RSP_BITS((resp), 104, 8)
#define SD_CSD_SPEED(resp)      MMC_RSP_BITS((resp), 96, 8)
#define  SD_CSD_SPEED_25_MHZ        0x32
#define  SD_CSD_SPEED_50_MHZ        0x5a
#define SD_CSD_CCC(resp)        MMC_RSP_BITS((resp), 84, 12)
#define  SD_CSD_CCC_ALL         0x5f5
#define SD_CSD_READ_BL_LEN(resp)    MMC_RSP_BITS((resp), 80, 4)
#define SD_CSD_READ_BL_PARTIAL(resp)    MMC_RSP_BITS((resp), 79, 1)
#define SD_CSD_WRITE_BLK_MISALIGN(resp) MMC_RSP_BITS((resp), 78, 1)
#define SD_CSD_READ_BLK_MISALIGN(resp)  MMC_RSP_BITS((resp), 77, 1)
#define SD_CSD_DSR_IMP(resp)        MMC_RSP_BITS((resp), 76, 1)
#define SD_CSD_C_SIZE(resp)     MMC_RSP_BITS((resp), 62, 12)
#define SD_CSD_CAPACITY(resp)       ((SD_CSD_C_SIZE((resp))+1) << \
                     (SD_CSD_C_SIZE_MULT((resp))+2))
#define SD_CSD_V2_C_SIZE(resp)      MMC_RSP_BITS((resp), 48, 22)
#define SD_CSD_V2_CAPACITY(resp)    ((SD_CSD_V2_C_SIZE((resp))+1) << 10)
#define SD_CSD_V2_BL_LEN        0x9 /* 512 */
#define SD_CSD_VDD_R_CURR_MIN(resp) MMC_RSP_BITS((resp), 59, 3)
#define SD_CSD_VDD_R_CURR_MAX(resp) MMC_RSP_BITS((resp), 56, 3)
#define SD_CSD_VDD_W_CURR_MIN(resp) MMC_RSP_BITS((resp), 53, 3)
#define SD_CSD_VDD_W_CURR_MAX(resp) MMC_RSP_BITS((resp), 50, 3)
#define  SD_CSD_VDD_RW_CURR_100mA   0x7
#define  SD_CSD_VDD_RW_CURR_80mA    0x6
#define SD_CSD_C_SIZE_MULT(resp)    MMC_RSP_BITS((resp), 47, 3)
#define SD_CSD_ERASE_BLK_EN(resp)   MMC_RSP_BITS((resp), 46, 1)
#define SD_CSD_SECTOR_SIZE(resp)    MMC_RSP_BITS((resp), 39, 7) /* +1 */
#define SD_CSD_WP_GRP_SIZE(resp)    MMC_RSP_BITS((resp), 32, 7) /* +1 */
#define SD_CSD_WP_GRP_ENABLE(resp)  MMC_RSP_BITS((resp), 31, 1)
#define SD_CSD_R2W_FACTOR(resp)     MMC_RSP_BITS((resp), 26, 3)
#define SD_CSD_WRITE_BL_LEN(resp)   MMC_RSP_BITS((resp), 22, 4)
#define  SD_CSD_RW_BL_LEN_2G        0xa
#define  SD_CSD_RW_BL_LEN_1G        0x9
#define SD_CSD_WRITE_BL_PARTIAL(resp)   MMC_RSP_BITS((resp), 21, 1)
#define SD_CSD_FILE_FORMAT_GRP(resp)    MMC_RSP_BITS((resp), 15, 1)
#define SD_CSD_COPY(resp)       MMC_RSP_BITS((resp), 14, 1)
#define SD_CSD_PERM_WRITE_PROTECT(resp) MMC_RSP_BITS((resp), 13, 1)
#define SD_CSD_TMP_WRITE_PROTECT(resp)  MMC_RSP_BITS((resp), 12, 1)
#define SD_CSD_FILE_FORMAT(resp)    MMC_RSP_BITS((resp), 10, 2)

/* SD R2 response (CID) */
#define SD_CID_MID(resp)        MMC_RSP_BITS((resp), 120, 8)
#define SD_CID_OID(resp)        MMC_RSP_BITS((resp), 104, 16)
#define SD_CID_PNM_CPY(resp, pnm)                   \
    do {                                \
        (pnm)[0] = MMC_RSP_BITS((resp), 96, 8);         \
        (pnm)[1] = MMC_RSP_BITS((resp), 88, 8);         \
        (pnm)[2] = MMC_RSP_BITS((resp), 80, 8);         \
        (pnm)[3] = MMC_RSP_BITS((resp), 72, 8);         \
        (pnm)[4] = MMC_RSP_BITS((resp), 64, 8);         \
        (pnm)[5] = '\0';                    \
    } while (0)
#define SD_CID_REV(resp)        MMC_RSP_BITS((resp), 56, 8)
#define SD_CID_PSN(resp)        MMC_RSP_BITS((resp), 24, 32)
#define SD_CID_MDT(resp)        MMC_RSP_BITS((resp), 8, 12)

/* Might be slow, but it should work on big and little endian systems. */
#define MMC_RSP_BITS(resp, start, len)  __bitfield((resp), (start)-8, (len))
static __inline int
__bitfield(u_int32_t *src, int start, int len)
{
    u_int8_t *sp;
    u_int32_t dst, mask;
    int shift, bs, bc;

    if (start < 0 || len < 0 || len > 32)
        return 0;

    dst = 0;
    mask = len % 32 ? UINT_MAX >> (32 - (len % 32)) : UINT_MAX;
    shift = 0;

    while (len > 0) {
        sp = (u_int8_t *)src + start / 8;
        bs = start % 8;
        bc = 8 - bs;
        if (bc > len)
            bc = len;
        dst |= (*sp++ >> bs) << shift;
        shift += bc;
        start += bc;
        len -= bc;
    }

    dst &= mask;
    return (int)dst;
}

#endif
