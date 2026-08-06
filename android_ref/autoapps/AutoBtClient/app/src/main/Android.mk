LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, java)
LOCAL_STATIC_JAVA_LIBRARIES := hcn_bluetooth android-support-v4 theme-utils
LOCAL_STATIC_JAVA_AAR_LIBRARIES := \
    skin-support-android
LOCAL_AAPT_FLAGS := --auto-add-overlay
LOCAL_PACKAGE_NAME := AutoBtClient
LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_REQUIRED_MODULES := hcn_bluetooth
LOCAL_PRIVATE_PLATFORM_APIS := true
include $(BUILD_PACKAGE)


