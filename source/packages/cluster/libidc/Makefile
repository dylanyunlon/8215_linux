CC=arm-buildroot-linux-gnueabi-gcc

LDFLAGS= -shared -lstdc++ -std=c++11

INCLUDE= \
	-I ./utils/ \
	-I ./src/ \
	-I ./include/ \
	-I ./kernel-headers/ \
	-I $(TARGET_DIR)/usr/include \

LOCAL_SRC_FILES += \
	$(wildcard ./src/*.cpp) \

LDFLAGS= -g
CXXFLAGS := `pkg-config --cflags --libs`
LOCAL_INSTALL_FILES := include/idc_client.h include/idc_common.h include/idc_server.h src/idc_priv.h
CXXFLAGS += $(INCLUDE)
LDFLAGS= -shared -lstdc++ -std=c++11 -lpthread
TARGET_LIB=libidc.so

all: $(TARGET_LIB)
$(TARGET_LIB): $(LOCAL_SRC_FILES)
	$(CC) $(CXXFLAGS) $(LDFLAGS)  -fPIC -Wall $^ -o $@  -lcrypto -L$(TARGET_DIR)/usr/lib $(LOCAL_INCLUDES)
clean:
	-rm -rf libidc.so
.PHONY: clean all