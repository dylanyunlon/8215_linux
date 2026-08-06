#
# Makefile for the dvpagent driver (for Linux Kernel 3.0.+)
# 

ifeq ($(KERNELDIR),)
KERNELDIR := $(PWD)/../../../../../kernel/
endif

PWD := $(shell pwd)

#EXTRA_CFLAGS := -O0 -g
EXTRA_CFLAGS := -I$(src)/../private_inc
#EXTRA_CFLAGS += -I$(DRIVERS_INC)/
EXTRA_CFLAGS += -I$(src)/../libosal/$(CONFIG_ATC_PLATFORM)/inc/
EXTRA_CFLAGS += -I$(src)/../inc/
EXTRA_CFLAGS += -I$(src)/../input/
EXTRA_CFLAGS += -I$(src)/../oal/
EXTRA_CFLAGS += -I$(KERNELDIR)/arch/arm/mach-ac83xx/include/mach/
EXTRA_CFLAGS += -I$(KERNELDIR)/include/misc/atc/metazone/
EXTRA_CFLAGS += -I$(KERNELDIR)/drivers/scsi/sym53c8xx_2/
EXTRA_CFLAGS += -I$(KERNELDIR)/include/media/atc/
EXTRA_CFLAGS += -I$(KERNELDIR)/include/generated/
EXTRA_CFLAGS += -I$(KERNELDIR)/include/linux/

obj-m = hdcp2xdrv.o 

hdcp2xdrv-objs := hdcp2x_drv.o hdcp2x_mod.o

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules 

	cp -f  hdcp2xdrv.ko $(PWD)/../../../../../../out/atc/ac8317/rel/root/usr/drivers/
	#cp dvpagent.ko $(DRIVERS_OUTPUT)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

