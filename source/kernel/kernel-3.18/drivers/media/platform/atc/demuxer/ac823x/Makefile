
cfasubdir := ../cfa/ape ../cfa/asf ../cfa/audio ../cfa/avi ../cfa/flv ../cfa/mkv ../cfa/mp4 ../cfa/mpg ../cfa/ogm ../cfa/rm ../cfa/ts ../cfa/audin

demuxersubdir := $(cfasubdir) ../cfa decrypt util esm gau parser pbbuf hal splitter stream

find_files=$(patsubst $(srctree)/$(src)/%.c,%.o, $(wildcard $(srctree)/$(src)/$(drv_demuxer_module)/*.c))

demuxer_obj = $(foreach drv_demuxer_module, $(demuxersubdir), $(find_files))

drvdmx-objs := $(demuxer_obj) dmx_inst.o dmx_os.o dmx_compat_ioctl32.o dmx_mod.o

obj-$(CONFIG_ATC_MEDIA_SUPPORT_DEMUXER)				+= drvdmx.o

ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/util
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/cfa
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/esm
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/gau
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/pbbuf
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/stream
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/parser
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/splitter
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/decrypt
ccflags-y += -I$(srctree)/drivers/media/platform/atc/demuxer/$(CONFIG_ATC_PLATFORM)/hal
ccflags-y += -I$(srctree)/drivers/media/platform/atc/mmisc/$(CONFIG_ATC_PLATFORM)
ccflags-y += -I$(srctree)/drivers/media/platform/atc/stc/$(CONFIG_ATC_PLATFORM)
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/ac83xx/audio/include
ccflags-y += -I$(srctree)/drivers/misc/atc/inc
ccflags-y += -I$(srctree)/drivers/misc/atc/private_inc
ccflags-y += -I$(srctree)/drivers/misc/atc/include
ccflags-y += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc


