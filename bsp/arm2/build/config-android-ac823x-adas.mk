AUTO_ADD_GLOBAL_DEFINE_BY_NAME := CONFIG_PLATFORM_AC823X 
AUTO_ADD_GLOBAL_DEFINE_BY_VALUE := 
AUTO_ADD_GLOBAL_DEFINE_BY_NAME_VALUE := CONFIG_PLATFORM CONFIG_PROJECT 

####################################################################################
#
#         Global Var list define
#
####################################################################################
CROSS_COMPILE   := arm-linux-androideabi-
CC              := $(CROSS_COMPILE)gcc --sysroot=$(DA_SYSROOT)
OBJCPY	        := $(CROSS_COMPILE)objcopy
LINK            := $(CROSS_COMPILE)ld
OBJDUMP	        := $(CROSS_COMPILE)objdump
ARM2OUTDIR      := $(ARM2OUTDIR)

UBOOT_DIR       := $(ANDROID_BUILD_TOP)/vendor/atc/proprietary/bootable/lk
ATC_KERNEL_DIR  := $(ANDROID_BUILD_TOP)/kernel/kernel-3.18

GLOBAL_INC += -I$(ARM2TOPDIR)/inc/ -I$(ARM2OBJDIR)/inc/
GLOBAL_INC += -I$(ATC_KERNEL_DIR)/drivers/misc/atc/inc/
GLOBAL_INC += -I$(UBOOT_DIR)/platform/ac823x/include/platform

NOECHO ?= 
####################################################################################



####################################################################################
#
#         Project Spec define macro
#
####################################################################################
CONFIG_PROJECT  := __ANDROID__
CONFIG_PLATFORM := AC823X
CONFIG_PLATFORM_AC823X = yes
CONFIG_ATC_PRJ_AC823X_ADAS = yes




####################################################################################
