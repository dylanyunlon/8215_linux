################################################################################
#
# lk
#
################################################################################

LK_VERSION =
LK_SITE = $(TOPDIR)/../vendor/autochips/proprietary/bootable/lk
LK_SITE_METHOD = local
#LK_LICENSE = GPL-2.0
#LK_LICENSE_FILES = COPYING
#LK_DEPENDENCIES =

LK_PROJECT = ac8x_car
LK_CROSS_COMPILE = $(TOPDIR)/../prebuilt/toolchain/aarch64/bin/aarch64-cros-linux-gnu-
#LK_CROSS_COMPILE = ~/atc/android/8015-6-12/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-
LK_INSTALL_IMAGES = YES

# Project configs
#ATC_AB_UPGRADE = no
#ATC_MACRO_VTS = no
#ATC_METAZONE_SUPPORT = no
#ATC_XEN_SUPPORT = no
#ATC_DTBO_FEATURE = no
#BOOTIMG_ENABLE = no
#CONFIG_INTSD_EXT4 = no
#HAVE_AEE_FEATURE = no
#SECURE_BOOT_ENABLE = no


#$(error  target cross is $(TARGET_CROSS))

# LK Framework parameters
LK_MAKE_OPTS = \
	ARCH_arm64_TOOLCHAIN_PREFIX="$(TARGET_CROSS)"	\
	TOOLCHAIN_PREFIX="$(TARGET_CROSS)"	\
	PROJECT=$(LK_PROJECT)	\
	HOST_OS=linux \
	BUILDROOT=$(@D) \
	ROOTDIR=$(LK_SITE) \
	LKROOT=$(LK_SITE)

# ATC Parameters
LK_MAKE_OPTS += \
	LCM_WITDH=1280 \
	LCM_HEIGHT=720 \
	ATC_MACRO_VTS=$(ATC_MACRO_VTS) \
	ATC_DTBO_FEATURE=$(ATC_DTBO_FEATURE) \
	BOOTIMG_ENABLE=$(BOOTIMG_ENABLE) \
	CONFIG_INTSD_EXT4=$(CONFIG_INTSD_EXT4) \
	AEE_FEATURE=$(HAVE_AEE_FEATURE) \
	SECURE_BOOT_ENABLE=$(SECURE_BOOT_ENABLE) \
	BOARD_AVB_ENABLE=$(BOARD_AVB_ENABLE) \
	SECURE_STORAGE_RPMB_SUPPORT=$(SECURE_STORAGE_RPMB_SUPPORT) \
	RPMB_CHIP_ID_KEY=$(RPMB_CHIP_ID_KEY) \
	ATC_AOSP_ENHANCEMENT=$(ATC_AOSP_ENHANCEMENT) \
	DEBUG_FPGA=$(DEBUG_FPGA) \
	ATC_LOGO_SUPPORT=$(ATC_LOGO_SUPPORT) \
	ATC_METAZONE_SUPPORT=$(ATC_METAZONE_SUPPORT) \
	ATC_EMMC_HW_WP_SUPPORT=$(ATC_EMMC_HW_WP_SUPPORT) \
	ATC_AB_UPGRADE=$(ATC_AB_UPGRADE) \
	TARGET_BUILD_VARIANT=$(TARGET_BUILD_VARIANT) \
	SEC_BOOT_SLOT_SWITCH=$(SEC_BOOT_SLOT_SWITCH) \
	SRC_TOOLS_DIR_PATH=$(TOPDIR)/../vendor/autochips/proprietary/tools

ifeq ($(ATC_METAZONE_SUPPORT), yes)
METAZONE_PATH := $(LK_SITE)/../../external/metazone
endif

ifeq ($(SECURE_BOOT_ENABLE), yes)
LK_TARGET := lk_verified.img
else
LK_TARGET := lk.bin
endif

ifeq ($(ATC_XEN_SUPPORT), yes)
	LK_MAKE_OPTS += XEN_ENABLE=1
endif

define LK_BUILD_CMDS
	$(MAKE) -C $(@D) $(LK_MAKE_OPTS)
endef

define LK_INSTALL_IMAGES_CMDS
	cp $(@D)/build-$(LK_PROJECT)/$(LK_TARGET) $(BINARIES_DIR)/lk.bin
endef

$(eval $(generic-package))
