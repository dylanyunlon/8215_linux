TARGET := aarch64
LOCAL_MODULE := idc_server_test

LOCAL_SRC_FILES := ./server_test.cpp \

LOCAL_INCLUDES := ../../include/ \
		$(DA_SYSROOT)/usr/include/CommonAPI-3.1/ \
		$(DA_SYSROOT)/usr/include/dbus-1.0/ \
		$(DA_SYSROOT)/usr/lib64/dbus-1.0/include/ \
		$(DA_SYSROOT)/usr/include/ \
		../../src-gen/ 

CPPFLAGS += -std=c++11

LOCAL_SHARE_LIBRARIES := -lidc -lstdc++ -lpthread -lsurface_atc -lCommonAPI -lCommonAPI-DBus -ldbus-1


include $(BUILD_EXECUTABLE)