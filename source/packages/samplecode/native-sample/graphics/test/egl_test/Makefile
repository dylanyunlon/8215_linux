CXX=arm-buildroot-linux-gnueabi-g++

TARGET := arm

LOCAL_SRC_FILES := fb-egl-sample.cpp

LOCAL_SHARE_LIBRARIES := -lEGL -lGLESv2 -lMali -lm

LDFLAGS := $(LOCAL_SHARE_LIBRARIES)

LOCAL_MODULE := egl_test

.PHONY: all
all: $(LOCAL_MODULE)

$(LOCAL_MODULE): $(LOCAL_SRC_FILES)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $(LOCAL_SRC_FILES)