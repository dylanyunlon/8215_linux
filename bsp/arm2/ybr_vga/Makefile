#
#
#   Compile YBR_VGA Code
#


include $(ARM2TOPDIR)/build/config.mk

ifeq "$(CONFIG_PLATFORM)" "AC83XX"
 CSRC_DIR = $(ARM2TOPDIR)/ybr_vga
else ifeq "$(CONFIG_PLATFORM)" "AC823X"
 CSRC_DIR =
else
 $(error "Error platform define~ please check var CONFIG_PLATFORM!")
endif

CSRC     = $(wildcard $(CSRC_DIR)/*.c)
COBJ     = $(patsubst %.c,%.o,$(CSRC))
COBJ_NODIR = $(notdir (COBJ))

ALLOBJS := $(COBJ)
   
ifeq "$(CONFIG_PLATFORM)" "AC83XX"
 LOCAL_INCULE := -I$(ATC_KERNEL_DIR)/include/media/atc
else ifeq "$(CONFIG_PLATFORM)" "AC823X"
 LOCAL_INCULE := 
else
 $(error "Error platform define ~ please check var CONFIG_PLATFORM!")
endif

#LDSCRIPT := $(CSRC_DIR)/arm2_ybr.lds

TARGETOBJ    := $(ARM2OBJDIR)/arm2_ybr.o
TARGETOBJDIR := $(ARM2OBJDIR)/ybr_vga/obj

include $(ARM2BUILDDIR)/module_common.mk
