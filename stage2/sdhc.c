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

/*
 * SD Host Controller driver based on the SD Host Controller Standard
 * Simplified Specification Version 1.00 (www.sdcard.com).
 */

#include "bsdtypes.h"
#include "memory.h"
#include "utils.h"
#include "sdmmc.h"
#include "sdhc.h"
#include <string.h>
#include "debug.h"

#ifdef CAN_HAZ_IRQ
#include "irq.h"
#endif

//#define SDHC_DEBUG

#define SDHC_COMMAND_TIMEOUT    500
#define SDHC_TRANSFER_TIMEOUT   5000

#define sdhc_wait_intr(a,b,c) sdhc_wait_intr_debug(__func__, __LINE__, a, b, c)

static inline u32 bus_space_read_4(bus_space_handle_t ioh, u32 reg)
{
    return read32(ioh + reg);
}

static inline u16 bus_space_read_2(bus_space_handle_t ioh, u32 reg)
{
    if(reg & 3)
        return (read32((ioh + reg) & ~3) & 0xffff0000) >> 16;
    else
        return (read32(ioh + reg) & 0xffff);
}

static inline u8 bus_space_read_1(bus_space_handle_t ioh, u32 reg)
{
    u32 mask;
    u32 addr;
    u8 shift;

    shift = (reg & 3) * 8;
    mask = (0xFF << shift);
    addr = ioh + reg;

    return (read32(addr & ~3) & mask) >> shift;
}

static inline void bus_space_write_4(bus_space_handle_t ioh, u32 r, u32 v)
{
    write32(ioh + r, v);
}

static inline void bus_space_write_2(bus_space_handle_t ioh, u32 r, u16 v)
{
    if(r & 3)
        mask32((ioh + r) & ~3, 0xffff0000, v << 16);
    else
        mask32((ioh + r), 0xffff, ((u32)v));
}

static inline void bus_space_write_1(bus_space_handle_t ioh, u32 r, u8 v)
{
    u32 mask;
    u32 addr;
    u8 shift;

    shift = (r & 3) * 8;
    mask = (0xFF << shift);
    addr = ioh + r;

    mask32(addr & ~3, mask, v << shift);
}

/* flag values */
#define SHF_USE_DMA     0x0001

#define HREAD1(hp, reg)                         \
    (bus_space_read_1((hp)->ioh, (reg)))
#define HREAD2(hp, reg)                         \
    (bus_space_read_2((hp)->ioh, (reg)))
#define HREAD4(hp, reg)                         \
    (bus_space_read_4((hp)->ioh, (reg)))
#define HWRITE1(hp, reg, val)                       \
    bus_space_write_1((hp)->ioh, (reg), (val))
#define HWRITE2(hp, reg, val)                       \
    bus_space_write_2((hp)->ioh, (reg), (val))
#define HWRITE4(hp, reg, val)                       \
    bus_space_write_4((hp)->ioh, (reg), (val))
#define HCLR1(hp, reg, bits)                        \
    HWRITE1((hp), (reg), HREAD1((hp), (reg)) & ~(bits))
#define HCLR2(hp, reg, bits)                        \
    HWRITE2((hp), (reg), HREAD2((hp), (reg)) & ~(bits))
#define HSET1(hp, reg, bits)                        \
    HWRITE1((hp), (reg), HREAD1((hp), (reg)) | (bits))
#define HSET2(hp, reg, bits)                        \
    HWRITE2((hp), (reg), HREAD2((hp), (reg)) | (bits))

int sdhc_start_command(struct sdhc_host *, struct sdmmc_command *);
int sdhc_wait_state(struct sdhc_host *, u_int32_t, u_int32_t);
int sdhc_soft_reset(struct sdhc_host *, int);
void sdhc_reset_intr_status(struct sdhc_host *hp);
int sdhc_wait_intr_debug(const char *func, int line, struct sdhc_host *, int, int);
void    sdhc_transfer_data(struct sdhc_host *, struct sdmmc_command *);
void    sdhc_read_data(struct sdhc_host *, u_char *, int);
void    sdhc_write_data(struct sdhc_host *, u_char *, int);
#ifdef SDHC_DEBUG
int sdhcdebug = 3;
#define DPRINTF(n,s)    do { if ((n) <= sdhcdebug) printf s; } while (0)
void    sdhc_dump_regs(struct sdhc_host *);
#else
#define DPRINTF(n,s)    do {} while(0)
#endif

/*
 * Called by attachment driver.  For each SD card slot there is one SD
 * host controller standard register set. (1.3)
 */
int
sdhc_host_found(struct sdhc_host *hp, struct sdhc_host_params *pa, bus_space_tag_t iot, bus_space_handle_t ioh, int usedma)
{
    u_int32_t caps;
    int error = 1;
    int max_clock;

#ifdef SDHC_DEBUG
    u_int16_t version;

    version = HREAD2(hp, SDHC_HOST_CTL_VERSION);
    DEBUG("sdhc: SD Host Specification/Vendor Version ");

    switch(SDHC_SPEC_VERSION(version)) {
    case 0x00:
        DEBUG("1.0/%u\n", SDHC_VENDOR_VERSION(version));
        break;
    default:
        DEBUG(">1.0/%u\n", SDHC_VENDOR_VERSION(version));
        break;
    }
#endif

    memset(hp, 0, sizeof(struct sdhc_host));

    /* Fill in the new host structure. */
    hp->iot = iot;
    hp->ioh = ioh;
    hp->data_command = 0;

    memcpy(&hp->pa, pa, sizeof(struct sdhc_host_params));

    /* Store specification version. */
    hp->version = HREAD2(hp, SDHC_HOST_CTL_VERSION);

    /*
     * Reset the host controller and enable interrupts.
     */
    (void)sdhc_host_reset(hp);

    /* Determine host capabilities. */
    caps = HREAD4(hp, SDHC_CAPABILITIES);

    /* Use DMA if the host system and the controller support it. */
    if (usedma && ISSET(caps, SDHC_DMA_SUPPORT))
        SET(hp->flags, SHF_USE_DMA);

    /*
     * Determine the base clock frequency. (2.2.24)
     */
    if (SDHC_SPEC_VERSION(hp->version) >= SDHC_SPEC_V3) {
        /* SDHC 3.0 supports 10-255 MHz. */
        max_clock = 255000;
        if (SDHC_BASE_FREQ_KHZ_V3(caps) != 0)
            hp->clkbase = SDHC_BASE_FREQ_KHZ_V3(caps);
    } else {
        /* SDHC 1.0/2.0 supports only 10-63 MHz. */
        max_clock = 63000;
        if (SDHC_BASE_FREQ_KHZ(caps) != 0)
            hp->clkbase = SDHC_BASE_FREQ_KHZ(caps);
    }
    if (hp->clkbase == 0) {
        /* The attachment driver must tell us. */
        DEBUG("sdhc: base clock frequency unknown\n");
        goto err;
    } else if (hp->clkbase < 10000 || hp->clkbase > max_clock) {
        DEBUG("sdhc: base clock frequency out of range: %u MHz\n",
            hp->clkbase / 1000);
        goto err;
    }

    DEBUG("sdhc: SDHC %d.0, %d MHz base clock\n",
        SDHC_SPEC_VERSION(hp->version) + 1, hp->clkbase / 1000);

    /*
     * Determine SD bus voltage levels supported by the controller.
     */
    if (ISSET(caps, SDHC_VOLTAGE_SUPP_1_8V))
        SET(hp->ocr, MMC_OCR_1_9V_2_0V);
    if (ISSET(caps, SDHC_VOLTAGE_SUPP_3_0V))
        SET(hp->ocr, MMC_OCR_2_9V_3_0V | MMC_OCR_3_0V_3_1V);
    if (ISSET(caps, SDHC_VOLTAGE_SUPP_3_3V))
        SET(hp->ocr, MMC_OCR_3_2V_3_3V | MMC_OCR_3_3V_3_4V);

    /*
     * Attach the generic SD/MMC bus driver.  (The bus driver must
     * not invoke any chipset functions before it is attached.)
     */
    hp->pa.attach(hp);

    return 0;

err:
    return (error);
}

#ifndef LOADER
/*
 * Shutdown hook established by or called from attachment driver.
 */
void
sdhc_shutdown(struct sdhc_host *hp)
{
    /* XXX chip locks up if we don't disable it before reboot. */
    (void)sdhc_host_reset(hp);
}
#endif

/*
 * Reset the host controller.  Called during initialization, when
 * cards are removed, upon resume, and during error recovery.
 */
int
sdhc_host_reset(struct sdhc_host *hp)
{
    u_int16_t imask;
    int error;

    /* Disable all interrupts. */
    HWRITE2(hp, SDHC_NINTR_SIGNAL_EN, 0);

    /*
     * Reset the entire host controller and wait up to 100ms for
     * the controller to clear the reset bit.
     */
    if ((error = sdhc_soft_reset(hp, SDHC_RESET_ALL)) != 0) {
        return (error);
    }

    /* Set data timeout counter value to max for now. */
    HWRITE1(hp, SDHC_TIMEOUT_CTL, SDHC_TIMEOUT_MAX);

    /* Enable interrupts. */
    imask =
#ifndef LOADER
        SDHC_CARD_REMOVAL | SDHC_CARD_INSERTION |
#endif
        SDHC_BUFFER_READ_READY | SDHC_BUFFER_WRITE_READY |
        SDHC_DMA_INTERRUPT | SDHC_BLOCK_GAP_EVENT |
        SDHC_TRANSFER_COMPLETE | SDHC_COMMAND_COMPLETE;

    HWRITE2(hp, SDHC_NINTR_STATUS_EN, imask);
    HWRITE2(hp, SDHC_EINTR_STATUS_EN, SDHC_EINTR_STATUS_MASK);
    HWRITE2(hp, SDHC_NINTR_SIGNAL_EN, imask);
    HWRITE2(hp, SDHC_EINTR_SIGNAL_EN, SDHC_EINTR_SIGNAL_MASK);

    return 0;
}

/*
 * Return non-zero if the card is currently inserted.
 */
int
sdhc_card_detect(struct sdhc_host *hp)
{
    return ISSET(HREAD4(hp, SDHC_PRESENT_STATE), SDHC_CARD_INSERTED) ?
        1 : 0;
}

/*
 * Set or change SD bus voltage and enable or disable SD bus power.
 * Return zero on success.
 */
int
sdhc_bus_power(struct sdhc_host *hp, u_int32_t ocr)
{
    u_int8_t vdd;
    DEBUG("sdhc_bus_power(0x%lx)\n", ocr);

    /* Disable bus power before voltage change. */
    HWRITE1(hp, SDHC_POWER_CTL, 0);

    /* If power is disabled, reset the host and return now. */
    if (ocr == 0) {
        (void)sdhc_host_reset(hp);
        return 0;
    }

    /*
     * Select the maximum voltage according to capabilities.
     */
    ocr &= hp->ocr;
    if (ISSET(ocr, MMC_OCR_3_2V_3_3V|MMC_OCR_3_3V_3_4V))
        vdd = SDHC_VOLTAGE_3_3V;
    else if (ISSET(ocr, MMC_OCR_2_9V_3_0V|MMC_OCR_3_0V_3_1V))
        vdd = SDHC_VOLTAGE_3_0V;
    else if (ISSET(ocr, MMC_OCR_1_9V_2_0V))
        vdd = SDHC_VOLTAGE_1_8V;
    else {
        /* Unsupported voltage level requested. */
        return EINVAL;
    }

    /*
     * Enable bus power.  Wait at least 1 ms (or 74 clocks) plus
     * voltage ramp until power rises.
     */
    HWRITE1(hp, SDHC_POWER_CTL, (vdd << SDHC_VOLTAGE_SHIFT) |
        SDHC_BUS_POWER);
    udelay(10000);

    /*
     * The host system may not power the bus due to battery low,
     * etc.  In that case, the host controller should clear the
     * bus power bit.
     */
    if (!ISSET(HREAD1(hp, SDHC_POWER_CTL), SDHC_BUS_POWER)) {
        DEBUG("Host controller failed to enable bus power\n");
        return ENXIO;
    }

    return 0;
}

/*
 * Return the smallest possible base clock frequency divisor value
 * for the CLOCK_CTL register to produce `freq' (KHz).
 */
static int
sdhc_clock_divisor(struct sdhc_host *hp, u_int freq)
{
    int max_div = 256;
    int div;

    if (SDHC_SPEC_VERSION(hp->version) >= SDHC_SPEC_V3)
        max_div = 2046;

    for (div = 1; div <= max_div; div *= 2)
        if ((hp->clkbase / div) <= freq)
            return (div / 2);
    /* No divisor found. */
    return -1;
}

/*
 * Set or change SDCLK frequency or disable the SD clock.
 * Return zero on success.
 */
int
sdhc_bus_clock(struct sdhc_host *hp, int freq, int timing)
{
    int div;
    int timo;
    int sdclk;

    DEBUG("%s(%d, %d)\n", __FUNCTION__, freq, timing);
#ifdef DIAGNOSTIC
    /* Must not stop the clock if commands are in progress. */
    if (ISSET(HREAD4(hp, SDHC_PRESENT_STATE), SDHC_CMD_INHIBIT_MASK) &&
        sdhc_card_detect(hp))
        DEBUG("sdhc_sdclk_frequency_select: command in progress\n");
#endif

    /* Stop SD clock before changing the frequency. */
    HWRITE2(hp, SDHC_CLOCK_CTL, 0);
    if (freq == SDMMC_SDCLK_OFF)
        return 0;

    if (timing == SDMMC_TIMING_LEGACY)
        HCLR1(hp, SDHC_HOST_CTL, SDHC_HIGH_SPEED);
    else
        HSET1(hp, SDHC_HOST_CTL, SDHC_HIGH_SPEED);

    /* Set the minimum base clock frequency divisor. */
    if ((div = sdhc_clock_divisor(hp, freq)) < 0) {
        /* Invalid base clock frequency or `freq' value. */
        return EINVAL;
    }
    if (SDHC_SPEC_VERSION(hp->version) >= SDHC_SPEC_V3)
        sdclk = SDHC_SDCLK_DIV_V3(div);
    else
        sdclk = SDHC_SDCLK_DIV(div);
    HWRITE2(hp, SDHC_CLOCK_CTL, sdclk);

    /* Start internal clock.  Wait 10ms for stabilization. */
    HSET2(hp, SDHC_CLOCK_CTL, SDHC_INTCLK_ENABLE);
    for (timo = 1000; timo > 0; timo--) {
        if (ISSET(HREAD2(hp, SDHC_CLOCK_CTL), SDHC_INTCLK_STABLE))
            break;
        udelay(10);
    }
    if (timo == 0) {
        DEBUG("sdhc: internal clock never stabilized\n");
        return ETIMEDOUT;
    }

    /* Enable SD clock. */
    HSET2(hp, SDHC_CLOCK_CTL, SDHC_SDCLK_ENABLE);

    return 0;
}

int
sdhc_bus_width(struct sdhc_host *hp, int width)
{
    int reg;

    DEBUG("%s(%d)\n", __FUNCTION__, width);

    if (width != 1 && width != 4 && width != 8)
        return EINVAL;

    reg = HREAD1(hp, SDHC_HOST_CTL);
    reg &= ~(SDHC_4BIT_MODE | SDHC_8BIT_MODE);

    if (width == 4) {
        reg |= SDHC_4BIT_MODE;
    } else if (width == 8) {
        reg |= SDHC_8BIT_MODE;
    }
    HWRITE1(hp, SDHC_HOST_CTL, reg);

    return 0;
}

void
sdhc_card_intr_mask(struct sdhc_host *hp, int enable)
{
    if (enable) {
        HSET2(hp, SDHC_NINTR_STATUS_EN, SDHC_CARD_INTERRUPT);
        HSET2(hp, SDHC_NINTR_SIGNAL_EN, SDHC_CARD_INTERRUPT);
    } else {
        HCLR2(hp, SDHC_NINTR_SIGNAL_EN, SDHC_CARD_INTERRUPT);
        HCLR2(hp, SDHC_NINTR_STATUS_EN, SDHC_CARD_INTERRUPT);
    }
}

void
sdhc_card_intr_ack(struct sdhc_host *hp)
{
    HSET2(hp, SDHC_NINTR_STATUS_EN, SDHC_CARD_INTERRUPT);
}

int
sdhc_wait_state(struct sdhc_host *hp, u_int32_t mask, u_int32_t value)
{
    u_int32_t state;
    int timeout;

    for (timeout = 500; timeout > 0; timeout--) {
        if (((state = HREAD4(hp, SDHC_PRESENT_STATE)) & mask)
            == value)
            return 0;
        udelay(10000);
    }
    DPRINTF(0,("sdhc: timeout waiting for %x (state=%d)\n", value, state));
    return ETIMEDOUT;
}

void
sdhc_exec_command(struct sdhc_host *hp, struct sdmmc_command *cmd)
{
    int error;

    if (cmd->c_datalen > 0)
        hp->data_command = 1;

    if (cmd->c_timeout == 0) {
        if (cmd->c_datalen > 0)
            cmd->c_timeout = SDHC_TRANSFER_TIMEOUT;
        else
            cmd->c_timeout = SDHC_COMMAND_TIMEOUT;
    }

    hp->intr_status = 0;

    /*
     * Start the MMC command, or mark `cmd' as failed and return.
     */
    error = sdhc_start_command(hp, cmd);
    if (error != 0) {
        cmd->c_error = error;
        SET(cmd->c_flags, SCF_ITSDONE);
        hp->data_command = 0;
        return;
    }

    /*
     * Wait until the command phase is done, or until the command
     * is marked done for any other reason.
     */

    int status = sdhc_wait_intr(hp, SDHC_COMMAND_COMPLETE, cmd->c_timeout);
    if (!ISSET(status, SDHC_COMMAND_COMPLETE)) {
        cmd->c_error = ETIMEDOUT;
        DEBUG("timeout dump: error_intr: 0x%x intr: 0x%x\n", hp->intr_error_status, hp->intr_status);
//      sdhc_dump_regs(hp);
        SET(cmd->c_flags, SCF_ITSDONE);
        hp->data_command = 0;
        return;
    }

//  DEBUG("command_complete, continuing...\n");

    /*
     * The host controller removes bits [0:7] from the response
     * data (CRC) and we pass the data up unchanged to the bus
     * driver (without padding).
     */
    if (cmd->c_error == 0 && ISSET(cmd->c_flags, SCF_RSP_PRESENT)) {
        if (ISSET(cmd->c_flags, SCF_RSP_136)) {
            u_char *p = (u_char *)cmd->c_resp;
            int i;

            for (i = 0; i < 15; i++)
                *p++ = HREAD1(hp, SDHC_RESPONSE + i);
        } else
            cmd->c_resp[0] = HREAD4(hp, SDHC_RESPONSE);
    }

    /*
     * If the command has data to transfer in any direction,
     * execute the transfer now.
     */
    if (cmd->c_error == 0 && cmd->c_datalen > 0)
        sdhc_transfer_data(hp, cmd);

    DPRINTF(1,("sdhc: cmd %u done (flags=%#x error=%d prev state=%d)\n",
        cmd->c_opcode, cmd->c_flags, cmd->c_error, (cmd->c_resp[0] >> 9) & 15));
    SET(cmd->c_flags, SCF_ITSDONE);
    hp->data_command = 0;
}

int
sdhc_start_command(struct sdhc_host *hp, struct sdmmc_command *cmd)
{
    u_int16_t blksize = 0;
    u_int16_t blkcount = 0;
    u_int16_t mode;
    u_int16_t command;
    int error;

    DPRINTF(1,("sdhc: start cmd %u arg=%#x data=%p dlen=%d flags=%#x\n",
        cmd->c_opcode, cmd->c_arg, cmd->c_data, cmd->c_datalen, cmd->c_flags));

    /*
     * The maximum block length for commands should be the minimum
     * of the host buffer size and the card buffer size. (1.7.2)
     */

    /* Fragment the data into proper blocks. */
    if (cmd->c_datalen > 0) {
        blksize = MIN(cmd->c_datalen, cmd->c_blklen);
        blkcount = cmd->c_datalen / blksize;
        if (cmd->c_datalen % blksize > 0) {
            /* XXX: Split this command. (1.7.4) */
            DEBUG("sdhc: data not a multiple of %d bytes\n", blksize);
            return EINVAL;
        }
    }

    /* Check limit imposed by 9-bit block count. (1.7.2) */
    if (blkcount > SDHC_BLOCK_COUNT_MAX) {
        DEBUG("sdhc: too much data\n");
        return EINVAL;
    }

    /* Prepare transfer mode register value. (2.2.5) */
    mode = 0;
    if (ISSET(cmd->c_flags, SCF_CMD_READ))
        mode |= SDHC_READ_MODE;
    if (blkcount > 0) {
        mode |= SDHC_BLOCK_COUNT_ENABLE;
        if (blkcount > 1) {
            mode |= SDHC_MULTI_BLOCK_MODE;
            /* XXX only for memory commands? */
            mode |= SDHC_AUTO_CMD12_ENABLE;
        }
    }
    if (ISSET(hp->flags, SHF_USE_DMA))
        mode |= SDHC_DMA_ENABLE;

    /*
     * Prepare command register value. (2.2.6)
     */
    command = (cmd->c_opcode & SDHC_COMMAND_INDEX_MASK) <<
        SDHC_COMMAND_INDEX_SHIFT;

    if (ISSET(cmd->c_flags, SCF_RSP_CRC))
        command |= SDHC_CRC_CHECK_ENABLE;
    if (ISSET(cmd->c_flags, SCF_RSP_IDX))
        command |= SDHC_INDEX_CHECK_ENABLE;
    if (cmd->c_data != NULL)
        command |= SDHC_DATA_PRESENT_SELECT;

    if (!ISSET(cmd->c_flags, SCF_RSP_PRESENT))
        command |= SDHC_NO_RESPONSE;
    else if (ISSET(cmd->c_flags, SCF_RSP_136))
        command |= SDHC_RESP_LEN_136;
    else if (ISSET(cmd->c_flags, SCF_RSP_BSY))
        command |= SDHC_RESP_LEN_48_CHK_BUSY;
    else
        command |= SDHC_RESP_LEN_48;

    /* Wait until command and data inhibit bits are clear. (1.5) */
    if ((error = sdhc_wait_state(hp, SDHC_CMD_INHIBIT_MASK, 0)) != 0)
        return error;

    if (ISSET(hp->flags, SHF_USE_DMA) && cmd->c_datalen > 0) {
        cmd->c_resid = blkcount;
        cmd->c_buf = cmd->c_data;

        if (ISSET(cmd->c_flags, SCF_CMD_READ) == 0) {
            dc_flushrange(cmd->c_data, cmd->c_datalen);
            ahb_flush_to(hp->pa.rb);
        }
        HWRITE4(hp, SDHC_DMA_ADDR, (u32)cmd->c_data);
    }

    DPRINTF(1,("sdhc: cmd=%#x mode=%#x blksize=%d blkcount=%d\n",
        command, mode, blksize, blkcount));

    /*
     * Start a CPU data transfer.  Writing to the high order byte
     * of the SDHC_COMMAND register triggers the SD command. (1.5)
     */
//  HWRITE2(hp, SDHC_TRANSFER_MODE, mode);
    HWRITE2(hp, SDHC_BLOCK_SIZE, blksize | 7 << 12);
    HWRITE2(hp, SDHC_BLOCK_COUNT, blkcount);
    HWRITE4(hp, SDHC_ARGUMENT, cmd->c_arg);
//  http://wiibrew.org/wiki/Reversed_Little_Endian
//  HWRITE2(hp, SDHC_COMMAND, command);
    HWRITE4(hp, SDHC_TRANSFER_MODE, ((u32)command << 16) | mode);

    return 0;
}

void
sdhc_transfer_data(struct sdhc_host *hp, struct sdmmc_command *cmd)
{
    int error;
    int status;

    error = 0;

    DPRINTF(1,("resp=%#x datalen=%d\n", MMC_R1(cmd->c_resp), cmd->c_datalen));
    if (ISSET(hp->flags, SHF_USE_DMA)) {
        for(;;) {
            status = sdhc_wait_intr(hp, SDHC_TRANSFER_COMPLETE |
                    SDHC_DMA_INTERRUPT,
                    SDHC_TRANSFER_TIMEOUT);
            if (!status) {
                DEBUG("DMA timeout %08x\n", status);
                error = ETIMEDOUT;
                break;
            }

            if (ISSET(status, SDHC_TRANSFER_COMPLETE)) {
//              DEBUG("got a TRANSFER_COMPLETE: %08x\n", status);
                break;
            }
        }
    } else
        DEBUG("fail.\n");



#ifdef SDHC_DEBUG
    /* XXX I forgot why I wanted to know when this happens :-( */
    if ((cmd->c_opcode == 52 || cmd->c_opcode == 53) &&
        ISSET(MMC_R1(cmd->c_resp), 0xcb00))
        DEBUG("sdhc: CMD52/53 error response flags %#x\n",
            MMC_R1(cmd->c_resp) & 0xff00);
#endif
    if (ISSET(cmd->c_flags, SCF_CMD_READ)) {
        ahb_flush_from(hp->pa.wb);
        dc_invalidaterange(cmd->c_data, cmd->c_datalen);
    }

    if (error != 0)
        cmd->c_error = error;
    SET(cmd->c_flags, SCF_ITSDONE);

    DPRINTF(1,("sdhc: data transfer done (error=%d)\n", cmd->c_error));
    return;
}

/* Prepare for another command. */
int
sdhc_soft_reset(struct sdhc_host *hp, int mask)
{
    int timo;

    DPRINTF(1,("sdhc: software reset reg=%#x\n", mask));

    HWRITE1(hp, SDHC_SOFTWARE_RESET, mask);
    for (timo = 10; timo > 0; timo--) {
        if (!ISSET(HREAD1(hp, SDHC_SOFTWARE_RESET), mask))
            break;
        udelay(10000);
        HWRITE1(hp, SDHC_SOFTWARE_RESET, 0);
    }
    if (timo == 0) {
        DPRINTF(1,("sdhc: timeout reg=%#x\n", HREAD1(hp, SDHC_SOFTWARE_RESET)));
        HWRITE1(hp, SDHC_SOFTWARE_RESET, 0);
        return (ETIMEDOUT);
    }
    return (0);
}

int
sdhc_wait_intr_debug(const char *funcname, int line, struct sdhc_host *hp, int mask, int timo)
{
    (void) funcname;
    (void) line;

    int status;

    mask |= SDHC_ERROR_INTERRUPT;
    mask |= SDHC_ERROR_TIMEOUT;

    status = hp->intr_status & mask;

    for (; timo > 0; timo--) {
#ifdef CAN_HAZ_IRQ
        if((get_cpsr() & 0b11111) == 0b10010)
#endif
            sdhc_intr(hp); // seems backwards but ok

        if (hp->intr_status != 0) {
            status = hp->intr_status & mask;
            break;
        }
        udelay(1000);
    }

    if (timo == 0) {
        status |= SDHC_ERROR_TIMEOUT;
    }
    hp->intr_status &= ~status;

    DPRINTF(2,("sdhc: funcname=%s, line=%d, timo=%d status=%#x intr status=%#x error %#x\n",
        funcname, line, timo, status, hp->intr_status, hp->intr_error_status));

    /* Command timeout has higher priority than command complete. */
    if (ISSET(status, SDHC_ERROR_INTERRUPT)) {
        DEBUG("resetting due to error interrupt\n");
//      sdhc_dump_regs(hp);

        hp->intr_error_status = 0;
        (void)sdhc_soft_reset(hp, SDHC_RESET_DAT|SDHC_RESET_CMD);
        status = 0;
    }

    /* Command timeout has higher priority than command complete. */
    if (ISSET(status, SDHC_ERROR_TIMEOUT)) {
        DEBUG("resetting due to timeout\n");
//      sdhc_dump_regs(hp);

        hp->intr_error_status = 0;
        (void)sdhc_soft_reset(hp, SDHC_RESET_DAT|SDHC_RESET_CMD);
        status = 0;
    }

    return status;
}

/*
 * Established by attachment driver at interrupt priority IPL_SDMMC.
 */
int
sdhc_intr(struct sdhc_host *hp)
{
    u_int16_t status;
    u_int16_t error;
    u_int16_t signal;

    DPRINTF(1,("sdhc_intr():\n"));
//  sdhc_dump_regs(hp);

    /* Find out which interrupts are pending. */
    status = HREAD2(hp, SDHC_NINTR_STATUS);
    if (!ISSET(status, SDHC_NINTR_STATUS_MASK)) {
        DPRINTF(1, ("unknown interrupt\n"));
        return 0;
    }

    error = HREAD2(hp, SDHC_EINTR_STATUS);
    signal = HREAD2(hp, SDHC_EINTR_SIGNAL_EN);

    /* Acknowledge the interrupts we are about to handle. */
    HWRITE2(hp, SDHC_NINTR_STATUS, status);
    DPRINTF(2,("sdhc: interrupt status=%d\n", status));

    /* Service error interrupts. */
    if (ISSET(status, SDHC_ERROR_INTERRUPT)) {
        /* Acknowledge error interrupts. */
        HWRITE2(hp, SDHC_EINTR_SIGNAL_EN, 0);
        (void)sdhc_soft_reset(hp, SDHC_RESET_DAT|SDHC_RESET_CMD);
        if (hp->data_command == 1) {
            hp->data_command = 0;
            hp->pa.abort();
        }
        HWRITE2(hp, SDHC_EINTR_STATUS, error);
        HWRITE2(hp, SDHC_EINTR_SIGNAL_EN, signal);

        DPRINTF(2,("sdhc: error interrupt, status=0x%x, signal=0x%x\n", error, signal));

        if (ISSET(error, SDHC_CMD_TIMEOUT_ERROR|
            SDHC_DATA_TIMEOUT_ERROR)) {
            hp->intr_error_status |= error;
            hp->intr_status |= status;
        }
    }

    /*
     * Wake up the blocking process to service command
     * related interrupt(s).
     */
    if (ISSET(status, SDHC_BUFFER_READ_READY|
        SDHC_BUFFER_WRITE_READY|SDHC_COMMAND_COMPLETE|
         SDHC_TRANSFER_COMPLETE)) {
        hp->intr_status |= status;
    }

    if (ISSET(status, SDHC_DMA_INTERRUPT)) {
        DPRINTF(2,("sdhc: dma left:%#x\n", HREAD2(hp, SDHC_BLOCK_COUNT)));
        // this works because our virtual memory
        // addresses are equal to the physical memory
        // addresses and because we require the target
        // buffer to be contiguous
        HWRITE4(hp, SDHC_DMA_ADDR, HREAD4(hp, SDHC_DMA_ADDR));
    }

    /* Service SD card interrupts. */
    if  (ISSET(status, SDHC_CARD_INTERRUPT)) {
        DPRINTF(0,("sdhc: card interrupt\n"));
        HCLR2(hp, SDHC_NINTR_STATUS_EN, SDHC_CARD_INTERRUPT);
    }

    /*
     * Wake up the sdmmc event thread to scan for cards.
     */
    if (ISSET(status, SDHC_CARD_REMOVAL|SDHC_CARD_INSERTION))
        hp->pa.attach(hp);

    return 1;
}

#ifdef SDHC_DEBUG
void
sdhc_dump_regs(struct sdhc_host *hp)
{
    DEBUG("0x%02x PRESENT_STATE:    %x\n", SDHC_PRESENT_STATE,
        HREAD4(hp, SDHC_PRESENT_STATE));
    DEBUG("0x%02x POWER_CTL:        %x\n", SDHC_POWER_CTL,
        HREAD1(hp, SDHC_POWER_CTL));
    DEBUG("0x%02x NINTR_STATUS:     %x\n", SDHC_NINTR_STATUS,
        HREAD2(hp, SDHC_NINTR_STATUS));
    DEBUG("0x%02x EINTR_STATUS:     %x\n", SDHC_EINTR_STATUS,
        HREAD2(hp, SDHC_EINTR_STATUS));
    DEBUG("0x%02x NINTR_STATUS_EN:  %x\n", SDHC_NINTR_STATUS_EN,
        HREAD2(hp, SDHC_NINTR_STATUS_EN));
    DEBUG("0x%02x EINTR_STATUS_EN:  %x\n", SDHC_EINTR_STATUS_EN,
        HREAD2(hp, SDHC_EINTR_STATUS_EN));
    DEBUG("0x%02x NINTR_SIGNAL_EN:  %x\n", SDHC_NINTR_SIGNAL_EN,
        HREAD2(hp, SDHC_NINTR_SIGNAL_EN));
    DEBUG("0x%02x EINTR_SIGNAL_EN:  %x\n", SDHC_EINTR_SIGNAL_EN,
        HREAD2(hp, SDHC_EINTR_SIGNAL_EN));
    DEBUG("0x%02x CAPABILITIES:     %x\n", SDHC_CAPABILITIES,
        HREAD4(hp, SDHC_CAPABILITIES));
    DEBUG("0x%02x MAX_CAPABILITIES: %x\n", SDHC_MAX_CAPABILITIES,
        HREAD4(hp, SDHC_MAX_CAPABILITIES));
}
#endif
