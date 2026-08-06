#
# Makefile for the 8237 metazone driver (for Linux Kernel 3.18)
# 
#

PWD := $(shell pwd)
EXTRA_CFLAGS += -I$(srctree)/include/misc/atc/metazone/
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/inc/
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
ifeq ($(CONFIG_MTD_NAND_ATC),y)
EXTRA_CFLAGS += -DNAND_BOOT
endif

ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))

ifeq "$(ATC_OS_NAME)" "android"
EXTRA_CFLAGS += -DCONFIG_MTZ_ANDROID
endif

obj-y += metazone_drv.o




