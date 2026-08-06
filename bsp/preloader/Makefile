export TOPDIR    = $(CURDIR)
export SRCDIR    = $(TOPDIR)/src
export OBJDIR    = $(TOPDIR)/obj
export TARGETDIR = $(TOPDIR)/target
export INFODIR   = $(TOPDIR)/info
export PREBUILDDIR  = $(prebuilddir)


#Compiler
CROSS_COMPILE   := $(TOPDIR)/i686/bin
export CC        = $(CROSS_COMPILE)/armv7a-mediatek451_001_vfp-linux-gnueabi-gcc
export OBJCPY		 =  $(CROSS_COMPILE)/armv7a-mediatek451_001_vfp-linux-gnueabi-objcopy
#Linker
export LINK      =  $(CROSS_COMPILE)/armv7a-mediatek451_001_vfp-linux-gnueabi-ld
export OBJDUMP      =  $(CROSS_COMPILE)/armv7a-mediatek451_001_vfp-linux-gnueabi-objdump
export STRIP      =  $(CROSS_COMPILE)/armv7a-mediatek451_001_vfp-linux-gnueabi-strip


export NM	= $(CROSS_COMPILE)/armv7a-mediatek451_001_vfp-linux-gnueabi-nm

export LIBGCC   =$(shell $(CC) --print-libgcc-file-name)
export LIBC     =$(shell $(CC) --print-file-name=libc.a)
export LIBM     =$(shell $(CC) --print-file-name=libm.a)
#Directory

#Compiler options


#export GOPTION   = -Os -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -nostdlib -mfpu=vfp -mfloat-abi=softfp -g -c -Werror $(CHIPDEFINE) $(TARGETDEFINE) $(SPECIALCONFIG)
export GOPTION   = -Os -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -nostdlib -g -c -Werror $(CHIPDEFINE) $(TARGETDEFINE) $(SPECIALCONFIG)

export GINCLUDE	 = -I$(SRCDIR)/include

#output target
target ?= realchip
#bootdevice ?= sd
include $(TOPDIR)/../../../config/rsv.mk

ifeq ($(target),sim)
	OUTPUT  := Preloader_sim
	TARGETDEFINE := -Dconfig_TARGET_SIM -Dconfig_DVT
endif

ifeq ($(target),fpga)

ifneq ($(bootdevice),)
	OUTPUT  := Preloader_fpga_$(bootdevice)
	TARGETDEFINE := -Dconfig_TARGET_FPGA 

ifeq ($(bootdevice),sd)
  TARGETDEFINE += -DBOOTDEVICE_SD
else ifeq ($(bootdevice),nand)
  TARGETDEFINE += -DBOOTDEVICE_NAND
else ifeq ($(bootdevice),emmc)
  TARGETDEFINE += -DBOOTDEVICE_EMMC
else ifeq ($(bootdevice),serialflash)
  TARGETDEFINE += -DBOOTDEVICE_SERIALFLASH
endif

else
	OUTPUT  := Preloader_fpga
	TARGETDEFINE := -Dconfig_TARGET_FPGA -Dconfig_DVT
endif

endif

ifeq ($(target),realchip)
ifneq ($(bootdevice),)
	OUTPUT  := Preloader_realchip_$(bootdevice)
	TARGETDEFINE := -Dconfig_TARGET_REALCHIP

ifeq ($(bootdevice),sd)
  TARGETDEFINE += -DBOOTDEVICE_SD
else ifeq ($(bootdevice),nand)
  TARGETDEFINE += -DBOOTDEVICE_NAND
else ifeq ($(bootdevice),emmc)
  TARGETDEFINE += -DBOOTDEVICE_EMMC
else ifeq ($(bootdevice),nor)
  TARGETDEFINE += -DBOOTDEVICE_NOR
endif

ifeq ($(project),)
else
  TARGETDEFINE += -D__$(project)__
endif

ifeq ($(os), linux)
        TARGETDEFINE += -D__LINUX__
endif

ifeq ($(ATC_AB_PARTITION_SUPPORT),true)
  TARGETDEFINE += -DATC_AB_PARTITION_SUPPORT
endif

ifeq ($(os),ce)
  TARGETDEFINE += -DConfig_WinCE
endif

else
	OUTPUT  := Preloader_realchip
	TARGETDEFINE := -Dconfig_TARGET_REALCHIP -Dconfig_DVT
endif
endif

ifeq ($(CFG_ARGS_RESERVED_ADDR),)
$(error ***CFG_ARGS_RESERVED_ADDR is NULL,please check***)
else
TARGETDEFINE += -DCONFIG_ARGS_START=$(CFG_ARGS_RESERVED_ADDR)
TARGETDEFINE += -DCONFIG_ARGS_SIZE=$(CFG_ARGS_RESERVED_SIZE)
endif

ifeq ($(CFG_DATAZONE_RESERVED_ADDR),)
$(error ***CONFIG_DATAZONE_RESERVED is NULL,please check***)
else
TARGETDEFINE += -DCONFIG_DATAZONE_START=$(CFG_DATAZONE_RESERVED_ADDR)
TARGETDEFINE += -DCONFIG_DATAZONE_SIZE=$(CFG_DATAZONE_RESERVED_SIZE)
endif

ifeq  ($(ATC_DEBUG_TYPE), user)
TARGETDEFINE +=-DCONFIG_ATC_USER
else
TARGETDEFINE +=-DCONFIG_ATC_USERDEBUG
endif

TARGETOUTPUT = $(chip)_$(OUTPUT)
export TARGETDEFINE
export target
export os

#chip version


chip	?= 3363
testloop ?= 10

CHIPDEFINE := -DCONFIG_CHIP_VER_CURR=$(chip)


export CHIPDEFINE
export testloop
export chip

ifeq ($(chip),3363)
	SPECIALCONFIG = -march=armv7-a
endif

SUBDIRS   = $(TOPDIR)/src


all: COMPILER
	@echo "Link..." 
	$(LINK) -nostdlib  --no-check-sections -o $(TARGETDIR)/$(TARGETOUTPUT) -T $(TOPDIR)/preloader_24k.lds -Map $(INFODIR)/map.txt --stats $(wildcard $(OBJDIR)/*.o) $(OBJDIR)/libhardware/libhardware.a
#	$(LINK) --gc-sections --start-group $(wildcard $(OBJDIR)/*/*.o) --end-group -T$(TOPDIR)/preloader_24k.lds -o $(INFODIR)/$(bootdevice).elf
	${OBJCPY} -O binary -R .comment -R .note -S -g $(TARGETDIR)/$(TARGETOUTPUT) $(TARGETDIR)/$(TARGETOUTPUT).bin
	cp  $(TARGETDIR)/$(TARGETOUTPUT)  $(INFODIR)/$(bootdevice).elf
	${OBJDUMP} -dxS  $(TARGETDIR)/$(TARGETOUTPUT) > $(INFODIR)/$(bootdevice).lst
	$(NM) $(TARGETDIR)/$(TARGETOUTPUT)| grep -v '\(compiled\)\|\(\.oi$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | LC_ALL=C sort >  $(TARGETDIR)/System.map
	$(TOPDIR)/tools/mkheader.py -map $(TARGETDIR)/System.map -o $(TARGETDIR)/$(TARGETOUTPUT)-header.bin  $(TARGETDIR)/$(TARGETOUTPUT).bin

GEN_AUTO_VERSION:
#	$(TOPDIR)/tools/gen_autover.sh

COMPILER: GEN_AUTO_VERSION
	@echo Compiler...
	mkdir -p $(OBJDIR)
	mkdir -p $(TARGETDIR)
	mkdir -p $(INFODIR)
	$(MAKE) -C $(SUBDIRS)	

clean:
	-rm -rf $(OBJDIR)
	-rm -rf $(TARGETDIR)
	-rm -rf $(INFODIR)	
