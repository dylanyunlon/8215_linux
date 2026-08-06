#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

#########################################################################

ifneq ($(OBJTREE),$(SRCTREE))
ifeq ($(CURDIR),$(SRCTREE))
dir :=
else
dir := $(subst $(SRCTREE)/,,$(CURDIR))
endif

obj := $(if $(dir),$(OBJTREE)/$(dir)/,$(OBJTREE)/)
src := $(if $(dir),$(SRCTREE)/$(dir)/,$(SRCTREE)/)

$(shell mkdir -p $(obj))
else
obj :=
src :=
endif

# clean the slate ...
PLATFORM_RELFLAGS =
PLATFORM_CPPFLAGS =
PLATFORM_LDFLAGS =

#########################################################################

ifeq ($(HOSTOS),darwin)
HOSTCC		= cc
else
HOSTCC		= gcc
endif
HOSTCFLAGS	= -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer
HOSTSTRIP	= strip

#########################################################################
#
# Option checker (courtesy linux kernel) to ensure
# only supported compiler options are used
#
cc-option = $(shell if $(CC) $(CFLAGS) $(1) -S -o /dev/null -xc /dev/null \
		> /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

#
# Include the make variables (CC, etc...)
#
AS	= $(CROSS_COMPILE)as
LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
CPP	= $(CC) -E
AR	= $(CROSS_COMPILE)ar
NM	= $(CROSS_COMPILE)nm
LDR	= $(CROSS_COMPILE)ldr
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB	= $(CROSS_COMPILE)RANLIB

#########################################################################

# Load generated board configuration
sinclude $(OBJTREE)/include/autoconf.mk

ifdef	ARCH
sinclude $(TOPDIR)/lib_$(ARCH)/config.mk	# include architecture dependend rules
endif
ifdef	CPU
sinclude $(TOPDIR)/cpu/$(CPU)/config.mk		# include  CPU	specific rules
endif
ifdef	SOC
sinclude $(TOPDIR)/cpu/$(CPU)/$(SOC)/config.mk	# include  SoC	specific rules
endif
ifdef	VENDOR
BOARDDIR = $(VENDOR)/$(BOARD)
else
BOARDDIR = $(BOARD)
endif
ifdef	BOARD
sinclude $(TOPDIR)/board/$(BOARDDIR)/config.mk	# include board specific rules
endif

#########################################################################

ifneq (,$(findstring s,$(MAKEFLAGS)))
ARFLAGS = cr
else
ARFLAGS = crv
endif
RELFLAGS= $(PLATFORM_RELFLAGS)
DBGFLAGS= -g # -DDEBUG
OPTFLAGS= -Os #-fomit-frame-pointer
ifndef LDSCRIPT
#LDSCRIPT := $(TOPDIR)/board/$(BOARDDIR)/u-boot.lds.debug
ifeq ($(CONFIG_NAND_U_BOOT),y)
LDSCRIPT := $(TOPDIR)/board/$(BOARDDIR)/u-boot-nand.lds
else
LDSCRIPT := $(TOPDIR)/board/$(BOARDDIR)/u-boot.lds
endif
endif
OBJCFLAGS += --gap-fill=0xff

gccincdir := $(shell $(CC) -print-file-name=include)

CPPFLAGS := $(DBGFLAGS) $(OPTFLAGS) $(RELFLAGS)		\
	-D__KERNEL__
ifneq ($(TEXT_BASE),)
CPPFLAGS += -DTEXT_BASE=$(TEXT_BASE)
endif

ifneq ($(OBJTREE),$(SRCTREE))
CPPFLAGS += -I$(OBJTREE)/include2 -I$(OBJTREE)/include
endif

CPPFLAGS += -I$(TOPDIR)/include
CPPFLAGS += -I$(TOPDIR)/rsv
CPPFLAGS += -I$(KERNEL_SRC)/include/uapi/
CPPFLAGS += -I$(KERNEL_SRC)/include/
CPPFLAGS += -I$(KERNEL_SRC)/arch/arm/include/
CPPFLAGS += -I$(KERNEL_SRC)/arch/arm/mach-ac83xx/include/
CPPFLAGS += -I$(KERNEL_SRC)/arch/arm/mach-ac83xx/include/mach/

CPPFLAGS += -I$(PWD)/../../../device/atc/ac8317/common/
CPPFLAGS += -I$(BSP_TOPDIR)/arm2/inc/
CPPFLAGS += -I$(TOPDIR)/include/asm-arm/arch-ac83xx/
CPPFLAGS += -I$(KERNEL_SRC)/drivers/misc/atc/inc/
CPPFLAGS += -I$(KERNEL_SRC)/include/misc/atc/metazone/

CPPFLAGS += -fno-builtin -ffreestanding -nostdinc	\
	-isystem $(gccincdir) -pipe $(PLATFORM_CPPFLAGS)

CPPFLAGS += -D__UBOOT__

ifdef BUILD_TAG
CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes \
	-DBUILD_TAG='"$(BUILD_TAG)"'
else
CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes
endif
CFLAGS += $(call cc-option,-fno-stack-protector)

# avoid trigraph warnings while parsing pci.h (produced by NIOS gcc-2.9)
# this option have to be placed behind -Wall -- that's why it is here
ifeq ($(ARCH),nios)
ifeq ($(findstring 2.9,$(shell $(CC) --version)),2.9)
CFLAGS := $(CPPFLAGS) -Wall -Wno-trigraphs
endif
endif

# $(CPPFLAGS) sets -g, which causes gcc to pass a suitable -g<format>
# option to the assembler.
AFLAGS_DEBUG :=

# turn jbsr into jsr for m68k
ifeq ($(ARCH),m68k)
ifeq ($(findstring 3.4,$(shell $(CC) --version)),3.4)
AFLAGS_DEBUG := -Wa,-gstabs,-S
endif
endif

AFLAGS := $(AFLAGS_DEBUG) -D__ASSEMBLY__ $(CPPFLAGS)

LDFLAGS += -Bstatic -T $(obj)u-boot.lds $(PLATFORM_LDFLAGS)
ifneq ($(TEXT_BASE),)
LDFLAGS += -Ttext $(TEXT_BASE)
endif

# Location of a usable BFD library, where we define "usable" as
# "built for ${HOST}, supports ${TARGET}".  Sensible values are
# - When cross-compiling: the root of the cross-environment
# - Linux/ppc (native): /usr
# - NetBSD/ppc (native): you lose ... (must extract these from the
#   binutils build directory, plus the native and U-Boot include
#   files don't like each other)
#
# So far, this is used only by tools/gdb/Makefile.

ifeq ($(HOSTOS),darwin)
BFD_ROOT_DIR =		/usr/local/tools
else
ifeq ($(HOSTARCH),$(ARCH))
# native
BFD_ROOT_DIR =		/usr
else
#BFD_ROOT_DIR =		/LinuxPPC/CDK		# Linux/i386
#BFD_ROOT_DIR =		/usr/pkg/cross		# NetBSD/i386
BFD_ROOT_DIR =		/opt/powerpc
endif
endif

ifneq ($(ANDROID_SRC),)
include $(ANDROID_SRC)/autochips/device/atc/ac8317/BoardConfig.mk
endif

include $(TOPDIR)/../../../../../config/rsv.mk

ifeq ($(CFG_ARGS_RESERVED_ADDR),)
$(error ***CFG_ARGS_RESERVED_ADDR is NULL,please check***)
else
CFLAGS += -DCONFIG_ARGS_START=$(CFG_ARGS_RESERVED_ADDR)
CFLAGS += -DCONFIG_ARGS_SIZE=$(CFG_ARGS_RESERVED_SIZE)
endif

ifeq ($(CFG_DATAZONE_RESERVED_ADDR),)
$(error ***CONFIG_DATAZONE_RESERVED is NULL,please check***)
else
CFLAGS += -DCONFIG_DATAZONE_START=$(CFG_DATAZONE_RESERVED_ADDR)
CFLAGS += -DCONFIG_DATAZONE_SIZE=$(CFG_DATAZONE_RESERVED_SIZE)
endif

ifeq  ($(ATC_DEBUG_TYPE), user)
CFLAGS +=-DCONFIG_ATC_USER
else
CFLAGS +=-DCONFIG_ATC_USERDEBUG
endif
###############################################################################################
# According to boot device, define some macro for read/write images from special device
ifeq  ($(AC83XX_BOOT_DEVICE),emmc)
   CFLAGS +=-DCONFIG_BOOT_MMC
   CFLAGS +=-DCONFIG_USRDATA_EXT4
   CFLAGS +=-DBOOT_FROM_EMMC
# when boot from emmc, defined CONFIG_SECURITY_UPGRADE,
# when boot from nand, undefined CONFIG_SECURITY_UPGRADE.
   CFLAGS +=-DCONFIG_SECURITY_UPGRADE
# $(warning ------ emmc bootup build ------)
endif

ifeq  ($(AC83XX_BOOT_DEVICE),sd2)
   CFLAGS +=-DCONFIG_BOOT_MMC
   CFLAGS +=-DCONFIG_USRDATA_EXT4
   CFLAGS +=-DBOOT_FROM_SD2
# $(warning ------ sd2 bootup build ------)
endif
################################################################################################

# when boot from nand, the follow define to
# enable/disable readback check for nand upgrade
# add by qiyun
   #CFLAGS +=-DCONFIG_NAND_UPG_RDBACK_CHK

ifeq ($(CMD_SDAGENT),true)
   CFLAGS +=-DCONFIG_CMD_SDAGENT
endif

ifeq ($(CMD_MSDC_ETT),true)
   CFLAGS +=-DCONFIG_MSDC_ETT
endif

ifeq  ($(AC83XX_BOOT_DEVICE),nand)
   CFLAGS +=-DLOAD_ATC_NAND_DRV
   CFLAGS +=-DCONFIG_SECURITY_UPGRADE
endif

ifeq ($(ATC_AB_PARTITION_SUPPORT),true)
    CFLAGS +=-DATC_AB_PARTITION_SUPPORT
endif

CFLAGS+=-D__UBOOT__

#########################################################################

export	HOSTCC HOSTCFLAGS CROSS_COMPILE \
	AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP MAKE
export	TEXT_BASE PLATFORM_CPPFLAGS PLATFORM_RELFLAGS CPPFLAGS CFLAGS AFLAGS

#########################################################################

# Allow boards to use custom optimize flags on a per dir/file basis
BCURDIR := $(notdir $(CURDIR))
$(obj)%.s:	%.S
	$(CPP) $(AFLAGS) $(AFLAGS_$(@F)) $(AFLAGS_$(BCURDIR)) -o $@ $<
$(obj)%.o:	%.S
	$(CC)  $(AFLAGS) $(AFLAGS_$(@F)) $(AFLAGS_$(BCURDIR)) -o $@ $< -c
$(obj)%.o:	%.c
	$(CC)  $(CFLAGS) $(CFLAGS_$(@F)) $(CFLAGS_$(BCURDIR)) -o $@ $< -c
$(obj)%.i:	%.c
	$(CPP) $(CFLAGS) $(CFLAGS_$(@F)) $(CFLAGS_$(BCURDIR)) -o $@ $< -c
$(obj)%.s:	%.c
	$(CC)  $(CFLAGS) $(CFLAGS_$(@F)) $(CFLAGS_$(BCURDIR)) -o $@ $< -c -S

#########################################################################

# import rsv address from rsv.mk
GLOBAL_CFLAG += \
	-DCFG_ARM2_RESERVED_ADDR=$(CFG_ARM2_RESERVED_ADDR)                       -DCFG_ARM2_RESERVED_SIZE=$(CFG_ARM2_RESERVED_SIZE) \
	-DCFG_ARGS_RESERVED_ADDR=$(CFG_ARGS_RESERVED_ADDR)                       -DCFG_ARGS_RESERVED_SIZE=$(CFG_ARGS_RESERVED_SIZE) \
	-DCFG_METAZONE_RESERVED_ADDR=$(CFG_METAZONE_RESERVED_ADDR)               -DCFG_METAZONE_RESERVED_SIZE=$(CFG_METAZONE_RESERVED_SIZE) \
	-DCFG_FRAMEBUFFER_RESERVED_ADDR=$(CFG_FRAMEBUFFER_RESERVED_ADDR)         -DCFG_FRAMEBUFFER_RESERVED_SIZE=$(CFG_FRAMEBUFFER_RESERVED_SIZE) \
	-DCFG_WCH_RESERVED_ADDR=$(CFG_WCH_RESERVED_ADDR)                         -DCFG_WCH_RESERVED_SIZE=$(CFG_WCH_RESERVED_SIZE) \
	-DCFG_ARM2_BACKCAR_UI_RESERVED_ADDR=$(CFG_ARM2_BACKCAR_UI_RESERVED_ADDR) -DCFG_ARM2_BACKCAR_UI_RESERVED_SIZE=$(CFG_ARM2_BACKCAR_UI_RESERVED_SIZE) \
	-DCFG_IMAGERESIZE_RESERVED_ADDR=$(CFG_IMAGERESIZE_RESERVED_ADDR)         -DCFG_IMAGERESIZE_RESERVED_SIZE=$(CFG_IMAGERESIZE_RESERVED_SIZE) \
	-DCFG_MM_RESERVED_ADDR=$(CFG_MM_RESERVED_ADDR)                           -DCFG_MM_RESERVED_SIZE=$(CFG_MM_RESERVED_SIZE) \
	-DCFG_VBA_RESERVED_ADDR=$(CFG_VBA_RESERVED_ADDR)                         -DCFG_VBA_RESERVED_SIZE=$(CFG_VBA_RESERVED_SIZE) \
	-DCFG_AUDIO_R1_ADDR=$(CFG_AUDIO_R1_ADDR)                                 -DCFG_AUDIO_R1_SIZE=$(CFG_AUDIO_R1_SIZE) \
	-DCFG_ANIMATION_RESERVED_ADDR=$(CFG_ANIMATION_RESERVED_ADDR)             -DCFG_ANIMATION_RESERVED_SIZE=$(CFG_ANIMATION_RESERVED_SIZE) \
	-DCFG_DATAZONE_RESERVED_ADDR=$(CFG_DATAZONE_RESERVED_ADDR)               -DCFG_DATAZONE_RESERVED_SIZE=$(CFG_DATAZONE_RESERVED_SIZE) \

CFLAGS += $(GLOBAL_CFLAG)
