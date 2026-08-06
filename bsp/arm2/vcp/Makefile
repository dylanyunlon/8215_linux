-include $(ARM2TOPDIR)/build/config.mk

#Use absolute path for axf finding the source code
NOWDIR  = $(shell pwd)

CSRC_DIR = $(ATC_KERNEL_DIR)/drivers/media/platform/atc/vcp/ac83xx/hal
CSRC     = $(wildcard $(CSRC_DIR)/*.c)
COBJ     = $(patsubst %.c,%.o,$(CSRC))
#COBJ_NODIR = $(notdir $(COBJ))

ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))
ifeq "$(ATC_OS_NAME)" "android"
LOCAL_CFLAG += -DVCP_FOR_ANDROID
endif

ALLOBJS := $(COBJ)


LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/mach/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/drivers/misc/atc/inc/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/drivers/misc/atc/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/drivers/media/platform/atc/vcp/ac83xx/hal/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/include/media/atc/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/arch/arm/boot/dts/include/dt-bindings/pinctrl
LOCAL_INCULE += -I$(KERNEL_OBJTREE)/include/


TARGETOBJ    := $(ARM2OBJDIR)/arm2_vcp.o
TARGETOBJDIR := $(ARM2OBJDIR)/vcp/obj

include $(ARM2BUILDDIR)/module_common.mk
