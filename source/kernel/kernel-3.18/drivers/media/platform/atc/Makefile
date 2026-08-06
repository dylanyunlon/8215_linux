ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))
ATC_PLATFORM_NAME := $(subst ",,$(CONFIG_ATC_PLATFORM))

ifeq "$(ATC_OS_NAME)" "linux"
obj-$(CONFIG_ATC_MEDIA_STORAGE_HELPER) += atc_storage_helper/
endif

obj-$(CONFIG_ATC_MEDIA_SUPPORT_MMISC) += mmisc/
obj-$(CONFIG_ATC_MEDIA_SUPPORT_STC) += stc/
obj-$(CONFIG_ATC_MEDIA_SUPPORT_DEMUXER) += demuxer/
obj-$(CONFIG_ATC_MEDIA_SUPPORT_VDEC) += vdec/
ifeq "$(ATC_OS_NAME)" "linux"
$(warning "this is linux, compile aud_linux dir $(ATC_OS_NAME)")
obj-$(CONFIG_ATC_MEDIA_SUPPORT_AUDIO) += aud_linux/
#obj-$(CONFIG_ATC_MEDIA_SUPPORT_YBRVGA) += ybr_vga/
else
$(warning "this is android, compile aud dir $(ATC_OS_NAME)")
ifeq "$(ATC_PLATFORM_NAME)" "ac823x"
obj-$(CONFIG_ATC_MEDIA_SUPPORT_AUDIO) += aud_ac823x/
else
obj-$(CONFIG_ATC_MEDIA_SUPPORT_AUDIO) += aud/
endif

endif
obj-$(CONFIG_ATC_MEDIA_SUPPORT_YBRVGA) += ybr_vga/
obj-$(CONFIG_ATC_MEDIA_SUPPORT_IMGRESZ) += imgresz/
obj-$(CONFIG_ATC_MEDIA_SUPPORT_IRTDMA) += irtdma/
obj-$(CONFIG_ATC_MEDIA_SUPPORT_GFX) += gfx/
obj-$(CONFIG_VIDEO_CVBS) += tvd/
obj-$(CONFIG_VIDEO_CVBS) += avin/
obj-$(CONFIG_WCH_ATC) += wch/
obj-$(CONFIG_VCP_ATC) += vcp/
obj-$(CONFIG_NR_ATC) += nr/
ifeq "$(ATC_OS_NAME)" "linux"
obj-$(CONFIG_VIDEO_ATC_VOUT) += vout/
endif
obj-$(CONFIG_ATC_MEDIA_SUPPORT_MHL) += mhl/
obj-$(CONFIG_MALI400) += mali400/
obj-$(CONFIG_MALI_MIDGARD) += mali_midgard/
