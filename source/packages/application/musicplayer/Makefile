# Makefile for musicplayer library
#
# Build: libmusicplayer.so
# Dependencies: libatcmediaplayer.so (prebuild), libmpd_db_bridge.so
#
# Follows the same build pattern as other AC8317 packages.

# Variables from build system (passed via .mk)
TOPDIR      ?= $(shell cd ../../../.. && pwd)
CC          ?= $(TOPDIR)/out/host/bin/arm-buildroot-linux-gnueabi-g++
CC_C        ?= $(TOPDIR)/out/host/bin/arm-buildroot-linux-gnueabi-gcc
SYSROOT_DIR ?= $(TOPDIR)/out/host/arm-buildroot-linux-gnueabi/sysroot

AWTK_APP_SRC = $(TOPDIR)/source/packages/application/musicplayer/awtk_app/src

TARGET      = libmusicplayer.so

# Source files (both are now C++)
CPP_SRCS    = music_scanner.cpp music_player.cpp

# cmus ID3/CUE parser sources (proper UTF-16/GBK support)
CMUS_C_SRCS = $(AWTK_APP_SRC)/cmus_bridge.c \
              $(filter-out $(AWTK_APP_SRC)/cmus/test_cmus_port.c, $(wildcard $(AWTK_APP_SRC)/cmus/*.c))

CPP_OBJS    = $(CPP_SRCS:.cpp=.o)
CMUS_OBJS   = $(notdir $(CMUS_C_SRCS:.c=.o))
OBJS        = $(CPP_OBJS) $(CMUS_OBJS)

# Compiler flags
CFLAGS      = -Wall -Wextra -Wno-unused-parameter -fPIC -O2
CFLAGS     += -DUSE_CMUS_ID3
CFLAGS     += -I$(AWTK_APP_SRC)
CFLAGS     += -I$(AWTK_APP_SRC)/cmus
CFLAGS     += -I$(TOPDIR)/prebuild/multimedia/mediaplayerserver/include
CFLAGS     += -I$(TOPDIR)/prebuild/multimedia/libatcmultimedia/include
CFLAGS     += -I$(TOPDIR)/prebuild/multimedia/mmisc/include
CFLAGS     += -I$(TOPDIR)/prebuild/multimedia/mmisc/osal/include
CFLAGS     += -I$(TOPDIR)/source/packages/multimedia/directrender/include
CFLAGS     += -I$(TOPDIR)/source/packages/multimedia/directrender/src/utils
# display.h (included by atcmediaplayer.h) and types.h (included by display.h)
CFLAGS     += -I$(TOPDIR)/source/kernel/kernel-3.18/include/media/atc
CFLAGS     += -I$(TOPDIR)/source/kernel/kernel-3.18/drivers/misc/atc/inc
# SQLite3 header (query functions in music_scanner.c use sqlite3 directly)
CFLAGS     += -I$(AWTK_APP_SRC)/sqlite3
CFLAGS     += --sysroot=$(SYSROOT_DIR)

CXXFLAGS    = $(CFLAGS) -std=c++11

LDFLAGS     = -shared -fPIC
LDFLAGS    += --sysroot=$(SYSROOT_DIR)
LDFLAGS    += -L$(SYSROOT_DIR)/usr/lib
LDFLAGS    += -latcmediaplayer
LDFLAGS    += -lpthread

# Rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "[musicplayer] Built: $@"

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

# cmus C sources live in awtk_app/src/cmus/ — must use gcc (not g++)
# because cmus_bridge.c uses GCC nested functions (auto void flush_track)
%.o: $(AWTK_APP_SRC)/cmus/%.c
	$(CC_C) $(CFLAGS) -std=c11 -c $< -o $@

%.o: $(AWTK_APP_SRC)/%.c
	$(CC_C) $(CFLAGS) -std=c11 -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

install: $(TARGET)
	install -m 0755 -D $(TARGET) $(STAGING_DIR)/usr/lib/$(TARGET)
	install -m 0644 -D music_scanner.h $(STAGING_DIR)/usr/include/music_scanner.h
	install -m 0644 -D music_player.h $(STAGING_DIR)/usr/include/music_player.h

.PHONY: all clean install