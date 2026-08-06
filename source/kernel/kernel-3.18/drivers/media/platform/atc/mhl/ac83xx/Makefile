#
# Makefile for the mhl hdmi driver (for Linux Kernel 3.0.*)
# 
#

ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))

mhldrv_obj = \
hdmi_lib/edid_data.o \
hdmi_lib/edid_hal.o \
hdmi_lib/hal_io.o \
hdmi_lib/hdmi_debug.o \
hdmi_lib/hdmi_rx_aud_task.o \
hdmi_lib/hdmi_rx_ctrl.o \
hdmi_lib/hdmi_rx_dvi.o \
hdmi_lib/hdmi_rx_hal.o \
hdmi_lib/hdmi_rx_task.o \
hdmi_lib/rx_io.o \
hdmi_lib/vga_table.o \
cbus_lib/mhl_rx_cbus.o \
cbus_lib/mhl_rx_cbus_ctrl.o \
mhl_mod.o 

#mhldrv-objs := $(mhldrv_obj) ../../../../../misc/atc/efuse/lib.a_shipped

ifeq "$(ATC_OS_NAME)" "linux"
ifeq ($(CONFIG_TRUSTZONE_SUPPORT),true)
ccflags-y += -DSUPPORT_KEY_IN_TRUST_ZONE
endif
endif

obj-$(CONFIG_ATC_MEDIA_SUPPORT_MHL) += mhldrv.o 

mhldrv-objs := $(mhldrv_obj)

ccflags-y += -Idrivers/misc/atc/inc
ccflags-y += -Idrivers/misc/atc/private_inc
ccflags-y += -Idrivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
ccflags-y += -Idrivers/misc/atc/tee/
ccflags-y += -Idrivers/media/platform/atc/mhl/ac83xx/include
ccflags-y += -Idrivers/media/platform/atc/mhl/ac83xx/mhl_hid_lib
ccflags-y += -I$(srctree)/include/media/atc/
ccflags-y += -I$(srctree)/include/misc/atc/metazone/
