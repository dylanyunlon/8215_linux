# Use bash, not whatever shell somebody has installed as /bin/sh
# This is repeated in config.mk, since envsetup.sh runs that file
# directly.
SHELL := /bin/bash

# this turns off the suffix rules built into make
.SUFFIXES:

# this turns off the RCS / SCCS implicit rules of GNU Make
% : RCS/%,v
% : RCS/%
% : %,v
% : s.%
% : SCCS/s.%

# If a rule fails, delete $@.
.DELETE_ON_ERROR:

# Absolute path of the present working direcotry.
# This overrides the shell variable $PWD, which does not necessarily points to
# the top of the source tree, for example when "make -C" is used in m/mm/mmm.
PWD := $(shell pwd)

TOP := .
TOPDIR := $(PWD)/
DEVICE ?= ac83xx
VARIANT ?= userdebug
export TOPDIR

BUILDROOT_DIR := $(TOPDIR)/buildroot
BUILDROOT_EXTERNAL=$(TOPDIR)/source/vendor/autochips/proprietary/build/configs
OUT_DIR := $(TOPDIR)/out

ifeq ("$(platform)","D1")
DEVICE := ac8015-xen_D1
endif

# Fix build error
unexport LD_LIBRARY_PATH
$(shell mkdir -p $(OUT_DIR)/build/$(DEVICE))

define CONFIG_ENABLE_OPT # (option, file)
    sed -i "/\\<$(1)\\>/d" $(2)
    echo '$(1)=y' >> $(2)
endef

define CONFIG_SET_OPT # (option, value, file)
    sed -i "/\\<$(1)\\>/d" $(3)
    echo '$(1)=$(2)' >> $(3)
endef

define CONFIG_DISABLE_OPT # (option, file)
    sed -i "/\\<$(1)\\>/d" $(2)
    echo '# $(1) is not set' >> $(2)
endef

define CONFIG_FIXUP_CMDS # (file)
    $(foreach opt, $(DEFCONFIG_ENABLE),
        $(call CONFIG_ENABLE_OPT,$(opt),$(1))
    )
    $(foreach opt, $(DEFCONFIG_DISABLE),
        $(call CONFIG_DISABLE_OPT,$(opt),$(1))
    )
endef

MERGED_DEFCONFIG = $(OUT_DIR)/$(DEVICE)_defconfig
DEFCONFIG_FRAGMENT =
DEFCONFIG_ENABLE =
DEFCONFIG_DISABLE =



QT_ENABLED :=

ifeq ($(UI_FRAMEWORK),qt)
    ifneq ($(ATC_DDR_SIZE),128)
        ifneq ($(AC83XX_BOOT_DEVICE_SIZE),128)
            ifeq ($(AC83XX_BOOT_DEVICE_SIZE),256)
                ifeq ($(ATC_AB_PARTITION_SUPPORT),false)
                    QT_ENABLED := y
                else
                    $(error "UI_FRAMEWORK is qt, ATC_AB_PARTITION_SUPPORT:$(ATC_AB_PARTITION_SUPPORT), AC83XX_BOOT_DEVICE_SIZE: $(AC83XX_BOOT_DEVICE_SIZE), Does not meet the conditions to launch QT")
                endif
            else
                QT_ENABLED := y
            endif
        endif
    endif

    ifeq ($(QT_ENABLED),y)
        $(warning "UI_FRAMEWORK is qt, include qt_fragment")
        DEFCONFIG_FRAGMENT += $(BUILDROOT_EXTERNAL)/configs/qt_fragment
        DEFCONFIG_FRAGMENT += $(BUILDROOT_EXTERNAL)/configs/usbkeyboard_hotplugin_fragment

        ifneq ($(wildcard $(TOPDIR)/source/kernel/kernel-3.18/drivers/usb/gadget/function/carlink),)
            DEFCONFIG_FRAGMENT += $(BUILDROOT_EXTERNAL)/configs/add_carplayapp_fragment
        endif

        ifneq ($(ATC_BT_CHIP),)
            DEFCONFIG_ENABLE += BR2_PACKAGE_BTAPP
            DEFCONFIG_ENABLE += BR2_PACKAGE_BTPHONE
            DEFCONFIG_ENABLE += BR2_PACKAGE_BTPTS_TOOL
            DEFCONFIG_ENABLE += BR2_PACKAGE_BLUECOMMON
            DEFCONFIG_ENABLE += BR2_PACKAGE_BLUECLIENT
            DEFCONFIG_ENABLE += BR2_PACKAGE_BLUESERVER
            DEFCONFIG_ENABLE += BR2_PACKAGE_BLUESTACK
        endif
    else
        $(error "UI_FRAMEWORK is qt, but QT is disabled. DDR_SIZE:$(ATC_DDR_SIZE) BOOT_DEVICE_SIZE:$(AC83XX_BOOT_DEVICE_SIZE) AB_PARTITION:$(ATC_AB_PARTITION_SUPPORT)")
    endif
else ifeq ($(UI_FRAMEWORK),awtk)
    $(warning "UI_FRAMEWORK is awtk, include awtk_fragment")
    DEFCONFIG_FRAGMENT += $(BUILDROOT_EXTERNAL)/configs/awtk_fragment
else ifeq ($(UI_FRAMEWORK),lvgl)
    $(warning "UI_FRAMEWORK is lvgl, include lvgl_fragment")
else
    $(warning "UI_FRAMEWORK is $(UI_FRAMEWORK)")
endif
# wifi
ifneq ($(ATC_WIFI_CHIP),)
DEFCONFIG_ENABLE += BR2_PACKAGE_WIFI
endif

ifneq ($(ATC_WIFI_CHIP),)
    ifneq ($(ATC_BT_CHIP),)
        ifneq ($(wildcard $(TOPDIR)/source/kernel/kernel-3.18/drivers/usb/gadget/function/carlink),)
            DEFCONFIG_FRAGMENT += $(BUILDROOT_EXTERNAL)/configs/add_carplay_fragment
        endif
    endif
endif

$(MERGED_DEFCONFIG): $(BUILDROOT_EXTERNAL)/configs/$(DEVICE)_defconfig $(DEFCONFIG_FRAGMENT)
	@mkdir -p $(dir $@)
	cp $< $@
	echo >> $@
	for f in $(DEFCONFIG_FRAGMENT); do \
		cat $$f >> $@; \
		echo >> $@; \
	done
	sed -i '/^$$/d' $@
	$(call CONFIG_FIXUP_CMDS,$@)

MAKE_OPTS = O=$(OUT_DIR)
MAKE_OPTS += BR2_EXTERNAL=$(BUILDROOT_EXTERNAL)
MAKE_OPTS += TARGET_BUILD_VARIANT=$(VARIANT)
MAKE_OPTS += BR2_DEFCONFIG=$(MERGED_DEFCONFIG)

# This is the default target.  It must be the first declared target.
PHONY := _all amenuconfig

_all: $(OUT_DIR)/$(DEVICE)_defconfig
	make -C $(BUILDROOT_DIR) $(MAKE_OPTS) defconfig && \
	make -C $(BUILDROOT_DIR) $(MAKE_OPTS)

amenuconfig:
	make -C $(BUILDROOT_DIR) $(MAKE_OPTS) defconfig && \
	make -C $(BUILDROOT_DIR) $(MAKE_OPTS) menuconfig && \
	make -C $(BUILDROOT_DIR) $(MAKE_OPTS) savedefconfig

# Add target to Makefile
PHONY += $(MAKECMDGOALS) sub-make

$(filter-out _all amenuconfig sub-make $(CURDIR)/Makefile, $(MAKECMDGOALS)) : sub-make
	@:

sub-make:
	make -C $(BUILDROOT_DIR) $(MAKE_OPTS) $(filter-out _all amenuconfig sub-make,$(MAKECMDGOALS))

PHONY += FORCE

FORCE:

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)
