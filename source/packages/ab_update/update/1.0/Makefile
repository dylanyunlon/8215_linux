
CC=arm-buildroot-linux-gnueabi-gcc

LDFLAGS= -lpthread
DEFINES=
INCLUDE=-I./include/ \
		-I$(TOPDIR)/../out/build/ac83xx/dbus-1.12.10  \
		-I$(TOPDIR)/../out/build/ac83xx/libminizip-1.0/

CFLAGS= -std=c++1z -Wall -Wextra -O2 -fPIC

ifeq ($(AC83XX_BOOT_DEVICE),emmc)
CFLAGS+= -DCONFIG_EMMC_BOOT
endif

ifeq ($(AC83XX_BOOT_DEVICE),nand)
CFLAGS+= -DCONFIG_NAND_BOOT
endif


#CFLAGS += -DSTRESS_ZIP_READ


SRCS :=./src/main.cpp \
	./src/updater/Updater.cpp \
	./src/utils/Util.cpp \
	./src/utils/md5.cpp \
	./src/utils/Bspatch.cpp \
	./src/worker/Worker.cpp \
	./src/worker/Action.cpp \
	./src/Observer.cpp \
	./src/ipc/IPC.cpp \
	./src/worker/ActionFactory.cpp \
	./src/bootctrl/BootCtrl.cpp \
	./src/gpt/GPT.cpp \
	./src/utils/CRC32.cpp \
	./src/utils/File.cpp \
	./src/utils/Image.cpp \
	./src/utils/Version.cpp \
	./src/ipc/socket/SocketProto.cpp \
	./src/ipc/socket/SocketServer.cpp \
	./src/ipc/socket/SocketRPC.cpp \
	./src/mtd/libmtd.cpp \
	./src/mtd/libmtd_legacy.cpp

OBJS=$(SRCS:.cpp=.o)

TARGET=update.bin

# bootctrl tool
BOOTCTRL_TOOL_SRC := ./src/tools/bootctrl_tool.cpp
BOOTCTRL_TOOL_OBJ := $(BOOTCTRL_TOOL_SRC:.cpp=.o)
BOOTCTRL_TOOL_TARGET := bootctrl_tool
TOOL_OBJS := $(filter-out ./src/main.o, $(OBJS))

all: $(TARGET) $(BOOTCTRL_TOOL_TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -ldbus-1 -ltinyxml2 -lbzip2  -lminizip -lz -L$(TARGET_DIR)/usr/lib

%.o: %.cpp
	$(CC)  $(CFLAGS) -Wall -o $@ -c $< $(INCLUDE)
%.o: %.c
	$(CC)  $(CFLAGS) -Wall -o $@ -c $< $(INCLUDE)

clean:
	rm -f *.o
	rm -rf $(BOOLCTRL_TOOL_TARGET)

$(BOOTCTRL_TOOL_TARGET): $(BOOTCTRL_TOOL_OBJ) $(TOOL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -ldbus-1 -ltinyxml2 -lbzip2  -lminizip -lz -L$(TARGET_DIR)/usr/lib
