#
#
#

ccflags-y += -I$(srctree)/include/media/atc/ac823x/
ccflags-y += -I$(srctree)/drivers/media/platform/atc/wch/ac823x/include
ccflags-y += -I$(srctree)/drivers/misc/atc/inc

obj-$(CONFIG_WCH_ATC) += wch.o 
wch-objs := wch_drv.o hal/wch_hal.o hal/wch_if.o
