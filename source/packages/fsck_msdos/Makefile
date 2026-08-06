###############################
# build fsck_msdos
###############################
CC=arm-buildroot-linux-gnueabi-gcc

LDFLAGS= -g -D_BSD_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
INCLUDE=-I. -I./include
TARGET := aarch64

SRCS:= \
    boot.c \
    check.c \
    dir.c \
    fat.c \
    main.c

TARGET=fsck.msdos

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(LDFLAGS) $^ -o $@  -lc -L$(TARGET_DIR)/usr/lib $(INCLUDE)

clean:
	rm -f *.o
