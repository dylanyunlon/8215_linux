TARGET := arm
CUR_DIR := $(shell pwd)
export SYSROOT_DIR
export KERNEL_TOPDIR

LOCAL_SRC_FILES := $(wildcard $(CUR_DIR)/src/utils/*.c) \
                   $(wildcard $(CUR_DIR)/src/vdec/*.c) \
                   $(wildcard $(CUR_DIR)/src/vsink/*.c) \
                   $(wildcard $(CUR_DIR)/src/avcodec/*.cpp)

LOCAL_INCLUDES :=  \
  $(SYSROOT_DIR)/usr/include \
  $(CUR_DIR)/include \
  $(CUR_DIR)/src/utils \
  $(CUR_DIR)/src/vdec \
  $(CUR_DIR)/src/vsink \
  $(CUR_DIR)/src/avcodec \
  $(CUR_DIR)/../atcomx-1.0/openmax/ \
  $(SYSROOT_DIR)/usr/include/glib-2.0/ \
  $(SYSROOT_DIR)/usr/lib/glib-2.0/include/ \
  $(KERNEL_TOPDIR)/include/media/atc/ \
  $(KERNEL_TOPDIR)/drivers/ \
  $(KERNEL_TOPDIR)/drivers/misc/atc/inc/

LOCAL_LIB_PATH := $(TARGET_TOP)/lib

LOCAL_SHARE_LIBRARIES := -lc -ldl -latcmmisc -lglib-2.0 
 
ifeq ($(SURFACE_CS_SUPPORT),1)
LOCAL_SHARE_LIBRARIES += -lsurface_client_atc
CPPFLAGS += -DSUPPORT_SURFACE_CS
else
LOCAL_SHARE_LIBRARIES += -lsurface_atc
endif

LOCAL_INSTALL_FILES := include/atcomxvdecinst.h \
  include/atcvideosink.h

LOCAL_MODULE := libdirectrender.so

.PHONY: all clean
all : $(LOCAL_MODULE)

LOCAL_OBJS := $(LOCAL_SRC_FILES:.c=.o)
INCLUDE_FLAGS      += $(addprefix -I,$(LOCAL_INCLUDES))
vpath %.h $(LOCAL_INCLUDES)

CFLAGS += -fPIC -Wall -Wno-write-strings

$(LOCAL_MODULE) : $(LOCAL_OBJS)
	$(CC) $(INCLUDE_FLAGS) $(CFLAGS) -shared $(LFLAGS) $^ -o $@ $(LOCAL_SHARE_LIBRARIES)
	$(MAKE) --directory=test

%.o:%.c
	$(CC) $(INCLUDE_FLAGS) $(CFLAGS) $(LFLAGS) -c $< -o $@

.PHONY : clean
clean :
	rm -f $(LOCAL_OBJS)
	rm -f $(LOCAL_MODULE)
