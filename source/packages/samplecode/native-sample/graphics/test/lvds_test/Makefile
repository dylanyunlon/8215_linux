CC=arm-buildroot-linux-gnueabi-gcc

TARGET := arm

LOCAL_SRC_FILES := lvds_test.c

LOCAL_SHARE_LIBRARIES := -lsettings_atc

LDFLAGS := $(LOCAL_SHARE_LIBRARIES)

LOCAL_MODULE := lvds_test

.PHONY: all
all: $(LOCAL_MODULE)

$(LOCAL_MODULE): $(LOCAL_SRC_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(LOCAL_SRC_FILES)
