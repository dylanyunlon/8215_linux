ccflags-y += -I$(srctree)/drivers/media/platform/atc/nr/$(CONFIG_ATC_PLATFORM)/include/
ccflags-y += -I$(srctree)/arch/arm64/include/
ccflags-y += -I$(srctree)/include/media/atc/$(CONFIG_ATC_PLATFORM)/
ccflags-y += -I$(srctree)/drivers/misc/atc/inc/

obj-$(CONFIG_PP_ATC) += nr.o 
nr-objs := nr_drv.o hal/nr_hal.o hal/nr_if.o
