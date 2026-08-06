#
# Makefile for the framebuffer driver (for Linux Kernel 3.18)
# 
#

EXTRA_CFLAGS += -I$(srctree)/arch/arm/mach-ac83xx/include/mach/
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/private_inc
EXTRA_CFLAGS += -I$(srctree)/$(src)/inc

obj-$(CONFIG_FB1_ATC) += fb1.o 
fb1-objs := fb1_dev.o 
