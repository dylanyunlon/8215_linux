#
# Makefile for the framebuffer driver (for Linux Kernel 3.18)
#
#
SRC=$(srctree)/drivers/video/fbdev/atc/ac823x/
EXTRA_CFLAGS += -I$(srctree)/arch/arm/mach-ac83xx/include/
EXTRA_CFLAGS += -I$(srctree)/arch/arm/mach-ac83xx/include/mach
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/private_inc
EXTRA_CFLAGS += -I$(srctree)/drivers/media/platform/atc/mali_midgard/include
EXTRA_CFLAGS += -I$(srctree)/include/media/atc
EXTRA_CFLAGS += -I$(srctree)/include/media/atc/ac823x
EXTRA_CFLAGS += -I$(srctree)/include
EXTRA_CFLAGS += -I$(srctree)/arch/arm64/include/asm
EXTRA_CFLAGS += -I$(SRC)/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/staging/android
EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/atc/ac823x/
EXTRA_CFLAGS += -DAC823X_CONFIG

osd_files = $(patsubst $(SRC)/osd/hal/%.c,osd/hal/%.o,$(wildcard $(SRC)/osd/hal/*.c))  osd/osd_misc.o
pmx_files = $(patsubst $(SRC)/pmx/%.c,pmx/%.o,$(wildcard $(SRC)/pmx/*.c))
vdp_files = $(patsubst $(SRC)/vdp/%.c,vdp/%.o,$(wildcard $(SRC)/vdp/*.c))
scl_files = $(patsubst $(SRC)/scl/%.c,scl/%.o,$(wildcard $(SRC)/scl/*.c))
tcon_files = $(patsubst $(SRC)/tcon/%.c,tcon/%.o,$(wildcard $(SRC)/tcon/*.c))

#atcfb-objs := fb_dev.o displayinit.o tve/tve_hal.o tve/tve_if.o $(osd_files) $(pmx_files) $(vdp_files) $(scl_files) $(tcon_files)
atcfb-objs := fb_dev.o displayinit.o $(osd_files) $(pmx_files) $(vdp_files) $(scl_files) $(tcon_files)

#$(warning $(atcfb-objs))
obj-$(CONFIG_FB0_ATC) += atcfb.o
