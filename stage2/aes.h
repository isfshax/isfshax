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

#ifndef __AES_H__
#define __AES_H__

#include "types.h"

#define AES_BLOCK_SIZE  16

void aes_reset(void);
void aes_set_iv(u8 *iv);
void aes_empty_iv();
void aes_set_key(u8 *key);
void aes_decrypt(u8 *src, u8 *dst, u32 blocks, u8 keep_iv);
void aes_encrypt(u8 *src, u8 *dst, u32 blocks, u8 keep_iv);

#endif /* __AES_H__ */
