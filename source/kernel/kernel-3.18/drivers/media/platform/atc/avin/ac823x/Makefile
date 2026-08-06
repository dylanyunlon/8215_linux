#
# Makefile for the avin V4L2 driver (for Linux Kernel 3.18)
# 
#

ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))

ifeq "$(ATC_OS_NAME)" "android"
EXTRA_CFLAGS += -DCVBS_NR_ENABLE
EXTRA_CFLAGS += -DCONFIG_AVM_ENABLE
EXTRA_CFLAGS += -DATC_AOSP_ENHANCEMENT_AUDIO
endif

EXTRA_CFLAGS += -DCONFIG_YBR_ENABLE

ccflags-y += -I$(srctree)/drivers/misc/atc/inc
ccflags-y += -I$(srctree)/drivers/misc/atc/private_inc
ccflags-y += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
ccflags-y += -I$(srctree)/drivers/media/platform/atc/mmisc/$(CONFIG_ATC_PLATFORM)
ccflags-y += -I$(srctree)/drivers/media/platform/atc/tvd/inc
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/esm
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/stream
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/gau
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/cfa
ccflags-y += -I$(srctree)/drivers/media/platform/atc/mhl/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_ac823x/ac83xx/audio/include
#ccflags-y += -I$(srctree)/arch/arm/mach-ac83xx/include/mach/
ccflags-y += -I$(srctree)/include/
ccflags-y += -I$(srctree)/include/media/atc/
ccflags-y += -I$(srctree)/include/media/atc/$(CONFIG_ATC_PLATFORM)

obj-m += avin.o
avin-objs = cvbs_hal.o avm_hal.o atc_capture.o audio_hal.o tvd_wch_mgr.o ybrvga_hal.o
#            digital_hal.o  \

ifeq "$(ATC_OS_NAME)" "android"
#avin-objs += tvp_hal.o
endif


ifeq ($(CONFIG_ATC_OS),"android") 
EXTRA_CFLAGS += -DANDROID
$(src)/libhvdetect.a_shipped: FORCE
	@cp -f $(KERNEL_EXT_LIBS)/libs/libhvdetect.a $(src)/libhvdetect.a_shipped
endif

ifeq ($(CONFIG_ATC_OS),"linux")
EXTRA_CFLAGS += -DLINUX
$(objtree)/drivers/media/platform/atc/avin/ac83xx/libhvdetect.a_shipped:
	@cp $(srctree)/../../../sources/prebuild/screen-hvdetect/image/usr/lib/libhvdetect.a $(objtree)/drivers/media/platform/atc/avin/ac83xx/libhvdetect.a_shipped  
endif 
#$(src)/libhvdetect.a_shipped: FORCE
#	@cp -f $(KERNEL_EXT_LIBS)/libs/libhvdetect.a_shipped $(src)/libhvdetect.a_shipped
#endif
