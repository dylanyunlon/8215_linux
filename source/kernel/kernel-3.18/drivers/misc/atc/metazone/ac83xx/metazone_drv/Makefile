#
# Makefile for the metazone driver (for Linux Kernel 3.18)
# 
#

PWD := $(shell pwd)
EXTRA_CFLAGS := -O0 -g
EXTRA_CFLAGS += -I$(srctree)/$(src)/inc
EXTRA_CFLAGS += -I$(objtree)/include/generated/
EXTRA_CFLAGS += -I$(srctree)/include/misc/atc/metazone/
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/inc/
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atcprivate_inc
EXTRA_CFLAGS += -I$(DRIVERS_INC)/
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
EXTRA_CFLAGS += -DCHIP_VER_AC83XX  -DMODULE
ifeq ($(CONFIG_MTD_NAND_ATC),y)
EXTRA_CFLAGS += -DNAND_BOOT
endif

ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))

ifeq "$(ATC_OS_NAME)" "android"
EXTRA_CFLAGS += -DCONFIG_MTZ_ANDROID
endif

obj-m := mtz_drv.o
mtz_drv-objs := \
	metazone_drv.o \
	metazone_dbg.o \
	../metazone/metazone_inter.o \

#drivers/misc/atc/metazone/metazone_drv/../metazone/lib.a:
#	$(MAKE) -C $(srctree) M=$(srctree)/drivers/misc/atc/metazone/metazone O=$(KERNEL_BUILDDIR)
#	@ mkdir -p $(UBOOT_SRC)/mtzlib; cp -f $(src)/../metazone/lib.a $(UBOOT_SRC)/mtzlib/atcmtz.a


