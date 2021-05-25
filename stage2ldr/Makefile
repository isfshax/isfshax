C_FILES  := $(foreach dir, ., $(wildcard $(dir)/*.c))
S_FILES  := $(foreach dir, ., $(wildcard $(dir)/*.S))

OBJS  := $(C_FILES:.c=.o)  $(S_FILES:.S=.o)

PREFIX  = $(DEVKITARM)/bin/arm-none-eabi
CC      = $(PREFIX)-gcc
LD      = $(PREFIX)-ld
STRIP   = $(PREFIX)-strip
OBJCOPY = $(PREFIX)-objcopy

ASFLAGS   = -marm -fomit-frame-pointer -mbig-endian -fshort-wchar -mcpu=arm926ej-s -march=armv5te -fno-zero-initialized-in-bss
CFLAGS   = -Os -Wall -fpic $(ASFLAGS)
LDFLAGS = -n -nostartfiles -Wl,-EB -L"$(DEVKITARM)/arm-none-eabi/lib"

all: stage2ldr.bin

stage2ldr.bin: stage2ldr.elf
	@echo $(notdir $@)
	@$(OBJCOPY) -S -O binary $^ $@

stage2ldr.elf: $(OBJS)
	@echo $(notdir $@)
	@$(CC) $(LDFLAGS) -T link.ld $^ -o $@

%.o: %.c
	@echo $(notdir $<)
	@$(CC) $(CFLAGS) -c $^ -o $@

%.o: %.S
	@echo $(notdir $<)
	@$(CC) $(ASFLAGS) -c $^ -o $@

clean:
	@rm -rf $(OBJS) stage2ldr.elf stage2ldr.bin
	@echo "Cleaned!"
