TARGET := arm

LOCAL_SRC_FILES := logorw.c
 
LOCAL_MODULE := liblogo.so

LOCAL_INCLUDES :=  $(LOCAL_PATH) \
	$(TOPDIR)/../bsp/arm2/display/ \
	$(TOPDIR)/../bsp/bootloader/uboot/uboot-83xx/drivers/displayoslayer/

CC := arm-buildroot-linux-gnueabi-gcc

.PHONY: all clean
all:$(LOCAL_MODULE)

LOCAL_OBJS := $(notdir $(LOCAL_SRC_FILES:.c=.o))
INCLUDE_FLAGS += $(addprefix -I, $(LOCAL_INCLUDES))
CFLAGS += -fPIC -Wall

$(LOCAL_MODULE) : $(LOCAL_OBJS)
	$(CC) $(INCLUDE_FLAGS) $(CLAGS) -shared $^ -o $@ $(LOCAL_SHARE_LIBRARIES)

%.o:%.c
	$(CC) $(INCLUDE_FLAGS) $(CFLAGS) -c $< -o $@

.PHONY : clean
clean:
	rm rf $(LOCAL_OBJS)
	rm -rf $(LOCAL_MODULE)
