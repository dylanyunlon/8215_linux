ccflags-y += -I$(srctree)/drivers/video/fbdev/atc/pp/$(CONFIG_ATC_PLATFORM)/include/
ccflags-y += -I$(srctree)/include/media/atc/ac823x/
ccflags-y += -I$(srctree)/drivers/misc/atc/inc/

obj-$(CONFIG_PP_ATC) += pp.o 
pp-objs := pp_drv.o hal/pp_hal.o hal/pp_if.o
