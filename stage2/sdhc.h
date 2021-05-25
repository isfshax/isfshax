/*  $OpenBSD: sdhcvar.h,v 1.3 2007/09/06 08:01:01 jsg Exp $ */

/*
 * Copyright (c) 2006 Uwe Stuehler <uwe@openbsd.org>
 * Copyright (c) 2009 Sven Peter <svenpeter@gmail.com>
 * Copyright (c) 2016 Daz Jones <daz@dazzozo.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _SDHCVAR_H_
#define _SDHCVAR_H_

#include "bsdtypes.h"
#include "memory.h"
#include "sdmmc.h"

struct sdhc_host_params {
    void (*attach)();
    void (*abort)();
    enum rb_client rb;
    enum wb_client wb;
};

struct sdhc_host {
    bus_space_tag_t iot;        /* host register set tag */
    bus_space_handle_t ioh;     /* host register set handle */
    u_int16_t version;      /* specification version */
    u_int clkbase;          /* base clock frequency in KHz */
    int flags;          /* flags for this host */
    u_int32_t ocr;          /* OCR value from capabilities */
    u_int8_t regs[14];      /* host controller state */
    volatile u_int16_t intr_status;      /* soft interrupt status */
    volatile u_int16_t intr_error_status;    /* soft error status */
    int data_command;

    struct sdhc_host_params pa;
};

/* Host controller functions called by the attachment driver. */
int sdhc_host_found(struct sdhc_host *, struct sdhc_host_params *, bus_space_tag_t, bus_space_handle_t, int);
void    sdhc_power(int, void *);
void    sdhc_shutdown(struct sdhc_host *);
int sdhc_intr(struct sdhc_host *);

/* Host standard register set */
#define SDHC_DMA_ADDR           0x00
#define SDHC_BLOCK_SIZE         0x04
#define SDHC_BLOCK_COUNT        0x06
#define SDHC_BLOCK_COUNT_MAX        512
#define SDHC_ARGUMENT           0x08
#define SDHC_TRANSFER_MODE      0x0c
#define SDHC_MULTI_BLOCK_MODE       (1<<5)
#define SDHC_READ_MODE          (1<<4)
#define SDHC_AUTO_CMD12_ENABLE      (1<<2)
#define SDHC_BLOCK_COUNT_ENABLE     (1<<1)
#define SDHC_DMA_ENABLE         (1<<0)
#define SDHC_COMMAND            0x0e
/* 14-15 reserved */
#define SDHC_COMMAND_INDEX_SHIFT    8
#define SDHC_COMMAND_INDEX_MASK     0x3f
#define SDHC_COMMAND_TYPE_ABORT     (3<<6)
#define SDHC_COMMAND_TYPE_RESUME    (2<<6)
#define SDHC_COMMAND_TYPE_SUSPEND   (1<<6)
#define SDHC_COMMAND_TYPE_NORMAL    (0<<6)
#define SDHC_DATA_PRESENT_SELECT    (1<<5)
#define SDHC_INDEX_CHECK_ENABLE     (1<<4)
#define SDHC_CRC_CHECK_ENABLE       (1<<3)
/* 2 reserved */
#define SDHC_RESP_LEN_48_CHK_BUSY   (3<<0)
#define SDHC_RESP_LEN_48        (2<<0)
#define SDHC_RESP_LEN_136       (1<<0)
#define SDHC_NO_RESPONSE        (0<<0)
#define SDHC_RESPONSE           0x10    /* - 0x1f */
#define SDHC_DATA           0x20
#define SDHC_PRESENT_STATE      0x24
/* 25-31 reserved */
#define SDHC_CMD_LINE_SIGNAL_LEVEL  (1<<24)
#define SDHC_DAT3_LINE_LEVEL        (1<<23)
#define SDHC_DAT2_LINE_LEVEL        (1<<22)
#define SDHC_DAT1_LINE_LEVEL        (1<<21)
#define SDHC_DAT0_LINE_LEVEL        (1<<20)
#define SDHC_WRITE_PROTECT_SWITCH   (1<<19)
#define SDHC_CARD_DETECT_PIN_LEVEL  (1<<18)
#define SDHC_CARD_STATE_STABLE      (1<<17)
#define SDHC_CARD_INSERTED      (1<<16)
/* 12-15 reserved */
#define SDHC_BUFFER_READ_ENABLE     (1<<11)
#define SDHC_BUFFER_WRITE_ENABLE    (1<<10)
#define SDHC_READ_TRANSFER_ACTIVE   (1<<9)
#define SDHC_WRITE_TRANSFER_ACTIVE  (1<<8)
/* 3-7 reserved */
#define SDHC_DAT_ACTIVE         (1<<2)
#define SDHC_CMD_INHIBIT_DAT        (1<<1)
#define SDHC_CMD_INHIBIT_CMD        (1<<0)
#define SDHC_CMD_INHIBIT_MASK       0x0003
#define SDHC_HOST_CTL           0x28
#define SDHC_8BIT_MODE          (1<<5)
#define SDHC_HIGH_SPEED         (1<<2)
#define SDHC_4BIT_MODE          (1<<1)
#define SDHC_LED_ON         (1<<0)
#define SDHC_POWER_CTL          0x29
#define SDHC_VOLTAGE_SHIFT      1
#define SDHC_VOLTAGE_MASK       0x07
#define SDHC_VOLTAGE_3_3V       0x07
#define SDHC_VOLTAGE_3_0V       0x06
#define SDHC_VOLTAGE_1_8V       0x05
#define SDHC_BUS_POWER          (1<<0)
#define SDHC_BLOCK_GAP_CTL      0x2a
#define SDHC_WAKEUP_CTL         0x2b
#define SDHC_CLOCK_CTL          0x2c
#define SDHC_SDCLK_DIV_SHIFT        8
#define SDHC_SDCLK_DIV_MASK     0xff
#define  SDHC_SDCLK_DIV_RSHIFT_V3   2
#define  SDHC_SDCLK_DIV_MASK_V3     0x300
#define SDHC_SDCLK_ENABLE       (1<<2)
#define SDHC_INTCLK_STABLE      (1<<1)
#define SDHC_INTCLK_ENABLE      (1<<0)
#define SDHC_TIMEOUT_CTL        0x2e
#define SDHC_TIMEOUT_MAX        0x0e
#define SDHC_SOFTWARE_RESET     0x2f
#define SDHC_RESET_MASK         0x5
#define SDHC_RESET_DAT          (1<<2)
#define SDHC_RESET_CMD          (1<<1)
#define SDHC_RESET_ALL          (1<<0)
#define SDHC_NINTR_STATUS       0x30
#define SDHC_ERROR_INTERRUPT        (1<<15)
#define SDHC_ERROR_TIMEOUT      (1<<14)
#define SDHC_CARD_INTERRUPT     (1<<8)
#define SDHC_CARD_REMOVAL       (1<<7)
#define SDHC_CARD_INSERTION     (1<<6)
#define SDHC_BUFFER_READ_READY      (1<<5)
#define SDHC_BUFFER_WRITE_READY     (1<<4)
#define SDHC_DMA_INTERRUPT      (1<<3)
#define SDHC_BLOCK_GAP_EVENT        (1<<2)
#define SDHC_TRANSFER_COMPLETE      (1<<1)
#define SDHC_COMMAND_COMPLETE       (1<<0)
#define SDHC_NINTR_STATUS_MASK      0x81ff
#define SDHC_EINTR_STATUS       0x32
#define SDHC_ADMA_ERROR         (1<<9)
#define SDHC_AUTO_CMD12_ERROR       (1<<8)
#define SDHC_CURRENT_LIMIT_ERROR    (1<<7)
#define SDHC_DATA_END_BIT_ERROR     (1<<6)
#define SDHC_DATA_CRC_ERROR     (1<<5)
#define SDHC_DATA_TIMEOUT_ERROR     (1<<4)
#define SDHC_DATA_ERROR         0x70
#define SDHC_CMD_INDEX_ERROR        (1<<3)
#define SDHC_CMD_END_BIT_ERROR      (1<<2)
#define SDHC_CMD_CRC_ERROR      (1<<1)
#define SDHC_CMD_TIMEOUT_ERROR      (1<<0)
#define SDHC_CMD_ERROR          0x0f
#define SDHC_EINTR_STATUS_MASK      0x03ff  /* excluding vendor signals */
#define SDHC_NINTR_STATUS_EN        0x34
#define SDHC_EINTR_STATUS_EN        0x36
#define SDHC_NINTR_SIGNAL_EN        0x38
#define SDHC_NINTR_SIGNAL_MASK      0x01ff
#define SDHC_EINTR_SIGNAL_EN        0x3a
#define SDHC_EINTR_SIGNAL_MASK      0x03ff  /* excluding vendor signals */
#define SDHC_CMD12_ERROR_STATUS     0x3c
#define SDHC_CAPABILITIES       0x40
#define SDHC_VOLTAGE_SUPP_1_8V      (1<<26)
#define SDHC_VOLTAGE_SUPP_3_0V      (1<<25)
#define SDHC_VOLTAGE_SUPP_3_3V      (1<<24)
#define SDHC_DMA_SUPPORT        (1<<22)
#define SDHC_HIGH_SPEED_SUPP        (1<<21)
#define SDHC_BASE_FREQ_SHIFT        8
#define SDHC_BASE_FREQ_MASK     0x3f
#define SDHC_BASE_FREQ_MASK_V3      0xff
#define SDHC_TIMEOUT_FREQ_UNIT      (1<<7)  /* 0=KHz, 1=MHz */
#define SDHC_TIMEOUT_FREQ_SHIFT     0
#define SDHC_TIMEOUT_FREQ_MASK      0x1f
#define SDHC_MAX_CAPABILITIES       0x48
#define SDHC_SLOT_INTR_STATUS       0xfc
#define SDHC_HOST_CTL_VERSION       0xfe
#define SDHC_SPEC_VERS_SHIFT        0
#define SDHC_SPEC_VERS_MASK     0xff
#define SDHC_VENDOR_VERS_SHIFT      8
#define SDHC_VENDOR_VERS_MASK       0xff
#define  SDHC_SPEC_V1           0
#define  SDHC_SPEC_V2           1
#define  SDHC_SPEC_V3           2

/* SDHC_CLOCK_CTL encoding */
#define SDHC_SDCLK_DIV(div)                     \
    (((div) & SDHC_SDCLK_DIV_MASK) << SDHC_SDCLK_DIV_SHIFT)
#define SDHC_SDCLK_DIV_V3(div)                      \
    (SDHC_SDCLK_DIV(div) |                      \
    (((div) & SDHC_SDCLK_DIV_MASK_V3) >> SDHC_SDCLK_DIV_RSHIFT_V3))

/* SDHC_CAPABILITIES decoding */
#define SDHC_BASE_FREQ_KHZ(cap)                     \
    ((((cap) >> SDHC_BASE_FREQ_SHIFT) & SDHC_BASE_FREQ_MASK) * 1000)
#define SDHC_BASE_FREQ_KHZ_V3(cap)                  \
    ((((cap) >> SDHC_BASE_FREQ_SHIFT) & SDHC_BASE_FREQ_MASK_V3) * 1000)
#define SDHC_TIMEOUT_FREQ(cap)                      \
    (((cap) >> SDHC_TIMEOUT_FREQ_SHIFT) & SDHC_TIMEOUT_FREQ_MASK)
#define SDHC_TIMEOUT_FREQ_KHZ(cap)                  \
    (((cap) & SDHC_TIMEOUT_FREQ_UNIT) ?             \
        SDHC_TIMEOUT_FREQ(cap) * 1000:              \
        SDHC_TIMEOUT_FREQ(cap))

/* SDHC_HOST_CTL_VERSION decoding */
#define SDHC_SPEC_VERSION(hcv)                      \
    (((hcv) >> SDHC_SPEC_VERS_SHIFT) & SDHC_SPEC_VERS_MASK)
#define SDHC_VENDOR_VERSION(hcv)                    \
    (((hcv) >> SDHC_VENDOR_VERS_SHIFT) & SDHC_VENDOR_VERS_MASK)

struct sdmmc_command;

int sdhc_host_reset(struct sdhc_host *hp);
int sdhc_card_detect(struct sdhc_host *hp);
int sdhc_bus_power(struct sdhc_host *hp, u_int32_t);
int sdhc_bus_clock(struct sdhc_host *hp, int, int);
int sdhc_bus_width(struct sdhc_host *hp, int);
void sdhc_card_intr_mask(struct sdhc_host *hp, int);
void sdhc_card_intr_ack(struct sdhc_host *hp);

void sdhc_exec_command(struct sdhc_host *hp, struct sdmmc_command *);

void sdhc_async_command(struct sdhc_host *hp, struct sdmmc_command *);
void sdhc_async_response(struct sdhc_host *hp, struct sdmmc_command *);

#endif
