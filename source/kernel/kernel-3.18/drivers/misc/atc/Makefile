#
# Makefile - Autochips Linux driver.
# Copyright(c) 2015, Autochips Corporation.
#
ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))
ATC_PLATFORM_NAME := $(subst ",,$(CONFIG_ATC_PLATFORM))

obj-$(CONFIG_ATC_LIB_OSAL)      += libosal/
obj-$(CONFIG_ATC_LIB_BSP)       += libatcbsp/
obj-y				+= oal/
obj-$(CONFIG_ATC_AC8237_BIM)	+=bim/
obj-$(CONFIG_ATC_BOOTPROF)	    += bootprof/
obj-$(CONFIG_ATC_GPS)		    += gps/

ifneq "$(ATC_PLATFORM_NAME)" "ac823x"
obj-$(CONFIG_ATC_CLI)		    += drvcli/
else
obj-$(CONFIG_ATC_CLI_823x)		+= drvcli_823x/
endif

obj-$(CONFIG_ATC_METAZONE)	    += metazone/
obj-$(CONFIG_ATC_GPS_6630)      += gps_6630/
obj-$(CONFIG_ATC_DVP)           += dvpagent/
obj-$(CONFIG_ATC_BKCAR)	        += backcar/
obj-$(CONFIG_PWR_LOSS_ATC_TEST) += power-loss-test/
#obj-m                          += hdcp2x/

ifeq ($(CONFIG_TRUSTZONE_SUPPORT),true)
obj-$(CONFIG_TRUSTY_ATC)        += trusty/
obj-$(CONFIG_ATC_TEE_SUPPORT)   += tee/
endif

obj-$(CONFIG_ATC_AEE_SUPPORT)   += aee/
obj-$(CONFIG_ATC_RAM_CONSOLE)   += ram_console/
obj-$(CONFIG_ATC_DUAL_ARM)	    += dualarm/
obj-$(CONFIG_ATC_BASE_POWER)    += base/

ifeq "$(ATC_OS_NAME)" "linux"
obj-y                           += bootanimation/
endif

obj-$(CONFIG_ATC_CPUFREQ)       += cpufreq/
obj-y                           += shutdown-ate/
obj-$(CONFIG_ATC_BOOT_STATE)    += boot_state/
obj-$(CONFIG_MTK_BOOT)          += boot/
obj-$(CONFIG_GATOR)             += gator-driver/
obj-y                           += arm2system_service/
#obj-y                           += ddr_bandwith/
