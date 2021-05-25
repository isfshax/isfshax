/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "types.h"
#include "memory.h"
#include "utils.h"
#include "latte.h"
#include "irq.h"
#include "debug.h"
#include "malloc.h"

void _dc_inval_entries(void *start, int count);
void _dc_flush_entries(const void *start, int count);
void _dc_flush(void);
void _ic_inval(void);
void _drain_write_buffer(void);

#ifndef LOADER
static u32 *__page_table;
void _dc_inval(void);
void _tlb_inval(void);
#endif

#define LINESIZE 0x20
#define CACHESIZE 0x4000

#define CR_MMU      (1 << 0)
#define CR_DCACHE   (1 << 2)
#define CR_ICACHE   (1 << 12)

// this is ripped from IOS, because no one can figure out just WTF this thing is doing
void _ahb_flush_to(enum rb_client dev) {
    u32 mask;
    switch(dev) {
        case RB_IOD: mask = 0x8000; break;
        case RB_IOI: mask = 0x4000; break;
        case RB_AIM: mask = 0x0001; break;
        case RB_FLA: mask = 0x0002; break;
        case RB_AES: mask = 0x0004; break;
        case RB_SHA: mask = 0x0008; break;
        case RB_EHCI: mask = 0x0010; break;
        case RB_OHCI0: mask = 0x0020; break;
        case RB_OHCI1: mask = 0x0040; break;
        case RB_SD0: mask = 0x0080; break;
        case RB_SD1: mask = 0x0100; break;
        default:
            // ahb_flush_to() does this now
            //DEBUG("_ahb_flush_to(%d): Invalid device\n", dev);
            return;
    }

    //NOTE: 0xd8b000x, not 0xd8b400x!
    if(read32(0xd8b0008) & mask) {
        return;
    }

    switch(dev) {
        case RB_FLA:
        case RB_AES:
        case RB_SHA:
        case RB_EHCI:
        case RB_OHCI0:
        case RB_OHCI1:
        case RB_SD0:
        case RB_SD1:
            while((read32(LT_BOOT0) & 0xF) == 9)
                set32(LT_COMPAT_MEMCTRL_WORKAROUND, 0x10000);
            clear32(LT_COMPAT_MEMCTRL_WORKAROUND, 0x10000);
            set32(LT_COMPAT_MEMCTRL_WORKAROUND, 0x2000000);
            mask32(LT_AHB_UNK124, 0x7C0, 0x280);
            set32(LT_AHB_UNK134, 0x400);
            while((read32(LT_BOOT0) & 0xF) != 9);
            set32(LT_AHB_UNK100, 0x400);
            set32(LT_AHB_UNK104, 0x400);
            set32(LT_AHB_UNK108, 0x400);
            set32(LT_AHB_UNK10C, 0x400);
            set32(LT_AHB_UNK110, 0x400);
            set32(LT_AHB_UNK114, 0x400);
            set32(LT_AHB_UNK118, 0x400);
            set32(LT_AHB_UNK11C, 0x400);
            set32(LT_AHB_UNK120, 0x400);
            clear32(0xd8b0008, mask);
            set32(0xd8b0008, mask);
            clear32(LT_AHB_UNK134, 0x400);
            clear32(LT_AHB_UNK100, 0x400);
            clear32(LT_AHB_UNK104, 0x400);
            clear32(LT_AHB_UNK108, 0x400);
            clear32(LT_AHB_UNK10C, 0x400);
            clear32(LT_AHB_UNK110, 0x400);
            clear32(LT_AHB_UNK114, 0x400);
            clear32(LT_AHB_UNK118, 0x400);
            clear32(LT_AHB_UNK11C, 0x400);
            clear32(LT_AHB_UNK120, 0x400);
            clear32(LT_COMPAT_MEMCTRL_WORKAROUND, 0x2000000);
            mask32(LT_AHB_UNK124, 0x7C0, 0xC0);
            break;
        case RB_IOD:
        case RB_IOI:
            set32(0xd8b0008, mask);
            break;
        default:
            break;
    }
}

// invalidate device and then starlet
void ahb_flush_to(enum rb_client dev)
{
    u32 mask;
    switch(dev) {
        case RB_IOD: mask = 0x8000; break;
        case RB_IOI: mask = 0x4000; break;
        case RB_AIM: mask = 0x0001; break;
        case RB_FLA: mask = 0x0002; break;
        case RB_AES: mask = 0x0004; break;
        case RB_SHA: mask = 0x0008; break;
        case RB_EHCI: mask = 0x0010; break;
        case RB_OHCI0: mask = 0x0020; break;
        case RB_OHCI1: mask = 0x0040; break;
        case RB_SD0: mask = 0x0080; break;
        case RB_SD1: mask = 0x0100; break;
        case RB_SD2: mask = 0x10000; break;
        case RB_SD3: mask = 0x20000; break;
        case RB_EHC1: mask = 0x40000; break;
        case RB_OHCI10: mask = 0x80000; break;
        case RB_EHC2: mask = 0x100000; break;
        case RB_OHCI20: mask = 0x200000; break;
        case RB_SATA: mask = 0x400000; break;
        case RB_AESS: mask = 0x800000; break;
        case RB_SHAS: mask = 0x1000000; break;
        default:
            DEBUG("ahb_flush_to(%d): Invalid device\n", dev);
            return;
    }

    u32 cookie = irq_kill();

    write32(AHMN_RDBI_MASK, mask);

    _ahb_flush_to(dev);
    if(dev != RB_IOD)
        _ahb_flush_to(RB_IOD);

    irq_restore(cookie);
}

// flush device and also invalidate memory
void ahb_flush_from(enum wb_client dev)
{
    u32 cookie = irq_kill();
    u16 req = 0;
    bool done = false;
    int i;

    switch(dev)
    {
        case WB_IOD:
            req = 0b0001;
            break;
        case WB_AIM:
        case WB_EHCI:
        case WB_EHC1:
        case WB_EHC2:
        case WB_SATA:
        case WB_DMAB:
            req = 0b0100;
            break;
        case WB_FLA:
        case WB_OHCI0:
        case WB_OHCI1:
        case WB_SD0:
        case WB_SD1:
        case WB_SD2:
        case WB_SD3:
        case WB_OHCI10:
        case WB_OHCI20:
        case WB_DMAC:
            req = 0b1000;
            break;
        case WB_AES:
        case WB_SHA:
        case WB_AESS:
        case WB_SHAS:
        case WB_DMAA:
            req = 0b0010;
            break;
        case WB_ALL:
            req = 0b1111;
            break;
        default:
            DEBUG("ahb_flush(%d): Invalid device\n", dev);
            goto done;
    }

    write16(MEM_FLUSH_MASK, req);

    for(i = 0; i < 1000000; i++) {
        if(!(read16(MEM_FLUSH_MASK) & req)) {
            done = true;
            break;
        }
        udelay(1);
    }

    if(!done) {
        DEBUG("ahb_flush(%d): Flush (0x%x) did not ack!\n", dev, req);
    }
done:
    irq_restore(cookie);
}

void dc_flushrange(const void *start, u32 size)
{
    u32 cookie = irq_kill();
    if(size > 0x4000) {
        _dc_flush();
    } else {
        void *end = ALIGN_FORWARD(((u8*)start) + size, LINESIZE);
        start = ALIGN_BACKWARD(start, LINESIZE);
        _dc_flush_entries(start, (end - start) / LINESIZE);
    }
    _drain_write_buffer();
    ahb_flush_from(WB_AIM);
    irq_restore(cookie);
}

void dc_invalidaterange(void *start, u32 size)
{
    u32 cookie = irq_kill();
    void *end = ALIGN_FORWARD(((u8*)start) + size, LINESIZE);
    start = ALIGN_BACKWARD(start, LINESIZE);
    _dc_inval_entries(start, (end - start) / LINESIZE);
    ahb_flush_to(RB_IOD);
    irq_restore(cookie);
}

void dc_flushall(void)
{
    u32 cookie = irq_kill();
    _dc_flush();
    _drain_write_buffer();
    ahb_flush_from(WB_AIM);
    irq_restore(cookie);
}

void ic_invalidateall(void)
{
    u32 cookie = irq_kill();
    _ic_inval();
    ahb_flush_to(RB_IOD);
    irq_restore(cookie);
}

void mem_protect(int enable, void *start, void *end)
{
    write16(MEM_PROT, enable?1:0);
    write16(MEM_PROT_START, (((u32)start) & 0xFFFFFFF) >> 12);
    write16(MEM_PROT_END, (((u32)end) & 0xFFFFFFF) >> 12);
    udelay(10);
}

void mem_setswap(int enable)
{
    u32 d = read32(LT_MEMIRR);

    if((d & 0x20) && !enable)
        write32(LT_MEMIRR, d & ~0x20);
    if((!(d & 0x20)) && enable)
        write32(LT_MEMIRR, d | 0x20);
}

#ifndef LOADER
u32 dma_addr(void *p)
{
    u32 addr = (u32)p;

    switch(addr>>20) {
        case 0xfff:
        case 0x0d4:
        case 0x0dc:
            if(read32(LT_MEMIRR) & 0x20) {
                addr ^= 0x10000;
            }
            addr &= 0x0001FFFF;
            addr |= 0x0d400000;
            break;
    }
    //DEBUG("DMA to %p: address %08x\n", p, addr);
    return addr;
}

#define SECTION             0x012

#define NONBUFFERABLE       0x000
#define BUFFERABLE          0x004
#define WRITETHROUGH_CACHE  0x008
#define WRITEBACK_CACHE     0x00C

#define DOMAIN(x)           ((x)<<5)

#define AP_ROM              0x000
#define AP_NOUSER           0x400
#define AP_ROUSER           0x800
#define AP_RWUSER           0xC00

// from, to, size: units of 1MB
void map_section(u32 from, u32 to, u32 size, u32 attributes)
{
    attributes |= SECTION;
    while(size--) {
        __page_table[from++] = (to++<<20) | attributes;
    }
}

//#define NO_CACHES

void mem_initialize(void)
{
    u32 cr;
    u32 cookie = irq_kill();

    DEBUG("MEM: cleaning up\n");

    _ic_inval();
    _dc_inval();
    _tlb_inval();

    DEBUG("MEM: unprotecting memory\n");

    mem_protect(0, NULL, NULL);

    DEBUG("MEM: configuring heap\n");

    extern char* fake_heap_start;
    extern char* fake_heap_end;
    extern char heap_start __asm__("__heap_start__");
    extern char heap_end __asm__("__heap_end__");

    fake_heap_start = &heap_start;
    fake_heap_end = &heap_end;

    DEBUG("MEM: mapping sections\n");

    __page_table = (u32 *)memalign(16384, 16384);
    memset32(__page_table, 0, sizeof(__page_table));

    map_section(0x080, 0x080, 0x003, WRITEBACK_CACHE | DOMAIN(0) | AP_RWUSER); // MEM0
    map_section(0x000, 0x000, 0x020, WRITEBACK_CACHE | DOMAIN(0) | AP_RWUSER); // MEM1
    map_section(0x100, 0x100, 0xC00, WRITEBACK_CACHE | DOMAIN(0) | AP_RWUSER); // MEM2
    map_section(0xFFF, 0xFFF, 0x001, WRITEBACK_CACHE | DOMAIN(0) | AP_RWUSER); // SRAM

    map_section(0x0D0, 0x0D0, 0x010, NONBUFFERABLE | DOMAIN(0) | AP_RWUSER); // MMIO

    set_dacr(0xFFFFFFFF); //manager access for all domains, ignore AP
    set_ttbr((u32)__page_table); //configure translation table

    _drain_write_buffer();

    cr = get_cr();

#ifndef NO_CACHES
    DEBUG("MEM: enabling caches\n");

    cr |= CR_DCACHE | CR_ICACHE;
    set_cr(cr);

    DEBUG("MEM: enabling MMU\n");

    cr |= CR_MMU;
    set_cr(cr);
#endif

    DEBUG("MEM: init done\n");

    irq_restore(cookie);
}

void mem_shutdown(void)
{
    u32 cookie = irq_kill();
    _dc_flush();
    _drain_write_buffer();
    u32 cr = get_cr();
    cr &= ~(CR_MMU | CR_DCACHE | CR_ICACHE); //disable ICACHE, DCACHE, MMU
    set_cr(cr);
    _ic_inval();
    _dc_inval();
    _tlb_inval();
    irq_restore(cookie);
}
#endif
