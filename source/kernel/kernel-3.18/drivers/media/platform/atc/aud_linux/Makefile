#
# Makefile for the framebuffer driver (for Linux Kernel 3.18)
# 
#

PLATFORM_NAME := $(subst ",,$(CONFIG_ATC_PLATFORM))

ifeq "$(PLATFORM_NAME)" "ac83xx"
  EXTRA_CFLAGS += -DCONFIG_DRV_AUD_AC83XX
endif

ifeq "$(PLATFORM_NAME)" "ac823x"
  EXTRA_CFLAGS += -DCONFIG_DRV_AUD_AC823X
endif


  EXTRA_CFLAGS += -DBT_SCO_USERSPACE_ENHANCEMENT

ccflags-y += -I$(srctree)/drivers/media/platform/atc/mmisc/$(CONFIG_ATC_PLATFORM)
ccflags-y += -I$(srctree)/drivers/media/platform/atc/stc/$(CONFIG_ATC_PLATFORM)
ccflags-y += -I$(srctree)/drivers/media/atc
ccflags-y += -I$(srctree)/drivers/misc/atc/inc
ccflags-y += -I$(srctree)/drivers/misc/atc/private_inc
ccflags-y += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/adsp/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/pcm/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/$(PLATFORM_NAME)/audio/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/audmhl/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/audin/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/asv/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/comm/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/hal/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/gpsmix/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/adc/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/aout/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/asrc/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/bypass/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/ckgen/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/dac/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/io/includle
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/iomg/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/linein/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/micin/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/mline/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/pcm/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/register
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud_linux/periph/test

ifndef KERNEL_STANDARD_CONFIG
KERNEL_STANDARD_CONFIG = false
endif

ifeq ($(KERNEL_STANDARD_CONFIG),true)
ccflags-y += -DKERNEL_STANDARD_API
endif



ADSP_FILES = $(patsubst $(srctree)/$(src)/adsp/%.c,adsp/%.o,$(wildcard $(srctree)/$(src)/adsp/*.c))
ASV_FILES = $(patsubst $(srctree)/$(src)/asv/%.c,asv/%.o,$(wildcard $(srctree)/$(src)/asv/*.c))
AUDIN_FILES = $(patsubst $(srctree)/$(src)/audin/%.c,audin/%.o,$(wildcard $(srctree)/$(src)/audin/*.c))
AUDIO_FILES = $(patsubst $(srctree)/$(src)/$(PLATFORM_NAME)/audio/%.c,$(PLATFORM_NAME)/audio/%.o,$(wildcard $(srctree)/$(src)/$(PLATFORM_NAME)/audio/*.c))
AUDMHL_FILES = $(patsubst $(srctree)/$(src)/audmhl/%.c,audmhl/%.o,$(wildcard $(srctree)/$(src)/audmhl/*.c))
COMM_FILES = $(patsubst $(srctree)/$(src)/comm/%.c,comm/%.o,$(wildcard $(srctree)/$(src)/comm/*.c))
GPSMIX_FILES = $(patsubst $(srctree)/$(src)/gpsmix/%.c,gpsmix/%.o,$(wildcard $(srctree)/$(src)/gpsmix/*.c))
PCM_FILES = $(patsubst $(srctree)/$(src)/pcm/%.c,pcm/%.o,$(wildcard $(srctree)/$(src)/pcm/*.c))
PERIPH_ADC_FILES = $(patsubst $(srctree)/$(src)/periph/adc/%.c,periph/adc/%.o,$(wildcard $(srctree)/$(src)/periph/adc/*.c))
PERIPH_AOUT_FILES = $(patsubst $(srctree)/$(src)/periph/aout/%.c,periph/aout/%.o,$(wildcard $(srctree)/$(src)/periph/aout/*.c))
PERIPH_ASRC_FILES = $(patsubst $(srctree)/$(src)/periph/asrc/%.c,periph/asrc/%.o,$(wildcard $(srctree)/$(src)/periph/asrc/*.c))
PERIPH_BYPASS_FILES = $(patsubst $(srctree)/$(src)/periph/bypass/%.c,periph/bypass/%.o,$(wildcard $(srctree)/$(src)/periph/bypass/*.c))
PERIPH_CKGEN_FILES = $(patsubst $(srctree)/$(src)/periph/ckgen/%.c,periph/ckgen/%.o,$(wildcard $(srctree)/$(src)/periph/ckgen/*.c))
PERIPH_DAC_FILES = $(patsubst $(srctree)/$(src)/periph/dac/%.c,periph/dac/%.o,$(wildcard $(srctree)/$(src)/periph/dac/*.c))
PERIPH_IO_FILES = $(patsubst $(srctree)/$(src)/periph/io/%.c,periph/io/%.o,$(wildcard $(srctree)/$(src)/periph/io/*.c))
PERIPH_IOMG_FILES = $(patsubst $(srctree)/$(src)/periph/iomg/%.c,periph/iomg/%.o,$(wildcard $(srctree)/$(src)/periph/iomg/*.c))
PERIPH_LINEIN_FILES = $(patsubst $(srctree)/$(src)/periph/linein/%.c,periph/linein/%.o,$(wildcard $(srctree)/$(src)/periph/linein/*.c))
PERIPH_MICIN_FILES = $(patsubst $(srctree)/$(src)/periph/micin/%.c,periph/micin/%.o,$(wildcard $(srctree)/$(src)/periph/micin/*.c))
PERIPH_MLINE_FILES = $(patsubst $(srctree)/$(src)/periph/mline/%.c,periph/mline/%.o,$(wildcard $(srctree)/$(src)/periph/mline/*.c))
PERIPH_PCM_FILES = $(patsubst $(srctree)/$(src)/periph/pcm/%.c,periph/pcm/%.o,$(wildcard $(srctree)/$(src)/periph/pcm/*.c))
PERIPH_TEST_FILES = $(patsubst $(srctree)/$(src)/periph/test/%.c,periph/test/%.o,$(wildcard $(srctree)/$(src)/periph/test/*.c))

$(warning $(AUDIO_FILES))

obj-$(CONFIG_ATC_MEDIA_SUPPORT_AUDIO) += adec.o
adec-y := \
	adec_mod.o \
	hal/aud_hal_afifo.o \
	$(ADSP_FILES) \
	$(ASV_FILES) \
	$(AUDIN_FILES) \
	$(AUDIO_FILES) \
	$(AUDMHL_FILES) \
	$(COMM_FILES) \
	$(GPSMIX_FILES) \
	$(PCM_FILES) \
	$(PERIPH_ADC_FILES) \
	$(PERIPH_AOUT_FILES) \
	$(PERIPH_ASRC_FILES) \
	$(PERIPH_BYPASS_FILES) \
	$(PERIPH_CKGEN_FILES) \
	$(PERIPH_DAC_FILES) \
	$(PERIPH_IO_FILES) \
	$(PERIPH_IOMG_FILES) \
	$(PERIPH_LINEIN_FILES) \
	$(PERIPH_MICIN_FILES) \
	$(PERIPH_MLINE_FILES) \
	$(PERIPH_PCM_FILES) \
	$(PERIPH_TEST_FILES) \
	lib/lib_8k.a \
	lib/lib_16k.a \
	lib/lib_adec_linux.a_shipped

adec-obj := $(adec-y)

AUD_PROTECT_DIR := $(TOPDIR)/../source/kernel/klibs/aud_protect

$(src)/lib/lib_adec_linux.a_shipped: FORCE
	@if [ -d "$(AUD_PROTECT_DIR)" ]; then \
		echo "audio protect code is exist, copy lib.a to dest"; \
		cp -f $(KERNEL_EXT_LIBS)/libs/libs_aud_linux.a $(src)/lib/lib_adec_linux.a_shipped; \
	else \
		echo "audio protect code is not exist, copy prebuild lib.a to dest"; \
		cp -f $(TOPDIR)/../prebuild/audio/lib.a $(src)/lib/lib_adec_linux.a_shipped; \
	fi

$(src)/lib/lib_8k.a: FORCE
	@cp -f $(srctree)/$(src)/lib/aec_abf_08K_neon.rvct40.glibc.a_shipped $(src)/lib/lib_8k.a

$(src)/lib/lib_16k.a: FORCE
	@cp -f $(srctree)/$(src)/lib/enh_arm_neon.rvct40.glibc.a_shipped $(src)/lib/lib_16k.a

