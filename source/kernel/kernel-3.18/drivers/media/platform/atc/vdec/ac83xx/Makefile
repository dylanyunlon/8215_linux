#
# This confidential and proprietary software may be used only as
# authorised by a licensing agreement from Autochips Limited
# (C) COPYRIGHT 2010-2012 Autochips Limited
# ALL RIGHTS RESERVED
# The entire notice above must be reproduced on all authorised
# copies and copies may only be made to the extent permitted
# by a licensing agreement from Autochips Limited.
#
#

ifndef CONFIG_TRUSTZONE_SUPPORT
CONFIG_TRUSTZONE_SUPPORT=false
endif

ifeq ($(CONFIG_TRUSTZONE_SUPPORT),true)
vdec-objs := vdec_mod.o
else
vdec-objs := vdec_mod.o lib.a
endif

obj-$(CONFIG_ATC_MEDIA_SUPPORT_VDEC) += vdec.o

ifeq ($(CONFIG_TRUSTZONE_SUPPORT),false)
$(src)/lib.a:
	@cp -f $(srctree)/drivers/misc/atc/efuse/lib.a_shipped $(src)/lib.a
endif

ccflags-y += -I$(srctree)/drivers/media/platform/atc/mmisc/$(CONFIG_ATC_PLATFORM)
ccflags-y += -I$(srctree)/drivers/media/platform/atc/stc/$(CONFIG_ATC_PLATFORM)
ccflags-y += -I$(srctree)/drivers/misc/atc/inc
ccflags-y += -I$(srctree)/drivers/misc/atc/private_inc
ccflags-y += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
