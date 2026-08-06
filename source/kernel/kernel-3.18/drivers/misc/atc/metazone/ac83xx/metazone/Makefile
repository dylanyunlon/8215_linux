#
# Makefile for the metazone driver (for Linux Kernel 3.18)
# 
#

PWD := $(shell pwd)
EXTRA_CFLAGS := -O0 -g
ccflags-y += -marm
ccflags-y += -march=armv5te
EXTRA_CFLAGS += -I$(srctree)/$(src)/inc
EXTRA_CFLAGS += -I$(srctree)/arch/arm/mach-ac83xx/include/mach/
EXTRA_CFLAGS += -I$(KERNEL_BUILDDIR)/include/generated/
EXTRA_CFLAGS += -I$(srctree)/include/linux/
EXTRA_CFLAGS += -I$(srctree)/include/misc/atc/metazone/
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/inc/
EXTRA_CFLAGS += -I$(srctree)/$(src)/../../../private_inc
EXTRA_CFLAGS += -I$(DRIVERS_INC)/
EXTRA_CFLAGS += -DCHIP_VER_AC83XX  -DMODULE

metazone_objs = metazone_inter.o

lib-y := \
$(metazone_objs)



