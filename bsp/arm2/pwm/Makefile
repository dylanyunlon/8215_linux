-include $(ARM2TOPDIR)/build/config.mk


CSRC_DIR = $(ATC_KERNEL_DIR)/drivers/pwm/atc/hal
CUR_CSRC     = $(wildcard $(CSRC_DIR)/*.c)
COBJ     = $(patsubst %.c,%.o,$(CUR_CSRC))
#COBJ_NODIR = $(notdir $(COBJ))

ALLOBJS := $(COBJ)

LOCAL_INCULE := -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/

LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/mach/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/drivers/misc/atc/inc/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/drivers/misc/atc/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/include/
#CFLAGS += -I$(ATC_KERNEL_DIR)/include/linux/
#CFLAGS += -I$(ATC_KERNEL_DIR)/include/uapi/
#CFLAGS += -I$(ATC_KERNEL_DIR)/arch/arm/include/
#CFLAGS += -I$(ATC_KERNEL_DIR)/arch/arm64/include/uapi/
#CFLAGS += -I$(ATC_KERNEL_DIR)/arch/arm/include/generated/

#SUBLINKOBJ = arm2_pwm
#SUBLINKOBJDIR = $(ARM2OUTDIR)/pwm/obj

TARGETOBJ    := $(ARM2OBJDIR)/arm2_pwm.o
TARGETOBJDIR := $(ARM2OBJDIR)/pwm/obj

include $(ARM2BUILDDIR)/module_common.mk
