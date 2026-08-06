LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ffmpeg_metadata_jni
LOCAL_CFLAGS := -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0
LOCAL_SRC_FILES := com_hcn_metadata_Retriever.cpp \
	    MediaMetadataRetriever.cpp \
        ffmpeg_mediametadataretriever.c \
        ffmpeg_utils.c
LOCAL_SHARED_LIBRARIES := libswscale libavcodec libavformat libavutil
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../ffmpeg/$(TARGET_ARCH_ABI)/include
LOCAL_LDLIBS := -llog
LOCAL_LDLIBS += -landroid
LOCAL_LDLIBS += -ljnigraphics

include $(BUILD_SHARED_LIBRARY)
