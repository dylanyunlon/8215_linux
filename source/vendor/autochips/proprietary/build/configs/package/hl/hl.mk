################################################################################
#
# hsm_loader
#
################################################################################

HL_VERSION =
HL_CODE_PATH := $(TOPDIR)/../vendor/autochips/proprietary/tinysys/hsm/loader
HL_PREBUILT_PATH := $(TOPDIR)/../prebuilt/images
HL_SITE=$(shell if [ -d $(HL_CODE_PATH) ]; then echo $(HL_CODE_PATH); else echo $(HL_PREBUILT_PATH); fi)
HL_SITE_METHOD = local
#HL_LICENSE = GPL-2.0
#HL_LICENSE_FILES = COPYING
#HL_DEPENDENCIES =

HL_PROJECT = ac8x_car
HL_INSTALL_IMAGES = YES

TINYSYS_CROSS_COMPILE = $(TOPDIR)/../vendor/autochips/proprietary/tinysys/rtos/toolchain/7.2.1/bin/arm-none-eabi-
HL_PREBUILD_IMG := $(TOPDIR)/../vendor/autochips/proprietary/tools/Bins/hl.bin
HL_SIGN_KEY := $(TOPDIR)/../vendor/autochips/proprietary/tools/sign-image/cust_key/atc_rsa_prvk
HL_HEADER_TOOL := $(TOPDIR)/../vendor/autochips/proprietary/tools/sign-image/generate_preloader_image_tool.py

# for hl framework config
HL_MAKE_OPTS = \
	target=realchip \
	bootdevice=mmc \
	TINYSYS_CROSS_COMPILE=$(TINYSYS_CROSS_COMPILE) \
	HL_TOP_PATH=$(HL_SITE) \
	TARGET_BUILD_VARIANT=$(TARGET_BUILD_VARIANT) \
	LOAD_VISS_TO_DRAM=$(LOAD_VISS_TO_DRAM) \
	ATC_AB_UPGRADE=$(ATC_AB_UPGRADE) \
	ATC_SFDIS_SUPPORT=yes \
	target_project=ac8015_android \
	out=$(@D)/HL_OBJ \

# for atc config
HL_MAKE_OPTS += \
	SECURE_BOOT_ENABLE=$(SECURE_BOOT_ENABLE) \
	SECURE_STORAGE_RPMB_SUPPORT=$(SECURE_STORAGE_RPMB_SUPPORT) \
	BOARD_AVB_ENABLE=$(BOARD_AVB_ENABLE) \
	SEC_BOOT_SLOT_SWITCH=$(SEC_BOOT_SLOT_SWITCH)

define HL_BUILD_CMDS
	@if [ -d $(HL_CODE_PATH) ]; then \
		$(MAKE) -C $(@D) $(HL_MAKE_OPTS) normal rsa; else \
		echo "HL Prebuild"; \
	fi
endef

define HL_INSTALL_IMAGES_CMDS
	@if [ -e $(@D)/HL_OBJ/hl.bin ]; then \
		cp -f $(@D)/HL_OBJ/hl.bin $(BINARIES_DIR); \
		cp $(@D)/HL_OBJ/hl_verified.img $(BINARIES_DIR); \
	else \
		cp -f $(@D)/hl.bin $(BINARIES_DIR); \
		cp $(@D)/hl_verified.img $(BINARIES_DIR); \
	fi
endef

$(eval $(generic-package))
