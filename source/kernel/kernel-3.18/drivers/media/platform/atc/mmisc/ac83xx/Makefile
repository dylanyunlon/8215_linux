
drvmmisc-objs = memmanager.o mmisc_mmlog.o mmisc_osemem.o mmisc_util.o mmisc_compat32_ioctl.o mmisc_mod.o

obj-$(CONFIG_ATC_MEDIA_SUPPORT_MMISC)			+= drvmmisc.o

ccflags-y		 += -Idrivers/misc/atc/inc
ccflags-y		 += -Idrivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
ccflags-y		 += -Idrivers/media/platform/atc/mmisc/$(CONFIG_ATC_PLATFORM)
ccflags-y		 += -DCONFIG_SECTION_BUILD_LINUX_PROG=0  -DCONFIG_SECTION_BUILD_LINUX_KO=0  -DCONFIG_SECTION_BUILD=0 
ccflags-y		 += -DCONFIG_DRV_TVE_SONY_E -DCONFIG_DRV_TVE_DOT_MESSAGE_SHOWING_E
