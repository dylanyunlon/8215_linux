################################################################################
#
# hsm_os
#
################################################################################

HSM_VERSION =
HSM_CODE_PATH := $(TOPDIR)/../vendor/autochips/proprietary/tinysys/hsm/os
HSM_PREBUILT_PATH := $(TOPDIR)/../prebuilt/images
HSM_SITE=$(shell if [ -d $(HSM_CODE_PATH) ]; then echo $(HSM_CODE_PATH); else echo $(HSM_PREBUILT_PATH); fi)
HSM_SITE_METHOD = local
#HSM_LICENSE = GPL-2.0
#HSM_LICENSE_FILES = COPYING
#HSM_DEPENDENCIES =

HSM_PROJECT = ac8x_car
HSM_INSTALL_IMAGES = YES

TINYSYS_CROSS_COMPILE = $(TOPDIR)/../vendor/autochips/proprietary/tinysys/rtos/toolchain/7.2.1/bin/arm-none-eabi-
HSM_PREBUILD_IMG := $(TOPDIR)/../vendor/autochips/proprietary/tools/Bins/hsm.bin
_IMAGE_EXTRACT_TOOL := $(TOPDIR)/../vendor/autochips/proprietary/tools/sign-image/extract_image
_IMAGE_SIGN_TOOL := $(TOPDIR)/../vendor/autochips/proprietary/tools/sign-image/make_image.sh
_CUST_KEY := $(TOPDIR)/../vendor/autochips/proprietary/tools/sign-image/cust_key/atc_rsa_prvk

# for hsm framework config
HSM_MAKE_OPTS = \
	target=realchip \
	bootdevice=mmc \
	TINYSYS_CROSS_COMPILE=$(TINYSYS_CROSS_COMPILE) \
	HSM_TOP_PATH=$(HSM_SITE) \
	TARGET_BUILD_VARIANT=$(TARGET_BUILD_VARIANT) \
	target_project=ac8015_android \
	out=$(@D)/HSM_OBJ

ifeq ($(SECURE_BOOT_ENABLE), yes)
HSM_TARGET := hsm_verified.img
else
HSM_TARGET := hsm.bin
endif

define HSM_BUILD_CMDS
	@if [ -d $(HSM_CODE_PATH) ]; then \
		$(MAKE) -C $(@D) $(HSM_MAKE_OPTS); else \
		echo "HSM Prebuild"; \
	fi
endef

define HSM_INSTALL_IMAGES_CMDS
	@if [ -e $(@D)/HSM_OBJ/$(HSM_TARGET) ]; then \
		cp -f $(@D)/HSM_OBJ/$(HSM_TARGET) $(BINARIES_DIR)/hsm.bin; \
	else \
		cp $(@D)/hsm.bin $(BINARIES_DIR)/hsm.bin; \
	fi
endef

$(eval $(generic-package))
