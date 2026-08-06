CC=arm-buildroot-linux-gnueabi-gcc
INCLUDE=-I./include

SRCS = ./storage_utils.c

OBJS=$(SRCS:.c=.o)

TARGET_BIN=storage_utils
TARGET_LIB=libstorage_utils.so

all: $(TARGET_BIN) $(TARGET_LIB)


$(TARGET_LIB): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@  -L$(TARGET_DIR)/usr/lib $(INCLUDE) -shared -fPIC

$(TARGET_BIN): $(TARGET_LIB)
	$(CC) $(LDFLAGS) main.c $(TARGET_LIB) -o $@  -L$(TARGET_DIR)/usr/lib $(INCLUDE)

%.o: %.c
	$(CC)  $(CFLAGS) -Wall -o $@ -c $< $(INCLUDE)

clean:
	rm -f *.o
