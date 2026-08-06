#select the correct platform
PLATFORM_NAME := $(subst ",,$(CONFIG_ATC_PLATFORM))

ifeq "$(PLATFORM_NAME)" "ac83xx"
	DRV_IMGRESZ_IP_PLATFORM := ac83xx
else ifeq "$(PLATFORM_NAME)" "ac823x"
	DRV_IMGRESZ_IP_PLATFORM := ac823x
endif

obj-y += $(DRV_IMGRESZ_IP_PLATFORM)/


