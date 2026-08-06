PLATFORM_NAME := $(subst ",,$(CONFIG_ATC_PLATFORM))
ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))

ifeq "$(PLATFORM_NAME)" "ac83xx"
#ifeq "$(ATC_OS_NAME)" "android"
    EXTRA_CFLAGS += -DCONFIG_AC83XX_WDT_ENABLE
    EXTRA_CFLAGS += -DCONFIG_AC83XX_HD_CUT_ENABLE
#    EXTRA_CFLAGS += -DCONFIG_AC83XX_PWR_CUT_INITIATIVE
#    EXTRA_CFLAGS += -DCONFIG_AC83XX_SW_PWR_CUT_INITIATIVE
#    EXTRA_CFLAGS += -DCONFIG_AC83XX_RANDOM_PWR_CUT_INITIATIVE
#endif
endif

ccflags-y += -I$(srctree)/drivers/misc/atc/power-loss-test/ac83xx/
ccflags-y += -I$(srctree)/kernel/
ccflags-y += -I$(srctree)/drivers/misc/atc/aee/aed/
ccflags-y += -I$(srctree)/include/linux/

obj-$(CONFIG_PWR_LOSS_ATC_TEST) += power_loss_test.o power_loss_emmc_test.o
