-include $(ARM2TOPDIR)/build/config.mk

CSRC :=  aud_aout.c aud_comm.c aud_dac.c aud_pinmux.c arm2pcmplay.c

COBJ := $(subst .c,.o,$(CSRC))

ALLOBJS := $(COBJ)

#Use absolute path for finding the source code
NOWDIR  = $(shell pwd)

LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/mach/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/drivers/misc/atc/inc/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/drivers/misc/atc/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/drivers/media/platform/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/include/media/atc/
#LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/include/uapi/


#include $(PROJECT_DIR)/board_config.mak
AUD_DAC_TYPE := PWM_DAC
ifeq ($(AUD_DAC_TYPE), PWM_DAC)
LOCAL_CFLAG += -DDAC_TYPE_PWM
else ifeq ($(AUD_DAC_TYPE), EXT_DAC)
LOCAL_CFLAG += -DDAC_TYPE_EXT
endif

TARGETOBJ    := $(ARM2OBJDIR)/arm2_pcm.o
TARGETOBJDIR := $(ARM2OBJDIR)/pcm_play/obj

include $(ARM2BUILDDIR)/module_common.mk
	
