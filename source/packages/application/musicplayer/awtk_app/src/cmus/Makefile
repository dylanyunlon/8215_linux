# Makefile for cmus ported modules
# Standalone build for verification; in production, integrate into musicplayer_awtk.mk

CC       ?= gcc
CFLAGS   ?= -Wall -I.
# For AC8215 cross-compile, override CC and CFLAGS:
#   make CC=arm-linux-gnueabihf-gcc CFLAGS="-Wall -I. -march=armv7-a"

SRCS = cmus_compat.c xmalloc.c file.c convert.c uchar.c gbuf.c \
       keyval.c id3.c comment.c cue.c cue_utils.c
OBJS = $(SRCS:.c=.o)

# Static library
libcmus_port.a: $(OBJS)
	$(AR) rcs $@ $^

# Test program
test_cmus_port: test_cmus_port.c libcmus_port.a
	$(CC) $(CFLAGS) -o $@ $< -L. -lcmus_port

# Individual object rules
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean test
clean:
	rm -f *.o libcmus_port.a test_cmus_port

test: test_cmus_port
	./test_cmus_port
