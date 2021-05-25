#include "aes.h"
#include "latte.h"
#include "utils.h"
#include "memory.h"
#include "string.h"

#define     AES_CMD_RESET   0
#define     AES_CMD_ENCRYPT 0x9000
#define     AES_CMD_DECRYPT 0x9800

static inline void aes_command(u16 cmd, u8 iv_keep, u32 blocks)
{
    if (blocks != 0)
        blocks--;
    write32(AES_CTRL, (cmd << 16) | (iv_keep ? 0x1000 : 0) | (blocks&0x7f));
    while (read32(AES_CTRL) & 0x80000000);
}

void aes_reset(void)
{
    write32(AES_CTRL, 0);
    while (read32(AES_CTRL) != 0);
}

void aes_set_iv(u8 *iv)
{
    int i;
    for(i = 0; i < 4; i++) {
        write32(AES_IV, *(u32 *)iv);
        iv += 4;
    }
}

void aes_empty_iv(void)
{
    int i;
    for(i = 0; i < 4; i++)
        write32(AES_IV, 0);
}

void aes_set_key(u8 *key)
{
    int i;
    for(i = 0; i < 4; i++) {
        write32(AES_KEY, *(u32 *)key);
        key += 4;
    }
}

void aes_decrypt(u8 *src, u8 *dst, u32 blocks, u8 keep_iv)
{
    dc_flushrange(src, blocks * 16);
    dc_invalidaterange(dst, blocks * 16);
    ahb_flush_to(RB_AES);

    int this_blocks = 0;
    while(blocks > 0) {
        this_blocks = blocks;
        if (this_blocks > 0x80)
            this_blocks = 0x80;

        write32(AES_SRC, dma_addr(src));
        write32(AES_DEST, dma_addr(dst));

        aes_command(AES_CMD_DECRYPT, keep_iv, this_blocks);

        blocks -= this_blocks;
        src += this_blocks<<4;
        dst += this_blocks<<4;
        keep_iv = 1;
    }

    ahb_flush_from(WB_AES);
    ahb_flush_to(RB_IOD);
}

void aes_encrypt(u8 *src, u8 *dst, u32 blocks, u8 keep_iv)
{
    dc_flushrange(src, blocks * 16);
    dc_invalidaterange(dst, blocks * 16);
    ahb_flush_to(RB_AES);
    
    int this_blocks = 0;
    while(blocks > 0) {
        this_blocks = blocks;
        if (this_blocks > 0x80)
            this_blocks = 0x80;
        
        write32(AES_SRC, dma_addr(src));
        write32(AES_DEST, dma_addr(dst));
        
        aes_command(AES_CMD_ENCRYPT, keep_iv, this_blocks);
        
        blocks -= this_blocks;
        src += this_blocks<<4;
        dst += this_blocks<<4;
        keep_iv = 1;
    }
    
    ahb_flush_from(WB_AES);
    ahb_flush_to(RB_IOD);
}
