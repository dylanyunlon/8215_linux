# Makefile for musicplayer library
#
# Build: libmusicplayer.so
# Dependencies: libatcmediaplayer.so (prebuild), libmpd_db_bridge.so
#
# Follows the same build pattern as other AC8317 packages.

# Variables from build system (passed via .mk)
TOPDIR      ?= $(shell cd ../../../.. && pwd)
CC          ?= $(TOPDIR)/out/host/bin/arm-buildroot-linux-gnueabi-g++
SYSROOT_DIR ?= $(TOPDIR)/out/host/arm-buildroot-linux-gnueabi/sysroot

MPD_DB_DIR   = $(TOPDIR)/source/packages/application/musicplayer/awtk_app/src/mpd_db

TARGET      = libmusicplayer.so

# Source files (both are now C++)
CPP_SRCS    = music_scanner.cpp music_player.cpp

CPP_OBJS    = $(CPP_SRCS:.cpp=.o)
OBJS        = $(CPP_OBJS)

# Compiler flags
CFLAGS      = -Wall -Wextra -fPIC -O2
# mpd_db_bridge.h is under mpd_db/, include its parent so #include "mpd_db/mpd_db_bridge.h" works
CFLAGS     += -I$(TOPDIR)/source/packages/application/musicplayer/awtk_app/src
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
CFLAGS     += -I$(TOPDIR)/source/packages/application/musicplayer/awtk_app/src/sqlite3
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

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

install: $(TARGET)
	install -m 0755 -D $(TARGET) $(STAGING_DIR)/usr/lib/$(TARGET)
	install -m 0644 -D music_scanner.h $(STAGING_DIR)/usr/include/music_scanner.h
	install -m 0644 -D music_player.h $(STAGING_DIR)/usr/include/music_player.h

.PHONY: all clean install