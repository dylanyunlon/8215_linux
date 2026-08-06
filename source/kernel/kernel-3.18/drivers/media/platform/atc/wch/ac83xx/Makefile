#
#
#
ccflags-y += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
ccflags-y += -I$(srctree)/drivers/misc/atc/inc
ccflags-y += -I$(srctree)/include/media/


obj-$(CONFIG_WCH_ATC) += wch.o 
wch-objs := wch_mod.o hal/wch_hal.o hal/wch_if.o
