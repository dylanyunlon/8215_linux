AUTO_ADD_GLOBAL_DEFINE_BY_NAME := CONFIG_PLATFORM_AC83XX CONFIG_VIDEOBOOTANI_EN
AUTO_ADD_GLOBAL_DEFINE_BY_VALUE :=
AUTO_ADD_GLOBAL_DEFINE_BY_NAME_VALUE := CONFIG_PLATFORM CONFIG_PROJECT CONFIG_VIDEOBOOTANI

####################################################################################
#
#         Global Var list define
#
####################################################################################
CROSS_COMPILE   := $(ARM2TOPDIR)/toolchain/bin/arm-none-eabi-
CC              := $(CROSS_COMPILE)gcc --sysroot=$(DA_SYSROOT)
OBJCPY	        := $(CROSS_COMPILE)objcopy
LINK            := $(CROSS_COMPILE)ld
OBJDUMP	        := $(CROSS_COMPILE)objdump
AR	        	:= $(CROSS_COMPILE)ar
NM	        	:= $(CROSS_COMPILE)nm
ARM2OUTDIR      := $(ARM2OUTDIR)

PY              := python3
COPY            := cp
CAT             := cat

UBOOT_DIR       := $(BSP_TOPDIR)/bootloader/uboot/uboot-83xx
ATC_KERNEL_DIR  := $(BSP_TOPDIR)/../source/kernel/kernel-3.18

GLOBAL_INC += -I$(ARM2TOPDIR)/inc/ -I$(ARM2OBJDIR)/inc/ -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/mach/ -I$(ARM2TOPDIR)/freertos/include/
GLOBAL_INC += -I$(ATC_KERNEL_DIR)/drivers/misc/atc/inc/
GLOBAL_INC += -I$(UBOOT_DIR)/include/asm-arm/arch-ac83xx/
GLOBAL_INC += -I$(ARM2TOPDIR)/rsv
GLOBAL_INC += -I$(ARM2TOPDIR)/lib/libfdt/include

NOECHO ?=
####################################################################################



####################################################################################
#
#         Project Spec define macro
#
####################################################################################
CONFIG_PROJECT  := __LINUX__
CONFIG_PLATFORM := AC83XX
CONFIG_PLATFORM_AC83XX = yes

CONFIG_VIDEOBOOTANI := EN
CONFIG_VIDEOBOOTANI_EN = yes




####################################################################################
#
# Linux arm2 feature control
#
####################################################################################

#linux arm2 backcar track feature.
TRACK_FEATURE_ENABLED := false

#depends on 'TRACK_FEATURE_ENABLED := false'
TRACK_BITS := 16

#Init tvd in uboot.
INIT_TVD_BEFORE_ARM2_START_ENABLED := false

#Enable/Disable backcar in the system upgrade mode
BACKCAR_IN_UPGRADE_MODE_ENABLED := false
