
CC=arm-buildroot-linux-gnueabi-gcc

LDFLAGS= -shared -lstdc++ -std=c++11

LOCAL_INCLUDES += \
               -I $(TOPDIR)/../source/packages/cluster/libidc/include \
               -I ./kernel-headers/ \
               -I $(TARGET_DIR)/usr/include \

LOCAL_SRC_FILES += \
                idcmediadata.cpp \
                idcclientmanager.cpp \

LDFLAGS= -shared -lstdc++ -std=c++11 -lidc -lpthread

LOCAL_INSTALL_FILES := idcclientmanager.h dcmediadata.h

TARGET_LIB := libidcclientmanager.so

include $(BUILD_SHARED_LIBRARY)

all: $(TARGET_LIB)
$(TARGET_LIB): $(LOCAL_SRC_FILES)
	$(CC) $(LOCAL_INCLUDES) $(LDFLAGS)  -fPIC -Wall $^ -o $@  -lcrypto -L$(TARGET_DIR)/usr/lib $(LOCAL_INCLUDES)
clean:
	-rm -rf libidcclientmanager.so
.PHONY: clean all