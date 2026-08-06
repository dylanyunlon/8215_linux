
CC=arm-buildroot-linux-gnueabi-gcc

DEFINES=
INCLUDE=-I./

CFLAGS= -Wall -Wextra -O2
LDFLAGS= -fPIC -shared

SRCS := blocksort.c \
	huffman.c 	\
	crctable.c \
	randtable.c \
	compress.c \
	decompress.c \
	bzlib.c 

OBJS=$(SRCS:.c=.o)

TARGET_LIB=libbzip2.so

all: $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -L$(TARGET_DIR)/usr/lib

%.o: %.c
	$(CC)  $(CFLAGS) -Wall -o $@ -c $< $(INCLUDE)

clean:
	rm -f *.o
