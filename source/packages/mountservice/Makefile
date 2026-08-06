CC=arm-buildroot-linux-gnueabi-gcc
CFLAGS += -Os -ffunction-sections -fdata-sections
LDFLAGS += -s -Wl,--gc-sections -Wl,--as-needed
INCLUDE=-I. -I./include -I./include/fs -I$(TOPDIR)/../out/build/ac83xx/dbus-1.12.10

ifeq ($(AC83XX_BOOT_DEVICE),emmc)
CFLAGS+= -DCONFIG_EMMC_BOOT
endif

ifeq ($(AC83XX_BOOT_DEVICE),nand)
CFLAGS+= -DCONFIG_NAND_BOOT
endif

SRCS = ./src/ATCMountService.cpp \
	./src/ATCMountServiceListener.cpp \
	./src/ATCMountServiceParser.cpp \
	./src/UeventList.cpp \
	./src/Utils.cpp	\
	./src/Process.cpp \
	./src/fs/Vfat.cpp \

OBJS=$(SRCS:.cpp=.o)

TARGET=mountservice

LOCAL_SHARE_LIBRARIES := -ldbus-1 -lpthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@  -lstdc++ -lpthread -ldbus-1 -L$(TARGET_DIR)/usr/lib $(INCLUDE)
%.o: %.cpp
	$(CC)  $(CFLAGS) -Wall -o $@ -c $< $(INCLUDE)

clean:
	rm -f *.o
