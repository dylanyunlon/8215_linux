TARGET := aarch64
LOCAL_MODULE := idc_test

LOCAL_SRC_FILES := ./test.cpp \
LOCAL_INCLUDES := \
		../../include/ \
		$(DA_SYSROOT)/usr/include/

CPPFLAGS += -std=c++11

LOCAL_SHARE_LIBRARIES := -lidc -lpthread

include $(BUILD_EXECUTABLE)