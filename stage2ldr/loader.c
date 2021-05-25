/*
    mini - a Free Software replacement for the Nintendo/BroadOn IOS.

    ELF loader

Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 2.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
#include "types.h"
#include "utils.h"
#include "start.h"
#include "hollywood.h"
#include "string.h"
#include "elf.h"

extern u8 __elf_start[];

void *loadelf(const u8 *elf) {
    if(memcmp("\x7F" "ELF\x01\x02\x01",elf,7)) {
        panic(0xE3);
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr*)elf;
    if(ehdr->e_phoff == 0) {
        panic(0xE4);
    }
    int count = ehdr->e_phnum;
    Elf32_Phdr *phdr = (Elf32_Phdr*)(elf + ehdr->e_phoff);
    while(count--)
    {
        if(phdr->p_type == PT_LOAD) {
            const void *src = elf + phdr->p_offset;
            memcpy(phdr->p_paddr, src, phdr->p_filesz);
        }
        phdr++;
    }
    return ehdr->e_entry;
}

static inline void disable_boot0()
{
    set32(HW_BOOT0, 0x1000);
}

static inline void mem_setswap()
{
    set32(HW_MEMMIRR, 0x20);
}

void *_main(void *base)
{
    void *entry;

    mem_setswap(1);
    disable_boot0(1);

    entry = loadelf(__elf_start);
    return entry;
}

