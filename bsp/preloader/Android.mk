# Copyright (C) 2014 AutoChips Inc
# Modification based on code covered by the mentioned copyright
# and/or permission notice(s).
# Copyright 2006 The Android Open Source Project

ifeq "$(TARGET_DEVICE)" "ac83xx_evb"
PRELOADER_DIR := $(PWD)
PRELOADER_ROOT_DIR  := $(call my-dir)
PRELOADER_OUT := $(PRELOADER_ROOT_DIR)/target
BUILD_PRELOADER_TARGET := $(PRELOADER_OUT)/3363_Preloader_realchip_sd.bin
INSTALLED_PRELOADER_TARGET := $(PRODUCT_OUT)/3363_Preloader_realchip_sd.bin
INSTALLED_PRELOADER_PREBUILT := $(ANDROID_BUILD_TOP)/vendor/atc/libs/ac83xx_evb/preloader/3363_Preloader_realchip_sd.bin

.PHONY:preloader

droid:preloader
droidcore:preloader


preloader: $(INSTALLED_PRELOADER_TARGET)
	echo "---Preloader compile done"

$(INSTALLED_PRELOADER_TARGET):$(BUILD_PRELOADER_TARGET)
	echo "Install Preloader Target!!!--@="$@
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -f $(BUILD_PRELOADER_TARGET) $@
	$(hide) cp -f $(BUILD_PRELOADER_TARGET) $(INSTALLED_PRELOADER_PREBUILT)

$(BUILD_PRELOADER_TARGET):FORCE
	echo "Build Preloader Target!!!"
	$(MAKE) -C $(PRELOADER_ROOT_DIR) target=realchip bootdevice=sd project=AndroidM
preloader-clean:
	@echo "Clean Preloader"
	$(MAKE) -C $(PRELOADER_ROOT_DIR) clean
endif

