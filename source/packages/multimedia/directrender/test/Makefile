TARGET := arm
CUR_DIR := $(shell pwd)


LOCAL_SRC_FILES := $(wildcard $(CUR_DIR)/*.cpp)

LOCAL_INCLUDES :=  \
  $(CUR_DIR)/../include \
  $(CUR_DIR)/../src/utils \
  $(CUR_DIR)/../../atcomx-1.0/openmax/ \
  $(KERNEL_TOPDIR)/include/media/atc/ \
  $(KERNEL_TOPDIR)/drivers/ \
  $(KERNEL_TOPDIR)/drivers/misc/atc/inc/ \
  $(SYSROOT_DIR)/usr/include/ \
  $(SYSROOT_DIR)/usr/include/glib-2.0/ \
  $(SYSROOT_DIR)/usr/lib/glib-2.0/include/

LOCAL_LIB_PATH := -L$(CUR_DIR)/..

LOCAL_SHARE_LIBRARIES := -lc -ldl
LOCAL_SHARE_LIBRARIES += -lsurface_atc -lpthread -ldirectrender

LOCAL_MODULE := atcdtdemo

.PHONY: all clean
all : $(LOCAL_MODULE)

LOCAL_OBJS := $(LOCAL_SRC_FILES:.c=.o)
INCLUDE_FLAGS      += $(addprefix -I,$(LOCAL_INCLUDES))
vpath %.h $(LOCAL_INCLUDES)

CFLAGS += -fPIC -Wall -g

$(LOCAL_MODULE) : $(LOCAL_OBJS)
	$(CC) $(INCLUDE_FLAGS) $(CFLAGS) $(LFLAGS) $^ -o $@ $(LOCAL_SHARE_LIBRARIES) $(LOCAL_LIB_PATH)

%.o:%.c
	$(CC) $(INCLUDE_FLAGS) $(CFLAGS) $(LFLAGS) -c $< -o $@

.PHONY : clean
clean :
	rm -f $(LOCAL_OBJS)
	rm -f $(LOCAL_MODULE)
