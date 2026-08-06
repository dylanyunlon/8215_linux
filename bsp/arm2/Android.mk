# Copyright (C) 2014 AutoChips Inc
# Modification based on code covered by the mentioned copyright
# and/or permission notice(s).
# Copyright 2006 The Android Open Source Project
ifeq "$(TARGET_PRODUCT)" "full_ac83xx_evb"
$(warning "arm2 Compile for ac83xx")
ARM2_PROJECT := AC83xx_EVB
LK_RESERVE_PATH := ac83xx
TARGET_PROJECT := android-ac83xx
else ifeq "$(TARGET_PRODUCT)" "full_ac823x_evb"
$(warning "arm2 Compile for ac823x")
LK_RESERVE_PATH := ac823x
ARM2_PROJECT := AC823x_EVB
TARGET_PROJECT := android-ac823x
else ifeq "$(TARGET_PRODUCT)" "full_ac823x_adas"
$(warning "arm2 Compile for ac823x adas")
LK_RESERVE_PATH := ac823x
ARM2_PROJECT := AC823x_ADAS
TARGET_PROJECT := android-ac823x-adas
endif

ARM2_ROOT_DIR := $(PWD)
ARM2_DIR := $(call my-dir)
ARM2_OUT := $(ANDROID_BUILD_TOP)/$(PRODUCT_OUT)/obj/ARM2_DIR
BUILD_ARM2_TARGET := $(ARM2_OUT)/arm2.bin
INSTALLED_ARM2_TARGET := $(PRODUCT_OUT)/arm2.bin
ATC_KERNEL_DIR    := $(ANDROID_BUILD_TOP)/kernel/kernel-3.18
export KERNEL_OBJTREE := $(ANDROID_BUILD_TOP)/$(PRODUCT_OUT)/obj/KERNEL_OBJ
UBOOT_SRC    := $(ANDROID_BUILD_TOP)/vendor/atc/proprietary/bootable/lk/platform/$(LK_RESERVE_PATH)/include/platform/
need_kernel := $(PRODUCT_OUT)/kernel
.PHONY:arm2 arm2-clean
droid:arm2
droidcore:arm2

arm2: $(INSTALLED_ARM2_TARGET)
	echo "---Invoke arm2 Compile!!!"

$(INSTALLED_ARM2_TARGET):$(BUILD_ARM2_TARGET)
	echo "Install Arm2 Target!!!--@="$@
	$(hide) mkdir -p $(dir $@)
	$(hide) cp -f $(ARM2_OUT)/arm2.bin $(INSTALLED_ARM2_TARGET)

$(BUILD_ARM2_TARGET): 
	echo "Build ARM2 Target!!!"
	$(MAKE) -C $(ARM2_DIR) target_project=$(TARGET_PROJECT) O=$(ARM2_OUT)
#	CROSS_COMPILE=arm-linux-androideabi- && $(MAKE) -C $(ARM2_DIR) UBOOT_SRC=$(UBOOT_SRC) ATC_KERNEL_DIR=$(ATC_KERNEL_DIR) ARM2OUTDIR=$(ARM2_OUT) KERNEL_OBJTREE=$(KERNEL_OBJTREE)

arm2-clean:
	@echo "Clean arm2"
	-rm -rf $(INSTALLED_ARM2_TARGET)
	$(MAKE) -C $(ARM2_DIR) clean
