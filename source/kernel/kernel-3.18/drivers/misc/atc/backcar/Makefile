#
# Makefile for AC83xx BackCar communication Driver
#

obj-m += backcar.o

PLATFORM_NAME := $(subst ",,$(CONFIG_ATC_PLATFORM))

ifeq "$(PLATFORM_NAME)" "ac83xx"
ccflags-y += -I$(srctree)/arch/arm/mach-ac83xx/include/mach/
endif

ifeq "$(PLATFORM_NAME)" "ac823x"
ccflags-y += -I$(srctree)/include/media/atc/ac823x/
endif

ccflags-y += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc/
ccflags-y += -I$(srctree)/drivers/misc/atc/inc/
ccflags-y += -I$(srctree)/include/media/atc/