#
# Makefile for ATC83xx irtdma
# 
#

PWD=$(shell pwd)
#EXTRA_CFLAGS := -O0 -g
EXTRA_CFLAGS := -I$(src)/
EXTRA_CFLAGS := -I$(src)/gfx_hal/
EXTRA_CFLAGS += -I$(DRIVERS_INC)/
EXTRA_CFLAGS += -I$(KERNELDIR)/arch/arm/mach-ac83xx/include/mach/
EXTRA_CFLAGS += -I$(KERNELDIR)/include/generated/
EXTRA_CFLAGS += -I$(src)/../libosal/$(CONFIG_ATC_PLATFORM)/inc
EXTRA_CFLAGS += -I$(src)/../private_inc
EXTRA_CFLAGS += -I$(src)/../oal
EXTRA_CFLAGS += -DCHIP_VER_AC83XX
EXTRA_CFLAGS += -I$(srctree)/arch/arm/mach-ac83xx/include/mach
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/private_inc
EXTRA_CFLAGS += -I$(srctree)/include/media/atc

include $(BOARD_CFG_FILE)
ifeq ($(GPIO124_FUN_TYPE), OUT_LOW)
ccflags-y				+= -DOUT_LOW
endif

irtdma_obj =  irt_dma_hw.o


obj-m := irtdma.o
irtdma-objs = \
	irtdma_drv.o \
	$(irtdma_obj)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	cp -f  irtdma.ko $(DRIVERS_OUTPUT)
	
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
