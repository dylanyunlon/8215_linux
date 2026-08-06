
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
EXTRA_CFLAGS += -I$(srctree)/drivers/media/platform/atc/gfx/ac83xx/

gfx_obj = \
gfx_hal/gfx_cmdque.o \
gfx_hal/gfx_dif.o \
gfx_hal/gfx_hw.o \
gfx_hal/gfx_if.o \
gfx_hal/gfx_hal_if.o \
gfx_hal/gfx_sw.o 

obj-m := gfx.o
gfx-objs = \
	gfx_drv.o \
	$(gfx_obj)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	cp -f  gfx.ko $(DRIVERS_OUTPUT)
	
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
