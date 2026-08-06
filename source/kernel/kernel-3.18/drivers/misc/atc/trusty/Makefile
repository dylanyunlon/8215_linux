#
# Makefile for the drvtee hdmi driver (for Linux Kernel 3.0.*)
# 
#
obj-y				+= teedrv.o
obj-y				+= efuse/
obj-y				+= hdcpdecryptkey/

ccflags-y += -Idrivers/misc/atc/inc
ccflags-y += -Idrivers/misc/atc/private_inc
ccflags-y += -Idrivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
ccflags-y += -Idrivers/media/platform/atc/mhl/include
ccflags-y += -I$(srctree)/arch/arm/mach-ac83xx/include/mach/
ccflags-y += -I$(srctree)/include/media/atc/

