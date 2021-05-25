#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
# SPECS is the directory containing the important build and link files
#---------------------------------------------------------------------------------
export TARGET	:=	stage2
export BUILD	?=	build

R_SOURCES	:=
SOURCES		:=	stage2 stage2/fatfs stage2/isfs

R_INCLUDES	:=
INCLUDES 	:=	stage2

DATA		:=

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH		:=	-march=armv5te -mcpu=arm926ej-s -marm -mthumb-interwork -mbig-endian -mfloat-abi=soft

CFLAGS		:=	-g -std=c11 -Wall -Wno-unused-function -O3 \
			-fomit-frame-pointer -ffunction-sections \
			$(ARCH)

CFLAGS		+=	$(INCLUDE) -DCAN_HAZ_IRQ -D_GNU_SOURCE -fno-builtin-printf -Wno-nonnull

CXXFLAGS	:=	$(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS		:=	-g $(ARCH)
LDFLAGS		 =	-n -nostartfiles -g --specs=../stage2/link.specs $(ARCH) -Wl,--gc-sections,-Map,$(TARGET).map \
			-L$(DEVKITARM)/lib/gcc/arm-none-eabi/5.3.0/be -L$(DEVKITARM)/arm-none-eabi/lib/be

LIBS		:=

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS		:=

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export ROOTDIR	:=	$(CURDIR)
export OUTPUT	:=	$(CURDIR)/$(TARGET)

SOURCES		:=	$(SOURCES) $(foreach dir,$(R_SOURCES), $(dir) $(filter %/, $(wildcard $(dir)/*/)))
INCLUDES	:=	$(INCLUDES) $(foreach dir,$(R_INCLUDES), $(dir) $(filter %/, $(wildcard $(dir)/*/)))

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
			$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.S=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@$(MAKE) -C $(ROOTDIR)/stage2ldr clean
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT)-strip.elf $(OUTPUT).bin
	@rm -fr $(ROOTDIR)/superblock.img $(ROOTDIR)/superblock.img.sha


#---------------------------------------------------------------------------------
else

DEPENDS		:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
ELFLOADER = $(ROOTDIR)/stage2ldr/stage2ldr.bin

$(ROOTDIR)/superblock.img: $(OUTPUT).bin
	@echo $(notdir $@)
	@python3 $(ROOTDIR)/isfshax.py $< $@

$(OUTPUT).bin: $(TARGET)-strip.elf $(ELFLOADER)
	@echo $(notdir $@)
	@cat $(ELFLOADER) $< > $@

$(TARGET)-strip.elf: $(TARGET).elf
	@echo $(notdir $@)
	@$(STRIP) $< -o $@

$(TARGET).elf: $(OFILES)

$(ELFLOADER):
	@$(MAKE) -C $(ROOTDIR)/stage2ldr


-include $(DEPENDS)


#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
