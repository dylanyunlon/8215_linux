#
# Makefile for the cli driver (for Linux Kernel 3.0.+)
#
#

PWD := $(shell pwd)
#EXTRA_CFLAGS := -O0 -g
EXTRA_CFLAGS := -I$(src)/../private_inc
#EXTRA_CFLAGS += -I$(DRIVERS_INC)/
#EXTRA_CFLAGS += -I$(src)/../libosal/inc/
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
EXTRA_CFLAGS += -I$(src)/../inc/
EXTRA_CFLAGS += -I$(src)/../aud/audio/include/
#EXTRA_CFLAGS += -I$(KERNELDIR)/drivers/media/platform/atc/demuxer/include
EXTRA_CFLAGS += -I$(KERNELDIR)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/include
EXTRA_CFLAGS += -I$(src)/../input/
EXTRA_CFLAGS += -I$(src)/../../kernel/drivers/serial/
EXTRA_CFLAGS += -I$(src)/../oal/
EXTRA_CFLAGS += -I$(src)/../ybr_vga/hal/
EXTRA_CFLAGS += -I$(KERNELDIR)/arch/arm/mach-ac83xx/include/mach/
EXTRA_CFLAGS += -I$(KERNELDIR)/include/generated/
EXTRA_CFLAGS += -I$(srctree)/include/media/atc
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/inc
#EXTRA_CFLAGS += -I$(KERNELDIR)include/config/arch/
#EXTRA_CFLAGS += -I$(KERNELDIR)include/asm-generic
#EXTRA_CFLAGS += -I$(KERNELDIR)kernel/linux
#EXTRA_CFLAGS += -I$(KERNELDIR)kernel/irq
EXTRA_CFLAGS += -DCHIP_VER_AC83XX
EXTRA_CFLAGS += -I$(PUBLIC_DRIVERS_DIR)/btdrv/
EXTRA_CFLAGS += -DKERNEL_STANDARD_API

obj-m = drvcli.o
drvcli-objs :=          \
    stl_lib.o           \
    cli_drv_cmd.o       \
    cli_input.o         \
    cli_parser.o        \
    cli_printf.o        \
    cli_uart.o          \
    cli_basic_cmd.o     \
    cli_aud_cmd.o       \
    cli_dmx_cmd.o       \
   #cli_uart_cmd.o      \
   #cli_vid_cmd.o       \
   #cli_mtz_cmd.o       \
   #cli_ybr_cmd.o       \
   #cli_bt_cmd.o		\
   #cli_keypad_cmd.o    \
   #cli_gpio_cmd.o      \
   #cli_i2c_cmd.o       \
   

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	cp -f  drvcli.ko $(PWD)/../../../../../../out/atc/ac8317/rel/root/usr/drivers/
	#cp -f drvcli.ko $(DRIVERS_OUTPUT)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

