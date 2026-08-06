#
# Makefile for the framebuffer driver (for Linux Kernel 3.18)
# 
#

ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))
ifeq "$(ATC_OS_NAME)" "android"
EXTRA_CFLAGS += -DVCP_FOR_ANDROID
endif

EXTRA_CFLAGS += -I$(srctree)/arch/arm/mach-ac83xx/include/mach/
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/inc
EXTRA_CFLAGS += -I$(srctree)/include/misc/atc/metazone
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/private_inc
EXTRA_CFLAGS += -I$(srctree)/$(src)/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/staging/android
EXTRA_CFLAGS += -I$(srctree)/include/media/atc
EXTRA_CFLAGS += -I$(srctree)/$(src)/osd/hal
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/dualarm/ac83xx
EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/atc/ac83xx/inc

osd_files = $(patsubst $(srctree)/$(src)/osd/hal/%.c,osd/hal/%.o,$(wildcard $(srctree)/$(src)/osd/hal/*.c))  osd/osd_misc.o
pmx_files = $(patsubst $(srctree)/$(src)/pmx/%.c,pmx/%.o,$(wildcard $(srctree)/$(src)/pmx/*.c))
vdp_files = $(patsubst $(srctree)/$(src)/vdp/%.c,vdp/%.o,$(wildcard $(srctree)/$(src)/vdp/*.c))
scl_files = $(patsubst $(srctree)/$(src)/scl/%.c,scl/%.o,$(wildcard $(srctree)/$(src)/scl/*.c))
tcon_files = $(patsubst $(srctree)/$(src)/tcon/%.c,tcon/%.o,$(wildcard $(srctree)/$(src)/tcon/*.c))
#dal_files = $(patsubst $(srctree)/$(src)/dal/%.c,dal/%.o,$(wildcard $(srctree)/$(src)/dal/*.c))

atcfb-objs :=fb_dev.o displayinit.o  tve/tve_hal.o tve/tve_if.o $(osd_files) $(pmx_files) $(vdp_files) $(scl_files) $(tcon_files)

#$(warning $(atcfb-objs))
obj-$(CONFIG_FB0_ATC) += atcfb.o