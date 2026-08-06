LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
# Module name should match apk name to be installed
LOCAL_MODULE := AutoMediaPlayer
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := app-release-mt8768.apk
LOCAL_MODULE_CLASS := APPS
LOCAL_CERTIFICATE := platform
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)

PRIVATE_EMBEDDED_JNI_LIBS := 'lib/*.so'
JNI_LIBS :=
$(foreach FILE,$(shell if unzip -l $(LOCAL_PATH)/$(LOCAL_SRC_FILES) $(PRIVATE_EMBEDDED_JNI_LIBS) > /dev/null ; then \
        rm -rf $(LOCAL_PATH)/uncompressedlibs && \
        mkdir $(LOCAL_PATH)/uncompressedlibs; \
        unzip $(LOCAL_PATH)/$(LOCAL_SRC_FILES) $(PRIVATE_EMBEDDED_JNI_LIBS) -d $(LOCAL_PATH)/uncompressedlibs > /dev/null && \
        find $(LOCAL_PATH)/uncompressedlibs -name *.so; \
    fi),$(eval JNI_LIBS += $(FILE)))
$(info echo $(JNI_LIBS))
LOCAL_PREBUILT_JNI_LIBS := $(subst $(LOCAL_PATH),,$(JNI_LIBS))
LOCAL_MULTILIB :=32

include $(BUILD_PREBUILT)
