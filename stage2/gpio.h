/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef __GPIO_H__
#define __GPIO_H__

enum {
    GP_POWER            = 0x00000001,
    GP_SHUTDOWN         = 0x00000002,
    GP_FAN              = 0x00000004,
    GP_DCDC             = 0x00000008,
    GP_DISPIN           = 0x00000010,
    GP_SLOTLED          = 0x00000020,
    GP_EJECTBTN         = 0x00000040,
    GP_SLOTIN           = 0x00000080,
    GP_SENSORBAR        = 0x00000100,
    GP_DOEJECT          = 0x00000200,
    GP_EEP_CS           = 0x00000400,
    GP_EEP_CLK          = 0x00000800,
    GP_EEP_MOSI         = 0x00001000,
    GP_EEP_MISO         = 0x00002000,
    GP_AV0_SCL          = 0x00004000,
    GP_AV0_SDA          = 0x00008000,
    GP_DEBUG0           = 0x00010000,
    GP_DEBUG1           = 0x00020000,
    GP_DEBUG2           = 0x00040000,
    GP_DEBUG3           = 0x00080000,
    GP_DEBUG4           = 0x00100000,
    GP_DEBUG5           = 0x00200000,
    GP_DEBUG6           = 0x00400000,
    GP_DEBUG7           = 0x00800000,
    GP_AV1_SCL          = 0x01000000,
    GP_AV1_SDA          = 0x02000000,
    GP_MUTELAMP         = 0x04000000,
    GP_BT_MODE          = 0x08000000,
    GP_CCRH_RST         = 0x10000000,
    GP_WIFI_MODE        = 0x20000000,
    GP_SDSLOT0_PWR      = 0x40000000,
};

enum {
    GP2_FANSPEED        = 0x00000001,
    GP2_SMC_SCL         = 0x00000002,
    GP2_SMC_SDA         = 0x00000004,
    GP2_DCDC2           = 0x00000008,
    GP2_AV_INT          = 0x00000010,
    GP2_CCRIO12         = 0x00000020,
    GP2_AV_RST          = 0x00000040,
};

#define GP_DEBUG_SHIFT 16
#define GP_DEBUG_MASK 0xFF0000

#define GP_ALL 0xFFFFFF
#define GP_OWNER_PPC (GP_AVE_SDA | GP_AVE_SCL | GP_DOEJECT | GP_SENSORBAR | GP_SLOTIN | GP_SLOTLED)
#define GP_OWNER_ARM (GP_ALL ^ GP_OWNER_PPC)
#define GP_INPUTS (GP_POWER | GP_EJECTBTN | GP_SLOTIN | GP_EEP_MISO | GP_AVE_SDA)
#define GP_OUTPUTS (GP_ALL ^ GP_INPUTS)
#define GP_ARM_INPUTS (GP_INPUTS & GP_OWNER_ARM)
#define GP_PPC_INPUTS (GP_INPUTS & GP_OWNER_PPC)
#define GP_ARM_OUTPUTS (GP_OUTPUTS & GP_OWNER_ARM)
#define GP_PPC_OUTPUTS (GP_OUTPUTS & GP_OWNER_PPC)
#define GP_DEFAULT_ON (GP_AVE_SCL | GP_DCDC | GP_FAN)
#define GP_ARM_DEFAULT_ON (GP_DEFAULT_ON & GP_OWNER_ARM)
#define GP_PPC_DEFAULT_ON (GP_DEFAULT_ON & GP_OWNER_PPC)

#endif

