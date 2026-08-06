#
#
#
ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))
ifeq "$(ATC_OS_NAME)" "android"
ccflags-y += -DVCP_FOR_ANDROID
endif
ccflags-y += -I$(srctree)/drivers/misc/atc/inc
ccflags-y += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
ccflags-y += -I$(srctree)/include/media/

obj-$(CONFIG_VCP_ATC) := vcp.o
vcp-objs := vcp_drv.o hal/cp.o
