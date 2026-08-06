-include $(ARM2TOPDIR)/build/config.mk

CUR_CSRC := aeabi_uidiv.3.c bt_speech.c bt_speech_proc.c bt_perf_stat.c bt_osl.c

COBJ     = $(subst .c,.o,$(CUR_CSRC))

ALLOBJS := $(COBJ)

#Use absolute path for axf finding the source code
NOWDIR  = $(shell pwd)


#CFLAGS += -I$(NOWDIR)/../inc
#CFLAGS += -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/
#CFLAGS += -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/mach/
#CFLAGS += -I$(ATC_KERNEL_DIR)/drivers/video/
#CFLAGS += -I$(ATC_KERNEL_DIR)/drivers/misc/atc/inc/
#CFLAGS += -I$(ATC_KERNEL_DIR)/drivers/misc/atc/
#CFLAGS += -I$(ATC_KERNEL_DIR)/drivers/media/platform/atc/tvd/inc/
#CFLAGS += -I$(ATC_KERNEL_DIR)/include/media/atc/
#CFLAGS += -I$(ATC_KERNEL_DIR)/include/
#CFLAGS += -I$(ATC_KERNEL_DIR)/include/linux/
#CFLAGS += -I$(ATC_KERNEL_DIR)/arch/arm/include/
#CFLAGS += -I$(ATC_KERNEL_DIR)/arch/arm64/include/uapi/
#CFLAGS += -I$(ATC_KERNEL_DIR)/include/uapi/
#CFLAGS += -I./
#CFLAGS += -DARM2_TVD_DRV
CFLAGS += -D__KERNEL__
LOCAL_CFLAG += -D__KERNEL__

#SUBLINKOBJ = arm2_speech
#SUBLINKOBJDIR = $(ARM2OUTDIR)/speech/obj
#CKGENDIR = $(DRIVERS_DIR)/libatcbsp/mtkbsp


TARGETOBJ    := $(ARM2OBJDIR)/arm2_speech.o
TARGETOBJDIR := $(ARM2OBJDIR)/speech/obj

include $(ARM2BUILDDIR)/module_common.mk
