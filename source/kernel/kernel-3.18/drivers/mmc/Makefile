#
# Makefile for the kernel mmc device drivers.
#
ifdef CONFIG_ATC_AOSP_ENHANCEMENT
include $(srctree)/drivers/net/wireless/connectivity/atc_wlan_definitions.mk
endif

subdir-ccflags-$(CONFIG_MMC_DEBUG) := -DDEBUG
subdir-ccflags-y += -I$(srctree)/drivers/misc/atc/inc

obj-$(CONFIG_MMC)		+= core/
obj-$(CONFIG_MMC)		+= card/
obj-$(subst m,y,$(CONFIG_MMC))	+= host/
