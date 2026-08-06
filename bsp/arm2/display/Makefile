-include $(ARM2TOPDIR)/build/config.mk

ifeq ($(TRACK_FEATURE_ENABLED), true)

#use 16 bits track image or default 32 bits track image ?
ifeq ($(TRACK_BITS), 16)
$(warning  "display use 16bits track image")
LOCAL_CFLAG += -DUSE_16BITS_TRACK_IMAGE=1
else
$(warning  "display use default 32bits track image")
# default setting, nothing to do
endif

#endif of TRACK_FEATURE_ENABLED
endif

OSD_SRC  = $(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac83xx/osd
VDP_SRC  = $(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac83xx/vdp
PMX_SRC  = $(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac83xx/pmx
SCL_SRC  = $(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac83xx/scl
TCON_SRC = $(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac83xx/tcon

CUR_CSRC     = display.c LcdShow8x24.c bl_os_layer.c mrf.c ./irtdma_hal/irt_dma_hw.c  ./irtdma_hal/irtdma_drv.c ./imgresz_hal/imgresz_drv.c ./imgresz_hal/imgresz_hal.c ./imgresz_hal/imgresz_hal_if.c
KER_CSRC     = $(wildcard $(OSD_SRC)/*.c) 
KER_CSRC    += $(wildcard $(OSD_SRC)/hal/*.c) 
KER_CSRC    += $(wildcard $(VDP_SRC)/*.c) 
KER_CSRC    += $(wildcard $(PMX_SRC)/*.c) 
KER_CSRC    += $(wildcard $(SCL_SRC)/*.c) 
KER_CSRC    += $(wildcard $(TCON_SRC)/*.c) 
CUR_COBJ     = $(patsubst %.c,%.o,$(CUR_CSRC))
KER_COBJ     = $(patsubst %.c,%.o,$(KER_CSRC))
TMP = $(notdir $(KER_COBJ))

#Use absolute path for finding the source code
NOWDIR  = $(shell pwd)

SOBJ	 = $(patsubst %.S,%.o,$(SSRC))	

ALLOBJS := $(CUR_COBJ) $(KER_COBJ) $(SOBJ)

LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/mach/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac83xx/inc/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/drivers/misc/atc/private_inc/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/drivers/misc/atc/inc/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/include/media/atc/
LOCAL_INCULE += -I$(ATC_KERNEL_DIR)/include/
LOCAL_INCULE += -I$(UBOOT_SRC)
LOCAL_INCULE += -I$(KERNEL_OBJTREE)/include/


AUD_DAC_TYPE := PWM_DAC
ifeq ($(GPIO124_FUN_TYPE), OUT_LOW)
LOCAL_CFLAG += -DOUT_LOW
endif

LVDS_OUTPUT_MODE := 8BITS
ifeq ($(LVDS_OUTPUT_MODE), AUTO)
LOCAL_CFLAG += -DLVDS_OUTPUT_MODE=2
endif

ifeq ($(LVDS_OUTPUT_MODE), 6BITS)
LOCAL_CFLAG += -DLVDS_OUTPUT_MODE=0
endif

ifeq ($(LVDS_OUTPUT_MODE), 8BITS)
LOCAL_CFLAG += -DLVDS_OUTPUT_MODE=1
endif

REAR_OUTPUT_MODE := NTSC
ifeq ($(REAR_OUTPUT_MODE), NTSC)
LOCAL_CFLAG += -DREAR_OUTPUT_MODE=2
else
LOCAL_CFLAG += -DREAR_OUTPUT_MODE=3
endif

#SUBLINKOBJ = arm2_display
#SUBLINKOBJDIR = $(ARM2OUTDIR)/display/obj

LDSCRIPT := $(ARM2BUILDDIR)/arm2_dram.lds

TARGETOBJ    := $(ARM2OBJDIR)/arm2_display.o
TARGETOBJDIR := $(ARM2OBJDIR)/display/obj

include $(ARM2BUILDDIR)/module_common.mk
