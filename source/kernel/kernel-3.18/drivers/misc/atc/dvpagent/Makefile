#
# Makefile for the dvpagent driver (for Linux Kernel 3.0.+)
#

ifeq ($(KERNELDIR),)
KERNELDIR := $(PWD)/../../../../../kernel-3.18/
endif

PWD := $(shell pwd)

#EXTRA_CFLAGS += -DCHIP_VER_AC83XX
#EXTRA_CFLAGS += -DDVP_PM_SUPPORT
#EXTRA_CFLAGS += -lpthread


ccflags-y += -I$(srctree)/drivers/misc/atc/libosal/ac83xx/inc/
ccflags-y += -I$(srctree)/drivers/misc/atc/inc/
ccflags-y += -I$(srctree)/drivers/misc/atc/aud/audio/include/
ccflags-y += -I$(srctree)/drivers/misc/atc/multimedia/demuxer/include/
ccflags-y += -I$(srctree)/drivers/misc/atc/input/
ccflags-y += -I$(srctree)/drivers/misc/atc/oal/
ccflags-y += -I$(srctree)/include/misc/atc/metazone/
ccflags-y += -I$(srctree)/drivers/scsi/sym53c8xx_2/
ccflags-y += -I$(srctree)/include/media/atc/
ccflags-y += -I$(srctree)/arch/arm/mach-ac83xx/include/mach/
ccflags-y += -I$(srctree)/include/generated/
ccflags-y += -I$(srctree)/include/linux/

ccflags-y += -DDVP_PM_SUPPORT
ccflags-y += -DCHIP_VER_AC83XX

obj-m = dvpagent.o

dvpagent-objs := dvpcomhw.o dvp_protocol.o agent.o agent_drv.o dvp_mod.o dvp_fsIo.o dvp_fs.o dvp_hal.o \
                 lib/atc_dvp_load.a

$(src)/lib/atc_dvp_load.a: FORCE
	@cp -f $(KERNEL_EXT_LIBS)/libs/atc_dvp_load.a $(src)/lib/atc_dvp_load.a

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

#	cp -f  dvpagent.ko $(PWD)/../../../../../../out/atc/ac8317/rel/root/usr/drivers/
	#cp dvpagent.ko $(DRIVERS_OUTPUT)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
