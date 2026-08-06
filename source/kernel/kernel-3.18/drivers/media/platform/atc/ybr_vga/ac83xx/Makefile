#
# Makefile for atc 83xx/823x ypbpr vga controller
# 
#

ccflags-y += -Idrivers/misc/atc/inc
ccflags-y += -Idrivers/misc/atc/private_inc
ccflags-y += -Idrivers/misc/atc/include/mach
ccflags-y += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
ccflags-y += -I$(srctree)/drivers/media/platform/atc/ybr_vga/ac83xx/hal
ccflags-y += -I$(srctree)/arch/arm/mach-ac83xx/include/mach/
ccflags-y += -I$(srctree)/include/media/atc/

obj-$(CONFIG_ATC_MEDIA_SUPPORT_YBRVGA) += ybr.o

ybr-objs := hal/autocolor_table.o hal/drv_adcset.o hal/drv_async.o hal/drv_auto.o \
            hal/drv_autocolor.o hal/drv_hdtv.o hal/drv_vdoclk.o hal/drv_vga.o \
	        hal/vga_auto.o hal/vga_hal_api.o hal/vga_hal_io.o hal/vga_table.o \
            hal/ybr_vga_common.o ybr_vga_drv_if.o ybr_vga_oal.o ybr_vga_mod.o