#
#
#   Compile TVD Code
#   Write by Zhi Qu@atc0057


include $(ARM2TOPDIR)/build/config.mk

ifeq "$(CONFIG_PLATFORM)" "AC83XX"
 CSRC_DIR = $(ATC_KERNEL_DIR)/drivers/media/platform/atc/tvd/ac83xx/tvd_drv
else ifeq "$(CONFIG_PLATFORM)" "AC823X"
 CSRC_DIR = $(ATC_KERNEL_DIR)/drivers/media/platform/atc/tvd/ac823x/tvd_drv
else
 $(error "Error platform define~ please check var CONFIG_PLATFORM!")
endif

ifeq ($(INIT_TVD_BEFORE_ARM2_START_ENABLED), true)
$(warning  "init tvd in uboot/lk")
LOCAL_CFLAG += -DINIT_TVD_BEFORE_ARM2_START
endif

CSRC     = $(wildcard $(CSRC_DIR)/*.c)
COBJ     = $(patsubst %.c,%.o,$(CSRC))
COBJ_NODIR = $(notdir (COBJ))

ALLOBJS := $(COBJ)
   
ifeq "$(CONFIG_PLATFORM)" "AC83XX"
 LOCAL_INCULE := -I$(ATC_KERNEL_DIR)/include/media/atc \
                 -I$(ATC_KERNEL_DIR)/drivers/media/platform/atc/tvd/ac83xx/inc
else ifeq "$(CONFIG_PLATFORM)" "AC823X"
 LOCAL_INCULE := -I$(ATC_KERNEL_DIR)/include/media/atc/ac823x_tvd \
                 -I$(ATC_KERNEL_DIR)/include/media/atc \
                 -I$(ATC_KERNEL_DIR)/drivers/media/platform/atc/tvd/ac823x/inc
else
 $(error "Error platform define~ please check var CONFIG_PLATFORM!")
endif

LDSCRIPT := $(ARM2BUILDDIR)/arm2_dram.lds

TARGETOBJ    := $(ARM2OBJDIR)/arm2_tvd.o
TARGETOBJDIR := $(ARM2OBJDIR)/tvd/obj

include $(ARM2BUILDDIR)/module_common.mk
