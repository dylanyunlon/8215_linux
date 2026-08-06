CC=arm-buildroot-linux-gnueabi-g++
INCLUDE=-I./


LDFLAGS= -std=c++14 -g
ifeq ($(AC83XX_BOOT_DEVICE),nand)
LDFLAGS += -DCONFIG_ATC_NAND
else
$(info  ****AC83XX_BOOT_DEVICE:$(AC83XX_BOOT_DEVICE)***)
endif
all: aee

aee: ./main.cpp ./dump_ne.cpp dump_ke.cpp ./utils.cpp
	$(CC) $(LDFLAGS) $^ -o $@ -lstdc++ -L$(TARGET_DIR)/usr/lib $(INCLUDE)

clean:
	rm -f *.o
