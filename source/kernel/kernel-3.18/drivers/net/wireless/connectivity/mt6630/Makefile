# SPDX-License-Identifier: GPL-2.0

#export CONFIG_ATC_OS_LINUX := y     # for ac8x linux
export CONFIG_MTK_COMBO := m
export CONFIG_MTK_COMBO_CHIP := MT6630

#####################################################
# CONFIG_ARCH_ACXXX
# CONFIG_ATC_OS_XXX=1
# CONFIG_ATC_OS_VERSION_XXX=1
#
ifdef CONFIG_ARCH_AC83XX

    # AC8317_LINUX
    ifeq ($(CONFIG_ATC_OS_VER),"dizzy")
        export CONFIG_ATC_OS="linux"
        export CONFIG_ATC_OS_VERSION="dizzy"
        subdir-ccflags-y += -DCONFIG_ATC_OS_LINUX=1
        subdir-ccflags-y += -DCONFIG_ATC_OS_VERSION_DIZZY=1

    # AC8317_M
    else ifeq ($(CONFIG_ATC_OS_VER),"m")
        export CONFIG_ATC_OS="android"
        export CONFIG_ATC_OS_VERSION="m"
        subdir-ccflags-y += -DCONFIG_ATC_OS_ANDROID=1
        subdir-ccflags-y += -DCONFIG_ATC_OS_VERSION_M=1

    # AC8317_JB2
    else
        export CONFIG_ATC_OS="android"
        export CONFIG_ATC_OS_VERSION="jb2"
        subdir-ccflags-y += -DCONFIG_ATC_OS_ANDROID=1
        subdir-ccflags-y += -DCONFIG_ATC_OS_VERSION_JB2=1
    endif

else ifdef CONFIG_ARCH_AC823X

    # below has defined at defconfig and autoconf.h

    # AC8237_O
    #
    # export CONFIG_ATC_OS="android"
    # export CONFIG_ATC_OS_VERSION="O"
    #
    # CONFIG_ARCH_AC823X=1
    # CONFIG_ATC_OS_ANDROID=1
    # CONFIG_ATC_OS_VERSION_O=1

    # AC8237_LINUX
    #
    # export CONFIG_ATC_OS="linux"
    # export CONFIG_ATC_OS_VERSION="2.5"
    #
    # CONFIG_ARCH_AC823X=1
    # CONFIG_ATC_OS_LINUX=1
    # CONFIG_ATC_OS_VERSION_YOCTO25=1

else ifdef CONFIG_ARCH_AC8X

    # below has defined at defconfig and autoconf.h

    # export CONFIG_ATC_OS="android"
    # export CONFIG_ATC_OS_VERSION="P"
    #
    # CONFIG_ARCH_AC8X=1
    # CONFIG_ATC_OS_ANDROID=1
    # CONFIG_ATC_OS_VERSION_P=1

else
    ifdef CONFIG_ARM64
        export CONFIG_ARCH_AC8X=y
        export CONFIG_ATC_OS="android"
        export CONFIG_ATC_OS_VERSION="P"
        subdir-ccflags-y += -DCONFIG_ARCH_AC8X=1
        subdir-ccflags-y += -DCONFIG_ATC_OS_ANDROID=1
        subdir-ccflags-y += -DCONFIG_ATC_OS_VERSION_P=1
    else
        # Caution: JB2 make -C connectivity will enter here
        $(warning FIXME: unknown ARCH)
    endif
endif

# gpio
ifdef CONFIG_ARCH_AC823X
    subdir-ccflags-y += -I$(KERNELDIR)/drivers/gpio/
endif

# === coredump config start

ifneq ($(and $(CONFIG_ARCH_AC8X),$(CONFIG_ATC_OS_LINUX)),)
    # AC8x Linux
    # CONFIG_ATC_AEE_FEATURE=y
    # but aee_kernel_warning and aed_combo_exception not ready
    # use stp_dump instead
else
    ifdef CONFIG_MTK_AEE_FEATURE
        export CONFIG_WMT_SUPPORT_AEE=y

    else ifdef CONFIG_ATC_AEE_FEATURE
        export CONFIG_WMT_SUPPORT_AEE=y
    endif
endif

ifndef CONFIG_WMT_SUPPORT_AEE
    export CONFIG_WMT_USE_STP_DUMP=y

else ifdef CONFIG_ARCH_AC83XX
    ifeq ($(CONFIG_ATC_OS_VERSION),"m")
        # AEE mode for AC8317M
    else
        # AC8317_JB2/AC8317_LINUX
        export CONFIG_WMT_USE_STP_DUMP=y
    endif
else
    # if andoird AEE not ready, we use stp_dump here.
    # export CONFIG_WMT_USE_STP_DUMP=y
endif

ifdef CONFIG_WMT_SUPPORT_AEE
    subdir-ccflags-y += -DCONFIG_WMT_SUPPORT_AEE=1
endif

ifdef CONFIG_WMT_USE_STP_DUMP
    subdir-ccflags-y += -DCONFIG_WMT_USE_STP_DUMP=1
endif

# === coredump config end

ifeq ($(CONFIG_SDIO_CLK_SWITCH),y)
    subdir-ccflags-y += -DSDIO_CLK_SWITCH
endif

ifeq ($(CONFIG_MTK_LTE_SUPPORT),y)
    subdir-ccflags-y += -D WMT_IDC_SUPPORT=1
else
    subdir-ccflags-y += -D WMT_IDC_SUPPORT=0
endif

subdir-ccflags-y += -D MTK_WCN_WMT_STP_EXP_SYMBOL_ABSTRACT
#subdir-ccflags-y += -D MTK_WCN_REMOVE_KERNEL_MODULE

ifeq ($(CONFIG_ARM64), y)
    subdir-ccflags-y += -D CONFIG_MTK_WCN_ARM64
endif

subdir-ccflags-y += -D MT6630
subdir-ccflags-y += -D MTK_MERGE_INTERFACE_SUPPORT

obj-y += common_detect/
obj-y += combo/

ifneq ($(and $(CONFIG_ARCH_AC8X),$(CONFIG_ATC_OS_LINUX)),)
    obj-$(CONFIG_ATC_GPS_6630)      += gps_6630/
endif

