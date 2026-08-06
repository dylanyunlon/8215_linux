include $(ARM2TOPDIR)/build/config.mk


CUR_CSRC := BCLib.c \
            custom_protocol.c \
            custom_ui.c

SSRC     = backcarRes.S

ifeq ($(TRACK_FEATURE_ENABLED), true)
$(warning  "backcar track feature enabled")
SSRC += TrackRes.S
CUR_CSRC += track.c
LOCAL_CFLAG += -DARM2_TRACK_ENABLE=1

#use 16 bits track image or default 32 bits track image ?
ifeq ($(TRACK_BITS), 16)
$(warning  "backcar use 16bits track image")
LOCAL_CFLAG += -DUSE_16BITS_TRACK_IMAGE=1
else
$(warning  "backcar use default 32bits track image")
# default setting, nothing to do
endif

# track feature enabled, hide car image on UI
#LOCAL_CFLAG += -DCAR_IMAGE_ON_UI=1

SOBJ	 = $(patsubst %.S,%.o,$(SSRC))
else
$(warning  "no no no backcar track feature enabled")

# no no track feature, show car image on UI
LOCAL_CFLAG += -DCAR_IMAGE_ON_UI=1

endif

ifeq "$(CONFIG_PLATFORM)" "AC823X"
    CUR_CSRC += ac823x_BCdisplay.c
else

#Use absolute path for finding the source code
NOWDIR  = $(shell pwd)

    CUR_CSRC += uart.c
    CUR_CSRC += fsc_sound.c
endif



COBJ	 = $(subst .c,.o,$(CUR_CSRC))
SOBJ	 = $(subst .S,.o,$(SSRC))

ALLOBJS := $(COBJ)

# remove rvc resource obj
#ALLOBJS := $(COBJ) \
#           $(SOBJ)

ifeq ($(INIT_TVD_BEFORE_ARM2_START_ENABLED), true)
$(warning  "init tvd in uboot/lk")
LOCAL_CFLAG += -DINIT_TVD_BEFORE_ARM2_START
endif

ifeq ($(BACKCAR_IN_UPGRADE_MODE_ENABLED), false)
LOCAL_CFLAG += -DDISABLE_BACKCAR_IN_UPGRADE_MODE
endif

ifeq "$(CONFIG_PLATFORM)" "AC83XX"
LOCAL_INCULE := -I$(ARM2TOPDIR)/inc/ \
                -I$(ARM2TOPDIR)/pcm_play/ \
                -I$(ARM2TOPDIR)/display/ \
                -I$(ARM2TOPDIR)/ybr_vga/ \
                -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/ \
                -I$(ATC_KERNEL_DIR)/arch/arm/mach-ac83xx/include/mach/ \
                -I$(ATC_KERNEL_DIR)/drivers/misc/atc/inc/ \
                -I$(ATC_KERNEL_DIR)/drivers/misc/atc/ \
                -I$(ATC_KERNEL_DIR)/drivers/media/platform/atc/tvd/inc/ \
                -I$(ATC_KERNEL_DIR)/include/media/atc/ \
                -I$(ATC_KERNEL_DIR)/include/ \
                -I$(ATC_KERNEL_DIR)/include/linux/ \
                -I$(ATC_KERNEL_DIR)/arch/arm/include/ \
                -I$(ATC_KERNEL_DIR)/arch/arm64/include/uapi/ \
                -I$(ATC_KERNEL_DIR)/include/uapi/ \
                -I$(UBOOT_SRC) \
                -DCHIP_VER_AC83XX \
                -I$(KERNEL_OBJTREE)/ \
                -I$(KERNEL_OBJTREE)/include/
else 
ifeq "$(CONFIG_PLATFORM)" "AC823X"
LOCAL_INCULE := -I$(ARM2TOPDIR)/inc/ \
                -I$(ARM2TOPDIR)/pcm_play/ \
                -I$(ARM2TOPDIR)/ybr_vga/ \
                -I$(ARM2TOPDIR)/display_823x/ \
                -I$(ATC_KERNEL_DIR)/include/media/atc/ac823x_tvd/ \
                -I$(ATC_KERNEL_DIR)/include/media/atc/ac823x/ \
                -I$(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac823x/osd/hal \
                -I$(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac823x/inc/ \
                -I$(ATC_KERNEL_DIR)/drivers/video/fbdev/atc/ac823x/ \
                -I$(UBOOT_SRC)/ \
                -I$(ATC_KERNEL_DIR)/include/media/atc/
else
 #$(error "Error platform define~ please check var CONFIG_PLATFORM!")
endif

endif

ifeq "$(CONFIG_PLATFORM)" "AC83XX"

include $(BOARD_CFG_FILE)

ifeq ($(BACKCAR_DATA_SOURCE), GPIO)
	LOCAL_CFLAG += -DGPIO_ENABLE
endif

ifeq ($(BACKCAR_DATA_SOURCE), UART4)
	LOCAL_CFLAG += -DUART4_ENABLE
endif

ifeq ($(ARM1_EXCEPTION_ARM2_BACKCAR_ENABLE), true)
	LOCAL_CFLAG += -DARM1_EXCEPTION_ARM2_BACKCAR_ENABLE
endif

endif

#LOCAL_CFLAG := -DCHIP_VER_AC83XX
LDSCRIPT := $(ARM2TOPDIR)/backcar/arm2_backcar.lds

TARGETOBJ	:= $(ARM2OBJDIR)/arm2_backcar.o
TARGETOBJDIR := $(ARM2OBJDIR)/backcar/obj

include $(ARM2BUILDDIR)/module_common.mk

