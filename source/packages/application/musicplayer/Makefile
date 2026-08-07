# Makefile for musicplayer library
#
# Build: libmusicplayer.so
# Dependencies: libatcmediaplayer.so (prebuild)
#
# Follows the same build pattern as other AC8317 packages.

# Variables from build system (passed via .mk)
TOPDIR      ?= $(shell cd ../../../.. && pwd)
CC          ?= $(TOPDIR)/out/host/bin/arm-buildroot-linux-gnueabi-g++
SYSROOT_DIR ?= $(TOPDIR)/out/host/arm-buildroot-linux-gnueabi/sysroot

TARGET      = libmusicplayer.so

# Source files
C_SRCS      = music_scanner.c
CPP_SRCS    = music_player.cpp

C_OBJS      = $(C_SRCS:.c=.o)
CPP_OBJS    = $(CPP_SRCS:.cpp=.o)
OBJS        = $(C_OBJS) $(CPP_OBJS)

# Compiler flags
CFLAGS      = -Wall -Wextra -fPIC -O2
CFLAGS     += -I$(TOPDIR)/prebuild/multimedia/mediaplayerserver/include
CFLAGS     += -I$(TOPDIR)/prebuild/multimedia/libatcmultimedia/include
CFLAGS     += -I$(TOPDIR)/prebuild/multimedia/mmisc/include
CFLAGS     += -I$(TOPDIR)/prebuild/multimedia/mmisc/osal/include
CFLAGS     += -I$(TOPDIR)/source/packages/multimedia/directrender/include
CFLAGS     += -I$(TOPDIR)/source/packages/multimedia/directrender/src/utils
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
