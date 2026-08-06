
ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))
ifeq "$(ATC_OS_NAME)" "android"
EXTRA_CFLAGS += -DAEE_FOR_ANDROID
endif

EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/atc/dal


obj-$(CONFIG_MTK_AEE_AED) += aed.o
aed-y := aed-main.o aed-debug.o aed-process.o
aed-$(CONFIG_MTK_AEE_DRAM_CONSOLE) += dram_console.o
aed-y += monitor_hang.o
