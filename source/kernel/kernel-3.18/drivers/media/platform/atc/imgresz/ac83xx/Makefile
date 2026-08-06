#
# Makefile for HW ImgResizer
# 
#

EXTRA_CFLAGS += -I$(srctree)/arch/arm/mach-ac83xx/include/mach
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/private_inc

imgresz_obj = \
imgresz_drv/imgresz_hal_if.o \
imgresz_drv/imgresz_hal.o \
imgresz_drv/imgresz_drv_if.o \
imgresz_drv/imgresz_drv_inst.o \
imgresz_drv/imgresz_drv.o \
imgresz_drv/imgresz_util.o \
imgresz_mod.o

obj-$(CONFIG_ATC_MEDIA_SUPPORT_IMGRESZ) := imgresz.o
imgresz-objs := $(imgresz_obj)
