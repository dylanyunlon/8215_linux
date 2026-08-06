-include $(ARM2TOPDIR)/build/config.mk

ifeq "$(CONFIG_PLATFORM)" "AC83XX"
 CSRC_DIR = $(ATC_KERNEL_DIR)/drivers/media/platform/atc/wch/ac83xx/hal
 CUR_CSRC := $(wildcard $(CSRC_DIR)/*.c)
else ifeq "$(CONFIG_PLATFORM)" "AC823X"
 CSRC_DIR = $(ATC_KERNEL_DIR)/drivers/media/platform/atc/wch/ac823x/hal
 CUR_CSRC := $(wildcard $(CSRC_DIR)/*.c)
else
 $(error "Error platform define~ please check var CONFIG_PLATFORM!")
endif

COBJ     = $(patsubst %.c,%.o,$(CUR_CSRC))

ALLOBJS := $(COBJ)

ifeq "$(CONFIG_PLATFORM)" "AC83XX"
 LOCAL_INCULE := -I$(ARM2TOPDIR)/inc \
                 -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/ \
                 -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/mach/ \
                 -I$(ATC_KERNEL_DIR)/drivers/misc/atc/inc/ \
                 -I$(ATC_KERNEL_DIR)/drivers/misc/atc/ \
                 -I$(ATC_KERNEL_DIR)/drivers/misc/atc/libosal/ac83xx/inc \
                 -I$(ATC_KERNEL_DIR)/drivers/media/platform/atc/wch/ac83xx/hal/ \
                 -I$(ATC_KERNEL_DIR)/drivers/media/platform/atc/tvd/inc/ \
                 -I$(ATC_KERNEL_DIR)/include/media/atc/ \
                 -I$(ATC_KERNEL_DIR)/include/media/ \
                 -I$(ATC_KERNEL_DIR)/include/ \
                 -I$(ATC_KERNEL_DIR)/include/linux/ \
                 -I$(ATC_KERNEL_DIR)/arch/arm/include/ \
                 -I$(ATC_KERNEL_DIR)/arch/arm64/include/uapi/ \
                 -I$(ATC_KERNEL_DIR)/include/uapi/ \
                 -I./ \
                 -I$(ATC_KERNEL_DIR)/arch/arm/boot/dts/include/dt-bindings/pinctrl \
                 -I$(KERNEL_OBJTREE)/ \
                 -I$(KERNEL_OBJTREE)/include/
else ifeq "$(CONFIG_PLATFORM)" "AC823X"
 LOCAL_INCULE := -I$(ATC_KERNEL_DIR)/include/media/atc/ac823x \
                 -I$(ATC_KERNEL_DIR)/drivers/media/platform/atc/wch/ac823x/include \
                 -I$(UBOOT_SRC)/ \
                 -I$(ARM2TOPDIR)/arm2/inc
else
 $(error "Error platform define~ please check var CONFIG_PLATFORM!")
endif

LDSCRIPT := $(ARM2BUILDDIR)/arm2_dram.lds

TARGETOBJ    := $(ARM2OBJDIR)/arm2_wch.o
TARGETOBJDIR := $(ARM2OBJDIR)/wch/obj

include $(ARM2BUILDDIR)/module_common.mk
