# drivers/barcelona/gps/Makefile
#
# Makefile for the Barcelona GPS driver.
#
# Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
# Author: Dimitry Andric <dimitry.andric@tomtom.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
ifeq ($(KERNELDIR),)
KERNELDIR := $(PWD)/../../../../../../kernel/
endif

PWD := $(shell pwd)
#EXTRA_CFLAGS := -O0 -g
EXTRA_CFLAGS := -I$(src)/
EXTRA_CFLAGS += -I$(src)/../inc
EXTRA_CFLAGS += -I$(src)/../oal
#EXTRA_CFLAGS += -I$(src)/../misc/atc/inc


obj-m = gps.o 


default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	cp -f  gps.ko $(PWD)/../../../../../../out/atc/ac8317/rel/root/usr/drivers/
# cp -f  gps.ko $(DRIVERS_OUTPUT)
  
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

# EOF
