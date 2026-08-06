
CC=arm-buildroot-linux-gnueabi-gcc

LDFLAGS= -shared -lstdc++ -std=c++11

LOCAL_INCLUDES += \
	-I ./command \
	-I ./common \
	-I ./packet \
	-I ./service \
	-I ./transport \
	-I ./utils \
	-I ./../libidc/include \
    -I $(TARGET_DIR)/usr/include \
	-I $(TOPDIR)/../source/packages/cluster/libidc/include \
	-I $(TOPDIR)/../source/packages/cluster/cluster_app/src \

$(info build path: $(TOPDIR)/../source/packages/cluster/libidc/include)
	
LOCAL_SRC_FILES += \
	$(wildcard ./command/*.cpp) \
	$(wildcard ./common/*.cpp) \
	$(wildcard ./packet/*.cpp) \
	$(wildcard ./service/*.cpp) \
	$(wildcard ./transport/*.cpp) \
	$(wildcard ./utils/*.cpp) \

LDFLAGS= -shared -lstdc++ -std=c++11 -lidc
TARGET_LIB=libcluster-service.so

all: $(TARGET_LIB)
$(TARGET_LIB): $(LOCAL_SRC_FILES)
	$(CC) $(LOCAL_INCLUDES) $(LDFLAGS)  -fPIC -Wall $^ -o $@  -lcrypto -lsurface_atc -L$(TARGET_DIR)/usr/lib $(LOCAL_INCLUDES)
clean:
	-rm -rf libcluster-service.so
.PHONY: clean all
