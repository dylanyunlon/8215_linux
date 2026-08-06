
drvstc-objs = stc_hal.o stc_os.o stc_mod.o

obj-$(CONFIG_ATC_MEDIA_SUPPORT_STC)			+= drvstc.o

ccflags-y += -I$(srctree)/drivers/misc/atc/inc
ccflags-y += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
ccflags-y += -I$(srctree)/drivers/media/platform/atc/stc/$(CONFIG_ATC_PLATFORM)
ccflags-y += -DCONFIG_SECTION_BUILD_LINUX_PROG=0  -DCONFIG_SECTION_BUILD_LINUX_KO=0  -DCONFIG_SECTION_BUILD=0 
ccflags-y += -DCONFIG_DRV_TVE_SONY_E -DCONFIG_DRV_TVE_DOT_MESSAGE_SHOWING_E
