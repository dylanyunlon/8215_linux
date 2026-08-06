TARGET := arm

LOCAL_PATH := $(call my-dir)
MY_LOCAL_PATH := $(LOCAL_PATH)

#CPPFLAGS += --sysroot=$(DA_SYSROOT)

LOCAL_SRC_FILES += fifo/audio_bt_sco_fifo_server.c
LOCAL_SRC_FILES += bt_threads/bt_threads.c
LOCAL_SRC_FILES += bt_threads/alsa_ops.c
LOCAL_SRC_FILES += bt_threads/bt_streams.c
#LOCAL_SRC_FILES += bt_threads/bt_enhancement.c
#LOCAL_SRC_FILES += bt_threads/enh_params.c
LOCAL_SRC_FILES += bt_threads/src_api.c
LOCAL_SRC_FILES += bt_threads/bt_src.c


#LOCAL_LDLIBS += -l_16k -l_8k
LOCAL_INCLUDES += ./../include \
                  ./../../policyaux/include
#\
#                  $(DA_SYSROOT)/usr/include/


LFLAGS += -v

#LOCAL_LDFLAGS +=  ./bt_threads/lib_16k.a

LOCAL_MODULE := audio_bt_server



LOCAL_LIB_PATH := $(DA_SYSROOT)/lib  $(DA_SYSROOT)/usr/lib ./bt_threads  ./

#LOCAL_LIB_PATH := $(DA_SYSROOT)/usr/lib
#LOCAL_STATIC_LIBRARIES := -l_16k -l_8k
#LOCAL_STATIC_LIBRARIES := -l_16k

LOCAL_SHARE_LIBRARIES := -lasound -lpthread
#-lspeech_enh -laudiovolumepolicyclient

#LDFLAGS += -lasound -lpthread

#LOCAL_SHARE_LIBRARIES := -lasound -lpthread  -lspeech_enh_lib  -l_16k -l_8k
#LOCAL_SHARE_LIBRARIES := -lasound -lpthread

include $(BUILD_EXECUTABLE)

