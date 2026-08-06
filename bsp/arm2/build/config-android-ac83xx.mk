AUTO_ADD_GLOBAL_DEFINE_BY_NAME := CONFIG_PLATFORM_AC83XX
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
HIDE            := @

GLOBAL_INC += -I$(ARM2TOPDIR)/inc/ -I$(ARM2OBJDIR)/inc/ -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/mach/
GLOBAL_INC += -I$(ATC_KERNEL_DIR)/drivers/misc/atc/inc/
GLOBAL_INC += -I$(UBOOT_DIR)/platform/ac83xx/include/platform

NOECHO ?=
####################################################################################



####################################################################################
#
#         Project Spec define macro
#
####################################################################################
CONFIG_PROJECT  := __ANDROID__
CONFIG_PLATFORM := AC83XX
CONFIG_PLATFORM_AC83XX = yes

#Init tvd in lk for 8317M
INIT_TVD_BEFORE_ARM2_START_ENABLED := true


####################################################################################
