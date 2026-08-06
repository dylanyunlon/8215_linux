TARGET := arm

CPPFLAGS += --sysroot=$(DA_SYSROOT) -D HAVE_CONFIG_H

LOCAL_SRC_FILES := atcplayersample-kb.cpp atcplayersample.cpp

ifneq ($(PKG_CONFIG_GST),0.1)

LOCAL_INCLUDES := \
  $(KERNEL_SRC)/include/media/atc/ \
  $(KERNEL_SRC)/drivers/ \
  $(KERNEL_SRC)/drivers/misc/atc/inc/ \
	$(DA_SYSROOT)/usr/include/glib-2.0/ \
	$(DA_SYSROOT)/usr/include/libxml2/ \
	$(DA_SYSROOT)/usr/lib/glib-2.0/include/ \
	$(DA_SYSROOT)/usr/include/gstreamer-1.0/ \
	$(DA_TOP)/lib/include/breakpad \

	
LOCAL_SHARE_LIBRARIES := -losal -lsurface_atc -lmmrender_atc -lgstreamer-1.0 -lgstatcutils-1.0 -lglib-2.0 -lmsdk -lpthread

else

LOCAL_INCLUDES := \
  $(KERNEL_SRC)/include/media/atc/ \
  $(KERNEL_SRC)/drivers/ \
  $(KERNEL_SRC)/drivers/misc/atc/inc/ \
  $(DA_SYSROOT)/usr/include/ \
	$(DA_SYSROOT)/usr/include/glib-2.0/ \
	$(DA_SYSROOT)/usr/include/libxml2/ \
	$(DA_SYSROOT)/usr/lib/glib-2.0/include/ \
	$(DA_SYSROOT)/usr/include/gstreamer-0.10/ \
	$(DA_TOP)/misc/osal/include  \
	$(DA_TOP)/lib/include \
	$(DA_TOP)/lib/include/breakpad \
	$(MM_PATH)/gstreamer/0.10.36/gst-libs/gst/atcutils  \
	$(MM_PATH)/gstreamer/0.10.36/gst-libs/gst  \
	$(MM_PATH)/msdk/include  \
	$(MM_PATH)/render/include \
	$(GRAPHICS_PATH)/surface/include  \
	
LOCAL_SHARE_LIBRARIES := -losal -lsurface_atc -lmmrender_atc -lgstreamer-0.10 -lgstatcutils-0.10 -lglib-2.0 -lmsdk -lpthread

endif

LOCAL_MODULE := atcplayersample

#LOCAL_LIB_PATH := $(DA_LIBDIR)/lib $(DA_SYSROOT)/usr/lib $(DA_SYSROOT)/lib

LOCAL_STATIC_LIBRARIES := -lbreakpad_client

include $(BUILD_EXECUTABLE)

