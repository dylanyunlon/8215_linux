-include $(ARM2TOPDIR)/build/config.mk

ifeq "$(CONFIG_PLATFORM)" "AC83XX"
LOCAL_CFLAG +=
else ifeq "$(CONFIG_PLATFORM)" "AC823X"

REAR_OUTPUT_MODE := NTSC
ifeq ($(REAR_OUTPUT_MODE), NTSC)
LOCAL_CFLAG += -DREAR_OUTPUT_MODE=2
else
LOCAL_CFLAG += -DREAR_OUTPUT_MODE=3
endif

LOCAL_CFLAG += -DAC823X_CONFIG
LOCAL_CFLAG += -DAC823X_ARM2
else
 $(error "Error platform define~ please check var CONFIG_PLATFORM!")
endif

OSD_DIR = $(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac823x/osd
PMX_DIR = $(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac823x/pmx
SCL_DIR = $(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac823x/scl
TCON_DIR = $(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac823x/tcon
VDP_DIR = $(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac823x/vdp

CUR_CSRC += $(wildcard $(OSD_DIR)/*.c)
CUR_CSRC += $(wildcard $(OSD_DIR)/hal/*.c)
CUR_CSRC += $(wildcard $(PMX_DIR)/*.c)
CUR_CSRC += $(wildcard $(SCL_DIR)/*.c)
CUR_CSRC += $(wildcard $(TCON_DIR)/*.c)
CUR_CSRC += $(wildcard $(VDP_DIR)/*.c)
CUR_CSRC += display.c \
						bl_os_layer.c \
						mrf.c

COBJ     = $(subst .c,.o,$(CUR_CSRC))

ALLOBJS := $(COBJ)

LOCAL_INCULE := -I$(ATC_KERNEL_DIR)/include/media/atc/ac823x/ \
								-I$(ATC_KERNEL_DIR)/drivers/misc/atc/inc/ \
								-I$(ATC_KERNEL_DIR)/drivers/misc/atc/private_inc/ \
 								-I$(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac823x/osd/hal \
 								-I$(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac823x/pmx/ \
 								-I$(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac823x/inc/ \
 								-I$(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac823x/vdp/ \
 								-I$(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac823x/ \
								-I$(ATC_KERNEL_DIR)/include/media/atc \
 								-I$(UBOOT_SRC)/ \
 								-I$(ARM2TOPDIR)/arm2/inc \
 								-I$(ARM2TOPDIR)/arm2/display_823x/




LDSCRIPT := arm2_display_823x.lds

TARGETOBJ    := $(ARM2OBJDIR)/arm2_display_823x.o
TARGETOBJDIR := $(ARM2OBJDIR)/display_823x/obj

include $(ARM2BUILDDIR)/module_common.mk
