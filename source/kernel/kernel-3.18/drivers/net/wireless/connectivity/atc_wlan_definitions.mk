# SPDX-License-Identifier: GPL-2.0-only

ifndef __ATC_WLAN_DEFINITIONS
__ATC_WLAN_DEFINITIONS := y

# ifeq ($(call kernel-version-lt, 4, 9, 0), 1)
# endif
define kernel-version-lt
$(shell \
    if [ $(VERSION)      -gt $(1) ]; then echo 0; \
    elif [ $(VERSION)    -lt $(1) ]; then echo 1; \
    elif [ $(PATCHLEVEL) -gt $(2) ]; then echo 0; \
    elif [ $(PATCHLEVEL) -lt $(2) ]; then echo 1; \
    elif [ $(SUBLEVEL)   -gt $(3) ]; then echo 0; \
    elif [ $(SUBLEVEL)   -lt $(3) ]; then echo 1; \
    else echo 0; \
    fi)
endef

define kernel-version-le
$(shell \
    if [ $(VERSION)      -gt $(1) ]; then echo 0; \
    elif [ $(VERSION)    -lt $(1) ]; then echo 1; \
    elif [ $(PATCHLEVEL) -gt $(2) ]; then echo 0; \
    elif [ $(PATCHLEVEL) -lt $(2) ]; then echo 1; \
    elif [ $(SUBLEVEL)   -gt $(3) ]; then echo 0; \
    elif [ $(SUBLEVEL)   -lt $(3) ]; then echo 1; \
    else echo 1; \
    fi)
endef

define kernel-version-eq
$(shell \
    if [ $(VERSION)      -ne $(1) ]; then echo 0; \
    elif [ $(PATCHLEVEL) -ne $(2) ]; then echo 0; \
    elif [ $(SUBLEVEL)   -ne $(3) ]; then echo 0; \
    else echo 1; \
    fi)
endef

define kernel-version-ne
$(shell \
    if [ $(VERSION)      -ne $(1) ]; then echo 1; \
    elif [ $(PATCHLEVEL) -ne $(2) ]; then echo 1; \
    elif [ $(SUBLEVEL)   -ne $(3) ]; then echo 1; \
    else echo 0; \
    fi)
endef

define kernel-version-gt
$(shell \
    if [ $(VERSION)      -gt $(1) ]; then echo 1; \
    elif [ $(VERSION)    -lt $(1) ]; then echo 0; \
    elif [ $(PATCHLEVEL) -gt $(2) ]; then echo 1; \
    elif [ $(PATCHLEVEL) -lt $(2) ]; then echo 0; \
    elif [ $(SUBLEVEL)   -gt $(3) ]; then echo 1; \
    elif [ $(SUBLEVEL)   -lt $(3) ]; then echo 0; \
    else echo 0; \
    fi)
endef

define kernel-version-ge
$(shell \
    if [ $(VERSION)      -gt $(1) ]; then echo 1; \
    elif [ $(VERSION)    -lt $(1) ]; then echo 0; \
    elif [ $(PATCHLEVEL) -gt $(2) ]; then echo 1; \
    elif [ $(PATCHLEVEL) -lt $(2) ]; then echo 0; \
    elif [ $(SUBLEVEL)   -gt $(3) ]; then echo 1; \
    elif [ $(SUBLEVEL)   -lt $(3) ]; then echo 0; \
    else echo 1; \
    fi)
endef

# LOCAL_PATH := $(call my-dir)
define my-dir
$(strip \
    $(eval LOCAL_MODULE_MAKEFILE := $$(lastword $$(MAKEFILE_LIST))) \
    $(patsubst %/,%,$(dir $(LOCAL_MODULE_MAKEFILE))) \
)
endef

ifeq ($(call kernel-version-ge, 5, 10, 0), 1)
    subdir-ccflags-y += -I$(srctree)/drivers/soc/autochips/include
else
    subdir-ccflags-y += -I$(call my-dir)/include
endif

ifndef CONFIG_ATC_WIFI_CHIP
    export CONFIG_ATC_WIFI_CHIP := "$(ATC_WIFI_CHIP)"
    subdir-ccflags-y += -DCONFIG_ATC_WIFI_CHIP="\"$(ATC_WIFI_CHIP)\""
    export CONFIG_ATC_WLAN_TRANSMISSION_MODE := "$(ATC_WLAN_TRANSMISSION_MODE)"
    subdir-ccflags-y += -DCONFIG_ATC_WLAN_TRANSMISSION_MODE="\"$(ATC_WLAN_TRANSMISSION_MODE)\""
    ifeq ($(ATC_WLAN_TRANSMISSION_MODE), SDIO_MODE)
        export CONFIG_ATC_WLAN_TRANSMISSION_MODE_SDIO := y
        subdir-ccflags-y += -DCONFIG_ATC_WLAN_TRANSMISSION_MODE_SDIO
        ifeq ($(ATC_WIFI_CHIP), ATC_CHIP_MT6630)
        export CONFIG_ATC_WIFI_CHIP_MT6630_SDIO := y
        subdir-ccflags-y += -DCONFIG_ATC_WIFI_CHIP_MT6630_SDIO
        else ifeq ($(ATC_WIFI_CHIP), CYPRESS)
        export CONFIG_ATC_WIFI_CHIP_CYPRESS_SDIO := y
        subdir-ccflags-y += -DCONFIG_ATC_WIFI_CHIP_CYPRESS_SDIO
        else ifeq ($(ATC_WIFI_CHIP), AIC8800)
        export CONFIG_ATC_WIFI_CHIP_AIC8800_SDIO := y
        subdir-ccflags-y += -DCONFIG_ATC_WIFI_CHIP_AIC8800_SDIO
        else ifeq ($(ATC_WIFI_CHIP), SELF_ADAPTIVE)
        export CONFIG_ATC_WIFI_CHIP_SELF_ADAPTIVE := y
        subdir-ccflags-y += -DCONFIG_ATC_WIFI_CHIP_SELF_ADAPTIVE
        endif
    else ifeq ($(ATC_WLAN_TRANSMISSION_MODE), PCIE_MODE)
        export CONFIG_ATC_WLAN_TRANSMISSION_MODE_PCIE := y
        subdir-ccflags-y += -DCONFIG_ATC_WLAN_TRANSMISSION_MODE_PCIE
        ifeq ($(ATC_WIFI_CHIP), CYPRESS)
        export CONFIG_ATC_WIFI_CHIP_CYPRESS_PCIE := y
        subdir-ccflags-y += -DCONFIG_ATC_WIFI_CHIP_CYPRESS_PCIE
        endif
    endif
endif

endif # __ATC_WLAN_DEFINITIONS
