CC=arm-buildroot-linux-gnueabi-gcc
export KENREL_HEADER_PATH

ifeq ($(ATC_BOOT_DEVICE),nand)
CFLAGS +=-DBOOT_NAND
endif

CFLAGS += -Os \
          -ffunction-sections \
          -fdata-sections \
          -fomit-frame-pointer

LDFLAGS += -Wl,--gc-sections

LOCAL_SRC_FILES := src/AtcDisplaySettings.c	\
	src/AtcLogoSettings.c  \
	src/DualarmDriver.c    \
	src/BootAnimationDrv.c 
	
LOCAL_INCLUDES := -I./include/	\
	-I$(KENREL_HEADER_PATH)/drivers/misc/atc/bootanimation	\
	-I$(KENREL_HEADER_PATH)/drivers/misc/atc/inc	\
	-I$(KENREL_HEADER_PATH)/include/media/atc

CFLAGS += -shared $(LOCAL_INCLUDES)
	
LOCAL_MODULE := libsettings_atc.so

.PHONY: all
all: $(LOCAL_MODULE)

$(LOCAL_MODULE): $(LOCAL_SRC_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS)  -o $@ $(LOCAL_SRC_FILES)
