/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef _LATTE_H
#define _LATTE_H

/*
 * Latte registers.
 * http://wiiubrew.org/wiki/Hardware/Latte_Registers
 */
#define LT_REG_BASE                   (0x0D800000)

#define LT_IPC_PPCMSG_COMPAT          (LT_REG_BASE + 0x000)
#define LT_IPC_PPCCTRL_COMPAT         (LT_REG_BASE + 0x004)
#define LT_IPC_ARMMSG_COMPAT          (LT_REG_BASE + 0x008)
#define LT_IPC_ARMCTRL_COMPAT         (LT_REG_BASE + 0x00C)

#define LT_TIMER                      (LT_REG_BASE + 0x010)
#define LT_ALARM                      (LT_REG_BASE + 0x014)

#define LT_INTSR_PPC_COMPAT           (LT_REG_BASE + 0x030)
#define LT_INTMR_PPC_COMPAT           (LT_REG_BASE + 0x034)
#define LT_INTSR_ARM_COMPAT           (LT_REG_BASE + 0x038)
#define LT_INTMR_ARM_COMPAT           (LT_REG_BASE + 0x03C)
#define LT_INTMR_ARM2X_COMPAT         (LT_REG_BASE + 0x040)

#define LT_UNK044                     (LT_REG_BASE + 0x044)
#define LT_AHB_WDG_STATUS             (LT_REG_BASE + 0x048)
#define LT_AHB_WDG_CONFIG             (LT_REG_BASE + 0x04C)
#define LT_AHB_DMA_STATUS             (LT_REG_BASE + 0x050)
#define LT_AHB_CPU_STATUS             (LT_REG_BASE + 0x054)
#define LT_ERROR                      (LT_REG_BASE + 0x058)
#define LT_ERROR_MASK                 (LT_REG_BASE + 0x05C)
#define LT_MEMIRR                     (LT_REG_BASE + 0x060)
#define LT_AHBPROT                    (LT_REG_BASE + 0x064)
#define LT_UNK068                     (LT_REG_BASE + 0x068)
#define LT_UNK06C                     (LT_REG_BASE + 0x06C)
#define LT_EXICTRL                    (LT_REG_BASE + 0x070)
#define LT_UNK074                     (LT_REG_BASE + 0x074)
#define LT_UNK088                     (LT_REG_BASE + 0x088)

#define LT_GPIOE_OUT                  (LT_REG_BASE + 0x0C0)
#define LT_GPIOE_DIR                  (LT_REG_BASE + 0x0C4)
#define LT_GPIOE_IN                   (LT_REG_BASE + 0x0C8)
#define LT_GPIOE_INTLVL               (LT_REG_BASE + 0x0CC)
#define LT_GPIOE_INTFLAG              (LT_REG_BASE + 0x0D0)
#define LT_GPIOE_INTMASK              (LT_REG_BASE + 0x0D4)
#define LT_GPIOE_INMIR                (LT_REG_BASE + 0x0D8)

#define LT_GPIO_ENABLE                (LT_REG_BASE + 0x0DC)
#define LT_GPIO_OUT                   (LT_REG_BASE + 0x0E0)
#define LT_GPIO_DIR                   (LT_REG_BASE + 0x0E4)
#define LT_GPIO_IN                    (LT_REG_BASE + 0x0E8)
#define LT_GPIO_INTLVL                (LT_REG_BASE + 0x0EC)
#define LT_GPIO_INTFLAG               (LT_REG_BASE + 0x0F0)
#define LT_GPIO_INTMASK               (LT_REG_BASE + 0x0F4)
#define LT_GPIO_INMIR                 (LT_REG_BASE + 0x0F8)
#define LT_GPIO_OWNER                 (LT_REG_BASE + 0x0FC)

#define LT_AHB_UNK100                 (LT_REG_BASE + 0x100)
#define LT_AHB_UNK104                 (LT_REG_BASE + 0x104)
#define LT_AHB_UNK108                 (LT_REG_BASE + 0x108)
#define LT_AHB_UNK10C                 (LT_REG_BASE + 0x10C)
#define LT_AHB_UNK110                 (LT_REG_BASE + 0x110)
#define LT_AHB_UNK114                 (LT_REG_BASE + 0x114)
#define LT_AHB_UNK118                 (LT_REG_BASE + 0x118)
#define LT_AHB_UNK11C                 (LT_REG_BASE + 0x11C)
#define LT_AHB_UNK120                 (LT_REG_BASE + 0x120)
#define LT_AHB_UNK124                 (LT_REG_BASE + 0x124)
#define LT_AHB_UNK130                 (LT_REG_BASE + 0x130)
#define LT_AHB_UNK134                 (LT_REG_BASE + 0x134)
#define LT_AHB_UNK138                 (LT_REG_BASE + 0x138)

#define LT_ARB_CFG                    (LT_REG_BASE + 0x140)
#define LT_DIFLAGS                    (LT_REG_BASE + 0x180)
#define LT_RESETS_AHB                 (LT_REG_BASE + 0x184)
#define LT_COMPAT_MEMCTRL_WORKAROUND  (LT_REG_BASE + 0x188)
#define LT_BOOT0                      (LT_REG_BASE + 0x18C)
#define LT_CLOCKINFO                  (LT_REG_BASE + 0x190)
#define LT_RESETS_COMPAT              (LT_REG_BASE + 0x194)
#define LT_CLOCKGATE_COMPAT           (LT_REG_BASE + 0x198)
#define LT_SATA_UNK1A8                (LT_REG_BASE + 0x1A8)
#define LT_SATA_UNK1C8                (LT_REG_BASE + 0x1C8)
#define LT_SATA_UNK1CC                (LT_REG_BASE + 0x1CC)
#define LT_SATA_UNK1D0                (LT_REG_BASE + 0x1D0)
#define LT_UNK1D8                     (LT_REG_BASE + 0x1D8)
#define LT_IOPOWER                    (LT_REG_BASE + 0x1DC)
#define LT_IOSTRENGTH_CTRL0           (LT_REG_BASE + 0x1E0)
#define LT_IOSTRENGTH_CTRL1           (LT_REG_BASE + 0x1E4)
#define LT_ACRCLK_STRENGTH_CTRL       (LT_REG_BASE + 0x1E8)
#define LT_OTPCMD                     (LT_REG_BASE + 0x1EC)
#define LT_OTPDATA                    (LT_REG_BASE + 0x1F0)
#define LT_UNK204                     (LT_REG_BASE + 0x204)
#define LT_ASICREV_ACR                (LT_REG_BASE + 0x214)
#define LT_UNK224                     (LT_REG_BASE + 0x224)
#define LT_UNK250                     (LT_REG_BASE + 0x250)
#define LT_UNK254                     (LT_REG_BASE + 0x254)
#define LT_UNK258                     (LT_REG_BASE + 0x258)

#define LT_IPC_PPC0_PPCMSG            (LT_REG_BASE + 0x400)
#define LT_IPC_PPC0_PPCCTRL           (LT_REG_BASE + 0x404)
#define LT_IPC_PPC0_ARMMSG            (LT_REG_BASE + 0x408)
#define LT_IPC_PPC0_ARMCTRL           (LT_REG_BASE + 0x40C)
#define LT_IPC_PPC1_PPCMSG            (LT_REG_BASE + 0x410)
#define LT_IPC_PPC1_PPCCTRL           (LT_REG_BASE + 0x414)
#define LT_IPC_PPC1_ARMMSG            (LT_REG_BASE + 0x418)
#define LT_IPC_PPC1_ARMCTRL           (LT_REG_BASE + 0x41C)
#define LT_IPC_PPC2_PPCMSG            (LT_REG_BASE + 0x420)
#define LT_IPC_PPC2_PPCCTRL           (LT_REG_BASE + 0x424)
#define LT_IPC_PPC2_ARMMSG            (LT_REG_BASE + 0x428)
#define LT_IPC_PPC2_ARMCTRL           (LT_REG_BASE + 0x42C)

#define LT_INTSR_AHBALL_PPC0          (LT_REG_BASE + 0x440)
#define LT_INTSR_AHBLT_PPC0           (LT_REG_BASE + 0x444)
#define LT_INTMR_AHBALL_PPC0          (LT_REG_BASE + 0x448)
#define LT_INTMR_AHBLT_PPC0           (LT_REG_BASE + 0x44C)
#define LT_INTSR_AHBALL_PPC1          (LT_REG_BASE + 0x450)
#define LT_INTSR_AHBLT_PPC1           (LT_REG_BASE + 0x454)
#define LT_INTMR_AHBALL_PPC1          (LT_REG_BASE + 0x458)
#define LT_INTMR_AHBLT_PPC1           (LT_REG_BASE + 0x45C)
#define LT_INTSR_AHBALL_PPC2          (LT_REG_BASE + 0x460)
#define LT_INTSR_AHBLT_PPC2           (LT_REG_BASE + 0x464)
#define LT_INTMR_AHBALL_PPC2          (LT_REG_BASE + 0x468)
#define LT_INTMR_AHBLT_PPC2           (LT_REG_BASE + 0x46C)
#define LT_INTSR_AHBALL_ARM           (LT_REG_BASE + 0x470)
#define LT_INTSR_AHBLT_ARM            (LT_REG_BASE + 0x474)
#define LT_INTMR_AHBALL_ARM           (LT_REG_BASE + 0x478)
#define LT_INTMR_AHBLT_ARM            (LT_REG_BASE + 0x47C)
#define LT_INTMR_AHBALL_ARM2X         (LT_REG_BASE + 0x480)
#define LT_INTMR_AHBLT_ARM2X          (LT_REG_BASE + 0x484)

#define LT_AHB2_WDG_STATUS            (LT_REG_BASE + 0x4A0)
#define LT_AHB2_DMA_STATUS            (LT_REG_BASE + 0x4A4)
#define LT_AHB2_CPU_STATUS            (LT_REG_BASE + 0x4A8)

#define LT_UNK4C8                     (LT_REG_BASE + 0x4C8)
#define LT_UNK4CC                     (LT_REG_BASE + 0x4CC)
#define LT_UNK4D0                     (LT_REG_BASE + 0x4D0)
#define LT_UNK4D4                     (LT_REG_BASE + 0x4D4)
#define LT_UNK4DC                     (LT_REG_BASE + 0x4DC)
#define LT_UNK4E0                     (LT_REG_BASE + 0x4E0)
#define LT_UNK4E4                     (LT_REG_BASE + 0x4E4)
#define LT_UNK500                     (LT_REG_BASE + 0x500)
#define LT_UNK504                     (LT_REG_BASE + 0x504)

#define LT_OTPPROT                    (LT_REG_BASE + 0x510)
#define LT_SYSPROT                    (LT_REG_BASE + 0x514)

#define LT_GPIOE2_OUT                 (LT_REG_BASE + 0x520)
#define LT_GPIOE2_DIR                 (LT_REG_BASE + 0x524)
#define LT_GPIOE2_IN                  (LT_REG_BASE + 0x528)
#define LT_GPIOE2_INTLVL              (LT_REG_BASE + 0x52C)
#define LT_GPIOE2_INTFLAG             (LT_REG_BASE + 0x530)
#define LT_GPIOE2_INTMASK             (LT_REG_BASE + 0x534)
#define LT_GPIOE2_INMIR               (LT_REG_BASE + 0x538)

#define LT_GPIO2_ENABLE               (LT_REG_BASE + 0x53C)
#define LT_GPIO2_OUT                  (LT_REG_BASE + 0x540)
#define LT_GPIO2_DIR                  (LT_REG_BASE + 0x544)
#define LT_GPIO2_IN                   (LT_REG_BASE + 0x548)
#define LT_GPIO2_INTLVL               (LT_REG_BASE + 0x54C)
#define LT_GPIO2_INTFLAG              (LT_REG_BASE + 0x550)
#define LT_GPIO2_INTMASK              (LT_REG_BASE + 0x554)
#define LT_GPIO2_INMIR                (LT_REG_BASE + 0x558)
#define LT_GPIO2_OWNER                (LT_REG_BASE + 0x55C)

#define LT_I2C_CLOCK                  (LT_REG_BASE + 0x570)
#define LT_I2C_INOUT_DATA             (LT_REG_BASE + 0x574)
#define LT_I2C_INOUT_CTRL             (LT_REG_BASE + 0x578)
#define LT_I2C_INOUT_SIZE             (LT_REG_BASE + 0x57C)
#define LT_I2C_INT_MASK               (LT_REG_BASE + 0x580)
#define LT_I2C_INT_STATE              (LT_REG_BASE + 0x584)

#define LT_ASICREV_CCR                (LT_REG_BASE + 0x5A0)
#define LT_DEBUG                      (LT_REG_BASE + 0x5A4)
#define LT_COMPAT_MEMCTRL_STATE       (LT_REG_BASE + 0x5B0)
#define LT_COMPAT_AHB_STATE           (LT_REG_BASE + 0x5B4)
#define LT_COMPAT_STEREO_OUT_SELECT   (LT_REG_BASE + 0x5B8)
#define LT_IOP2X                      (LT_REG_BASE + 0x5BC)
#define LT_UNK5C0                     (LT_REG_BASE + 0x5C0)
#define LT_IOSTRENGTH_CTRL2           (LT_REG_BASE + 0x5C8)
#define LT_UNK5CC                     (LT_REG_BASE + 0x5CC)
#define LT_RESETS                     (LT_REG_BASE + 0x5E0)
#define LT_RESETS_AHMN                (LT_REG_BASE + 0x5E4)
#define LT_CLOCKGATE                  (LT_REG_BASE + 0x5E8)
#define LT_SYSPLL_CFG                 (LT_REG_BASE + 0x5EC)
#define LT_ABIF_CPLTL_OFFSET          (LT_REG_BASE + 0x620)
#define LT_ABIF_CPLTL_DATA            (LT_REG_BASE + 0x624)
#define LT_UNK628                     (LT_REG_BASE + 0x628)
#define LT_60XE_CFG                   (LT_REG_BASE + 0x640)
#define LT_UNK660                     (LT_REG_BASE + 0x660)
#define LT_UNK640                     (LT_REG_BASE + 0x640)
#define LT_DCCMPT                     (LT_REG_BASE + 0x708)

/*
 * NAND registers.
 * http://wiiubrew.org/wiki/Hardware/NAND_Interface
 */
#define NAND_REG_BASE       (0x0D010000)

#define NAND_CTRL           (NAND_REG_BASE + 0x000)
#define NAND_CONF           (NAND_REG_BASE + 0x004)
#define NAND_ADDR0          (NAND_REG_BASE + 0x008)
#define NAND_ADDR1          (NAND_REG_BASE + 0x00C)
#define NAND_DATA           (NAND_REG_BASE + 0x010)
#define NAND_ECC            (NAND_REG_BASE + 0x014)
#define NAND_BANK           (NAND_REG_BASE + 0x018)
#define NAND_UNK1           (NAND_REG_BASE + 0x01C)
#define NAND_BANK_CTRL      (NAND_REG_BASE + 0x030)
#define NAND_UNK3           (NAND_REG_BASE + 0x040)

/*
 * AES registers.
 * http://wiiubrew.org/wiki/Hardware/AES_Engine
 */
#define AES_REG_BASE        (0x0D020000)

#define AES_CTRL            (AES_REG_BASE + 0x000)
#define AES_SRC             (AES_REG_BASE + 0x004)
#define AES_DEST            (AES_REG_BASE + 0x008)
#define AES_KEY             (AES_REG_BASE + 0x00C)
#define AES_IV              (AES_REG_BASE + 0x010)

/*
 * SHA-1 registers.
 * http://wiiubrew.org/wiki/Hardware/SHA-1_Engine
 */
#define SHA_REG_BASE        (0x0D030000)

#define SHA_CTRL            (SHA_REG_BASE + 0x000)
#define SHA_SRC             (SHA_REG_BASE + 0x004)
#define SHA_H0              (SHA_REG_BASE + 0x008)
#define SHA_H1              (SHA_REG_BASE + 0x00C)
#define SHA_H2              (SHA_REG_BASE + 0x010)
#define SHA_H3              (SHA_REG_BASE + 0x014)
#define SHA_H4              (SHA_REG_BASE + 0x018)

/*
 * SD Host Controller registers.
 * http://wiiubrew.org/wiki/Hardware/SD_Host_Controller
 */
#define SD0_REG_BASE        (0x0D070000)
#define SD1_REG_BASE        (0x0D080000)
#define SD2_REG_BASE        (0x0D100000)
#define SD3_REG_BASE        (0x0D110000)

/*
 * OHCI registers.
 * http://wiiubrew.org/wiki/Hardware/USB_Host_Controller
 */
#define OHCI0_REG_BASE      (0x0D050000)
#define OHCI1_REG_BASE      (0x0D060000)
#define OHCI10_REG_BASE     (0x0D130000)
#define OHCI20_REG_BASE     (0x0D150000)

/*
 * EHCI registers.
 * http://wiiubrew.org/wiki/Hardware/USB_Host_Controller
 */
#define EHCI0_REG_BASE      (0x0D040000)
#define EHCI1_REG_BASE      (0x0D120000)
#define EHCI2_REG_BASE      (0x0D140000)

/*
 * EXI registers.
 * http://wiiubrew.org/wiki/Hardware/Legacy#External_Interface
 */
#define EXI_REG_BASE        (0x0D806800)

#define EXI0_REG_BASE       (EXI_REG_BASE  + 0x000)
#define EXI1_REG_BASE       (EXI_REG_BASE  + 0x014)
#define EXI2_REG_BASE       (EXI_REG_BASE  + 0x028)
#define EXIBOOT_REG_BASE    (EXI_REG_BASE  + 0x040)

#define EXI0_CSR            (EXI0_REG_BASE + 0x000)
#define EXI0_MAR            (EXI0_REG_BASE + 0x004)
#define EXI0_LENGTH         (EXI0_REG_BASE + 0x008)
#define EXI0_CR             (EXI0_REG_BASE + 0x00C)
#define EXI0_DATA           (EXI0_REG_BASE + 0x010)

#define EXI1_CSR            (EXI1_REG_BASE + 0x000)
#define EXI1_MAR            (EXI1_REG_BASE + 0x004)
#define EXI1_LENGTH         (EXI1_REG_BASE + 0x008)
#define EXI1_CR             (EXI1_REG_BASE + 0x00C)
#define EXI1_DATA           (EXI1_REG_BASE + 0x010)

#define EXI2_CSR            (EXI2_REG_BASE + 0x000)
#define EXI2_MAR            (EXI2_REG_BASE + 0x004)
#define EXI2_LENGTH         (EXI2_REG_BASE + 0x008)
#define EXI2_CR             (EXI2_REG_BASE + 0x00C)
#define EXI2_DATA           (EXI2_REG_BASE + 0x010)

/*
 * Memory Controller registers.
 * http://wiiubrew.org/wiki/Hardware/Memory_Controller
 */
#define MEM_REG_BASE        (0x0D8B4000)

#define MEM_PROT            (MEM_REG_BASE + 0x20A)
#define MEM_PROT_START      (MEM_REG_BASE + 0x20C)
#define MEM_PROT_END        (MEM_REG_BASE + 0x20E)
#define MEM_REFRESH_FLAG    (MEM_REG_BASE + 0x226)
#define MEM_FLUSH_MASK      (MEM_REG_BASE + 0x228)
#define MEM_FLUSH_ACK       (MEM_REG_BASE + 0x22A)
#define MEM_SEQ_REG_VAL     (MEM_REG_BASE + 0x2C4)
#define MEM_SEQ_REG_ADDR    (MEM_REG_BASE + 0x2C6)
#define MEM_SEQ0_REG_VAL    (MEM_REG_BASE + 0x300)
#define MEM_SEQ0_REG_ADDR   (MEM_REG_BASE + 0x302)

/*
 * AHMN registers.
 * http://wiiubrew.org/wiki/Hardware/XN_Controller
 */
#define AHMN_REG_BASE       (0x0D8B0800)

#define AHMN_MEM0_CONFIG    (AHMN_REG_BASE + 0x000)
#define AHMN_MEM1_CONFIG    (AHMN_REG_BASE + 0x004)
#define AHMN_MEM2_CONFIG    (AHMN_REG_BASE + 0x008)
#define AHMN_RDBI_MASK      (AHMN_REG_BASE + 0x00C)
#define AHMN_ERROR_MASK     (AHMN_REG_BASE + 0x020)
#define AHMN_ERROR          (AHMN_REG_BASE + 0x024)
#define AHMN_UNK40          (AHMN_REG_BASE + 0x040)
#define AHMN_UNK44          (AHMN_REG_BASE + 0x044)
#define AHMN_TRANSFER_STATE (AHMN_REG_BASE + 0x050)
#define AHMN_WORKAROUND     (AHMN_REG_BASE + 0x054)

#define AHMN_MEM0           (AHMN_REG_BASE + 0x100)
#define AHMN_MEM1           (AHMN_REG_BASE + 0x200)
#define AHMN_MEM2           (AHMN_REG_BASE + 0x400)

#endif
