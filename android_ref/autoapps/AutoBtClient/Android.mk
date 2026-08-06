LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
# Module name should match apk name to be installed
LOCAL_MODULE := AutoBtClient
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := AutoBtClient.apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)

LOCAL_CERTIFICATE := platform
include $(BUILD_PREBUILT)