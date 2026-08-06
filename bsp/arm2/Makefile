#
# Makefile for the MT33xx ARM2
# 
#

O ?= $(ARM2TOPDIR)/obj
export ARM2TOPDIR    :=  $(CURDIR)
export ARM2BUILDDIR  :=  $(ARM2TOPDIR)/build
export ARM2OBJDIR    :=  $(O)
export ARM2INFODIR   :=  $(O)/info
export ARM2TARGETDIR :=  $(O)/target

#this variable is passed from Android.mk
export $(target_project)

include $(ARM2BUILDDIR)/config.mk

ifeq "$(CONFIG_PLATFORM)" "AC83XX"
SUBDIRS       := \
                 $(ARM2TOPDIR)/speech \
                 $(ARM2TOPDIR)/pwm \
                 $(ARM2TOPDIR)/freertos \
                 $(ARM2TOPDIR)/display \
                 $(ARM2TOPDIR)/tvd \
                 $(ARM2TOPDIR)/wch \
                 $(ARM2TOPDIR)/vcp \
                 $(ARM2TOPDIR)/src \
                 $(ARM2TOPDIR)/pcm_play \
				 $(ARM2TOPDIR)/vba \
				 $(ARM2TOPDIR)/lib/libawtk/src/awtk-1.8 \
				 $(ARM2TOPDIR)/lib/libawtk/platform/8317 \
				 $(ARM2TOPDIR)/app/awtk/ridedash_dual_theme \

else ifeq "$(CONFIG_PLATFORM)" "AC823X"
SUBDIRS    := $(ARM2TOPDIR)/src \
              $(ARM2TOPDIR)/wch \
              $(ARM2TOPDIR)/tvd \
              $(ARM2TOPDIR)/display_823x \
              $(ARM2TOPDIR)/backcar
#              $(ARM2TOPDIR)/speech \
#              $(ARM2TOPDIR)/pcm_play 

else
 $(error "Error platform define~ please check var CONFIG_PLATFORM!")
endif


ifeq "$(CONFIG_PLATFORM)" "AC83XX"
    STATICLIBS := $(ARM2TOPDIR)/gpio/atc_gpio.a 
endif

STATICLIBS += $(ARM2TOPDIR)/speech/aec_abf_arm9rvct40.a
STATICLIBS += $(ARM2OBJDIR)/lib/rsv/librsv.a
STATICLIBS += $(ARM2OBJDIR)/lib/libfdt/libfdt.a

TVDLIBDIR = $(ARM2TOPDIR)/obj
SPEECHLIB = $(ARM2TOPDIR)/speech/aec_abf_arm9rvct40.a
GPIOLIB = $(ARM2TOPDIR)/gpio/atc_gpio.a

ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))
ifeq "$(ATC_OS_NAME)" "android"
 LOCAL_CFLAGS += -DVCP_FOR_ANDROID
endif

TARGETOBJ  := $(ARM2BUILDDIR)/arm2.bin
ifeq "$(CONFIG_PROJECT)" "__ANDROID__"
$(warning "arm2 Compile for android")
LDSCRIPT   := $(ARM2BUILDDIR)/arm2.lds

else

ifeq "$(CONFIG_PLATFORM)" "AC83XX"
$(warning  "arm2 for ac8317 Linux")
LDSCRIPT   := $(ARM2BUILDDIR)/arm2_83xx_Linux.lds
else
$(warning  "Linux - not not not ac8317")
LDSCRIPT   := $(ARM2BUILDDIR)/arm2.lds
endif
endif
include $(ARM2BUILDDIR)/elf.mk

