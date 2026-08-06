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
  EXTRA_CFLAGS += -DDEBUG

ccflags-y += -I$(srctree)/drivers/media/platform/atc/mmisc/$(CONFIG_ATC_PLATFORM)
ccflags-y += -I$(srctree)/drivers/media/platform/atc/stc/$(CONFIG_ATC_PLATFORM)
ccflags-y += -I$(srctree)/drivers/media/atc
ccflags-y += -I$(srctree)/drivers/misc/atc/inc
ccflags-y += -I$(srctree)/drivers/misc/atc/private_inc
ccflags-y += -I$(srctree)/drivers/misc/atc/libosal/ac83xx/inc
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/adsp/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/apcm/comm/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/apcm/hardware/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/apcm/stream/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/apcm/sndcard/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/apcm/unit_test/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/pcm/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/$(PLATFORM_NAME)/audio/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/audmhl/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/audin/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/asv/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/comm/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/hal/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/gpsmix/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/adc/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/aout/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/asrc/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/bypass/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/ckgen/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/dac/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/io/includle
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/iomg/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/linein/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/micin/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/mline/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/pcm/include
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/register
ccflags-y += -I$(srctree)/drivers/media/platform/atc/aud/periph/test

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
APCM_COMM_FILES = $(patsubst $(srctree)/$(src)/apcm/comm/%.c,apcm/comm/%.o,$(wildcard $(srctree)/$(src)/apcm/comm/*.c))
APCM_HW_FILES = $(patsubst $(srctree)/$(src)/apcm/hardware/%.c,apcm/hardware/%.o,$(wildcard $(srctree)/$(src)/apcm/hardware/*.c))
APCM_STRM_FILES = $(patsubst $(srctree)/$(src)/apcm/stream/%.c,apcm/stream/%.o,$(wildcard $(srctree)/$(src)/apcm/stream/*.c))
APCM_SNDCARD_FILES = $(patsubst $(srctree)/$(src)/apcm/sndcard/%.c,apcm/sndcard/%.o,$(wildcard $(srctree)/$(src)/apcm/sndcard/*.c))
APCM_UNIT_TEST_FILES = $(patsubst $(srctree)/$(src)/apcm/unit_test/%.c,apcm/unit_test/%.o,$(wildcard $(srctree)/$(src)/apcm/unit_test/*.c))
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
        $(APCM_COMM_FILES) \
	$(APCM_HW_FILES) \
	$(APCM_STRM_FILES) \
	$(APCM_SNDCARD_FILES) \
	$(APCM_UNIT_TEST_FILES) \
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
	lib/lib_adec.a_shipped

adec-obj := $(adec-y)

$(src)/lib/lib_adec.a_shipped: FORCE
	@cp -f $(KERNEL_EXT_LIBS)/libs/libs_aud_$(PLATFORM_NAME).a $(src)/lib/lib_adec.a_shipped

$(src)/lib/lib_8k.a: FORCE
	@cp -f $(srctree)/$(src)/lib/aec_abf_08K_neon.rvct40.glibc.a_shipped $(src)/lib/lib_8k.a

$(src)/lib/lib_16k.a: FORCE
	@cp -f $(srctree)/$(src)/lib/enh_arm_neon.rvct40.glibc.a_shipped $(src)/lib/lib_16k.a

