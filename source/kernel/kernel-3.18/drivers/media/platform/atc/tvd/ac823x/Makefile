#
# Makefile for the tvd driver (for Linux Kernel 3.18)
# 
#


#ccflags-y += -I$(srctree)/arch/arm/mach-ac83xx/include/mach
ccflags-y += -I$(srctree)/drivers/misc/atc/inc
ccflags-y += -I$(srctree)/drivers/misc/atc/include/
ccflags-y += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
ccflags-y += -I$(srctree)/drivers/media/platform/atc/tvd/ac823x/inc
ccflags-y += -I$(srctree)/include/media/atc/ac823x_tvd

obj-$(CONFIG_VIDEO_CVBS) += tvd.o 
tvd-objs := tvd_mod.o tvd_drv/arm2_comm_if.o tvd_drv/tvd_core.o tvd_drv/tvd_drv_if.o tvd_drv/tvd_hal_if.o tvd_drv/tdc.o

