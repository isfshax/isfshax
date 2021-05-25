/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
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

#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include "types.h"

typedef struct
{
    // Bank 0 (Wii)
    struct {
        u8 wii_boot1_hash[20];
        u8 wii_common_key[16];
        u32 wii_ng_id;
        union {
             struct {
                 u8 wii_ng_priv[30];
                 u8 _wii_wtf1[18];
             };
             struct {
                 u8 _wii_wtf2[28];
                 u8 wii_nand_hmac[20];
             };
        };
        u8 wii_nand_key[16];
        u8 wii_rng_key[16];
        u32 wii_unk1;
        u32 wii_unk2; // 0x00000007
    };

    // Bank 1 (Wii U)
    struct {
        u32 security_level;
        u32 iostrength_flag;
        u32 seeprom_pulse;
        u32 unk1;
        u8 fw_ancast_key[16];
        u8 seeprom_key[16];
        u8 unk2[16];
        u8 unk3[16];
        u8 vwii_common_key[16];
        u8 common_key[16];
        u8 unk4[16];
    };

    // Bank 2 (Wii U)
    struct {
        u8 unk5[16];
        u8 unk6[16];
        u8 ssl_master_key[16];
        u8 external_master_key[16];
        u8 unk7[16];
        u8 xor_key[16];
        u8 rng_key[16];
        u8 nand_key[16];
    };

    // Bank 3 (Wii U)
    struct {
        u8 emmc_key[16];
        u8 dev_master_key[16];
        u8 drh_key[16];
        u8 unk8[48];
        u8 nand_hmac[20];
        u8 unk9[12];
    };

    // Bank 4 (Wii U)
    struct {
        u8 unk10[16];
        union {
            u8 external_seed_full[16];
            struct {
                u8 _wtf1[12];
                u8 external_seed[4];
            };
        };
        u8 vwii_ng_priv[32];
        u8 unk11[32];
        union {
            u8 rng_seed_full[16];
            struct {
                u32 rng_seed;
                u8 _wtf2[12];
            };
        };
        u8 unk12[16];
    };

    // Bank 5 (Wii U)
    struct {
        u32 rootca_version;
        u32 rootca_ms;
        u32 unk13;
        u8 rootca_signature[64];
        u8 unk14[20];
        u8 unk15[32];
    };

    // Bank 6 (Wii SEEPROM)
    struct {
        u8 wii_seeprom_certificate[96];
        u8 wii_seeprom_signature[32];
    };

    // Bank 7 (Misc.)
    struct {
        u8 unk16[32];
        u8 boot1_key[16];
        u8 unk17[16];
        u8 _empty1[32];
        u8 unk18[16];
        char ascii_tag[12];
        u32 jtag_status;
    };
} __attribute__((packed, aligned(4))) otp_t;

_Static_assert(sizeof(otp_t) == 0x400, "OTP size must be 0x400!");

extern otp_t otp;

void crypto_read_otp();

#endif
