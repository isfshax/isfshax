/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"     /* FatFs lower layer API */
#include "ff.h"

#include <wctype.h>

#include "string.h"
#include "sdcard.h"
#include "sdhc.h"
#include "utils.h"

static u8 buffer[SDMMC_DEFAULT_BLOCKLEN * SDHC_BLOCK_COUNT_MAX] ALIGNED(32);

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv       /* Physical drive number to identify the drive */
)
{
    (void)pdrv;

    switch(sdcard_check_card()) {
        case SDMMC_INSERTED:
            return 0;
        case SDMMC_NEW_CARD:
            return STA_NOINIT;
        default:
            return STA_NODISK;
    }
}



/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv               /* Physical drive number to identify the drive */
)
{
    if (sdcard_check_card() == SDMMC_NO_CARD)
        return STA_NODISK;

    sdcard_ack_card();
    return disk_status(pdrv);
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,      /* Physical drive number to identify the drive */
    BYTE *buff,     /* Data buffer to store read data */
    DWORD sector,   /* Sector address in LBA */
    UINT count      /* Number of sectors to read */
)
{
    (void)pdrv;

    while(count) {
        u32 work = min(count, SDHC_BLOCK_COUNT_MAX);

        if(sdcard_read(sector, work, buffer) != 0)
            return RES_ERROR;

        memcpy(buff, buffer, work * SDMMC_DEFAULT_BLOCKLEN);

        sector += work;
        count -= work;
        buff += work * SDMMC_DEFAULT_BLOCKLEN;
    }

    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
    BYTE pdrv,          /* Physical drive number to identify the drive */
    const BYTE *buff,   /* Data to be written */
    DWORD sector,       /* Sector address in LBA */
    UINT count          /* Number of sectors to write */
)
{
    (void)pdrv;

    while(count) {
        u32 work = min(count, SDHC_BLOCK_COUNT_MAX);

        memcpy(buffer, buff, work * SDMMC_DEFAULT_BLOCKLEN);

        if(sdcard_write(sector, work, buffer) != 0)
            return RES_ERROR;

        sector += work;
        count -= work;
        buff += work * SDMMC_DEFAULT_BLOCKLEN;
    }

    return RES_OK;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
    BYTE pdrv,      /* Physical drive number to identify the drive */
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    (void)pdrv;

    if (cmd == CTRL_SYNC)
        return RES_OK;

    if (cmd == GET_SECTOR_SIZE) {
        *(u32*)buff = SDMMC_DEFAULT_BLOCKLEN;
        return RES_OK;
    }

    if (cmd == GET_BLOCK_SIZE) {
        *(u32*)buff = 1;
        return RES_OK;
    }

    if (cmd == GET_SECTOR_COUNT) {
        int sectors = sdcard_get_sectors();
        if(sectors < 0) return RES_ERROR;
        *(u32*)buff = sectors;
        return RES_OK;
    }

    return RES_PARERR;
}
#endif

DWORD get_fattime()
{
    // NO
    return 0;
}

#include "option/unicode.c"
