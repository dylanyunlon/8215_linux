
PWD := $(shell pwd)
#EXTRA_CFLAGS := -O0 -g
#EXTRA_CFLAGS := -g
#EXTRA_CFLAGS := -I$(src)/
#EXTRA_CFLAGS += -I$(src)/../inc
#EXTRA_CFLAGS += -I$(src)/../../../../arch/arm/mach-ac83xx/include/mach/
#EXTRA_CFLAGS += -I$(KERNELDIR)/include/generated/
#EXTRA_CFLAGS += -I$(src)/../libosal/inc
#EXTRA_CFLAGS += -I$(src)/../private_inc
#EXTRA_CFLAGS += -I$(src)/../oal/
#EXTRA_CFLAGS += -DCHIP_VER_AC83XX

#ccflags-y := -I$(srctree)/arch/arm/mach-ac83xx/include/mach/
ccflags-y += -I$(srctree)/drivers/misc/atc/inc/
ccflags-y += -I$(srctree)/drivers/misc/atc/include/
ccflags-y += -I$(srctree)/drivers/misc/atc/private_inc/
ccflags-y += -I$(srctree)/include/generated/

$(warning $(srctree))

obj-m += dualarmdrv.o

dualarmdrv-objs	:= dual_hal.o dualarm.o

#hw_sema.o 

