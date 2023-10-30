#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/base_rules

GCC_VERSION := $(shell $(CC) -dumpversion)

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
# SPECS is the directory containing the important build and link files
#---------------------------------------------------------------------------------
export TARGET	:=	minute_minute
export BUILD	?=	build

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export ROOTDIR	:=	$(CURDIR)
export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)


.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@$(MAKE) -C $(ROOTDIR)/$(TARGET) -f Makefile.isfshax clean
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
BOOT1 = $(ROOTDIR)/$(TARGET)/isfshax_stage2-strip.elf

$(ROOTDIR)/superblock.img: $(OUTPUT).bin
	@echo $(notdir $@)
	@python3 $(ROOTDIR)/isfshax.py $< $@

$(OUTPUT).bin: $(BOOT1) $(ELFLOADER)
	@echo $(notdir $@)
	@cat $(ELFLOADER) $< > $@

$(ELFLOADER):
	@$(MAKE) -C $(ROOTDIR)/stage2ldr

$(BOOT1):
	@$(MAKE) -C $(ROOTDIR)/$(TARGET) -f Makefile.isfshax


-include $(DEPENDS)


#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
