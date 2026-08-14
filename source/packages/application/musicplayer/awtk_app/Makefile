# Makefile for AWTK Music Player Application
#
# Two build modes:
#   make                    - cross-compile for AC8215 ARM target
#   make HOST_BUILD=1       - host x86 build for development/testing
#
# Dependencies:
#   - AWTK library (libawtk.so + headers)
#   - libatcmediaplayer.so (cross-build only)
#   - pthread
#
# Directory layout:
#   awtk_app/
#   ├── src/           ← AWTK UI + controller sources
#   ├── res/           ← AWTK resources (images, UI XML)
#   ├── Makefile       ← this file
#   └── SConscript     ← alternative SCons build (for awtk-linux-fb)

TOPDIR      ?= $(shell cd ../../../.. && pwd)

# ===========================================================================
# Cross-compile toolchain (ARM, AC8215)
# ===========================================================================
ifndef HOST_BUILD
  CC          ?= $(TOPDIR)/out/host/bin/arm-buildroot-linux-gnueabi-gcc
  CXX         ?= $(TOPDIR)/out/host/bin/arm-buildroot-linux-gnueabi-g++
  SYSROOT_DIR ?= $(TOPDIR)/out/host/arm-buildroot-linux-gnueabi/sysroot
  AWTK_DIR    ?= $(TOPDIR)/bsp/arm2/lib/libawtk/src/awtk-1.8
  TARGET      = music_player
else
  CC          = gcc
  CXX         = g++
  SYSROOT_DIR =
  AWTK_DIR    ?= /usr/local/awtk
  TARGET      = music_player_host
endif

# ===========================================================================
# Source files
# ===========================================================================
# AWTK app sources
APP_C_SRCS  = src/main.c \
              src/music_app.c \
              src/music_ui.c \
              src/usb_monitor.c

# Shared library sources (from parent musicplayer dir)
LIB_C_SRCS  = ../music_scanner.c
LIB_CPP_SRCS = ../music_player.cpp

C_SRCS      = $(APP_C_SRCS) $(LIB_C_SRCS)
CPP_SRCS    = $(LIB_CPP_SRCS)

C_OBJS      = $(C_SRCS:.c=.o)
CPP_OBJS    = $(CPP_SRCS:.cpp=.o)
OBJS        = $(C_OBJS) $(CPP_OBJS)

# ===========================================================================
# Compiler & linker flags
# ===========================================================================
COMMON_FLAGS = -Wall -Wextra -Wno-unused-parameter -O2 -fPIC
COMMON_FLAGS += -DAWTK_LINUX_FB

# Include paths
INC_FLAGS    = -Isrc -I..
INC_FLAGS   += -I$(AWTK_DIR)/src
INC_FLAGS   += -I$(AWTK_DIR)/src/ext_widgets
INC_FLAGS   += -I$(AWTK_DIR)/3rd

ifndef HOST_BUILD
  INC_FLAGS += --sysroot=$(SYSROOT_DIR)
  INC_FLAGS += -I$(TOPDIR)/prebuild/multimedia/mediaplayerserver/include
  INC_FLAGS += -I$(TOPDIR)/prebuild/multimedia/libatcmultimedia/include
  INC_FLAGS += -I$(TOPDIR)/prebuild/multimedia/mmisc/include
  INC_FLAGS += -I$(TOPDIR)/prebuild/multimedia/mmisc/osal/include
endif

CFLAGS      = $(COMMON_FLAGS) $(INC_FLAGS) -std=c11
CXXFLAGS    = $(COMMON_FLAGS) $(INC_FLAGS) -std=c++11

# Linker flags
LDFLAGS     =

ifndef HOST_BUILD
  LDFLAGS  += --sysroot=$(SYSROOT_DIR)
  LDFLAGS  += -L$(SYSROOT_DIR)/usr/lib
  LDFLAGS  += -L$(AWTK_DIR)/lib
  LDFLAGS  += -latcmediaplayer
endif

LDFLAGS    += -lawtk -lpthread -lm -ldl

# ===========================================================================
# Build rules
# ===========================================================================
.PHONY: all clean install release

all: $(TARGET)
	@echo "[musicplayer-awtk] Built: $(TARGET)"

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	@echo "[musicplayer-awtk] Cleaned"

install: $(TARGET)
	install -m 0755 -D $(TARGET) $(DESTDIR)/usr/bin/$(TARGET)
	@echo "[musicplayer-awtk] Installed to $(DESTDIR)/usr/bin/$(TARGET)"

# Release: copy binary + resources to a release directory
RELEASE_DIR = release
release: $(TARGET)
	@mkdir -p $(RELEASE_DIR)/bin $(RELEASE_DIR)/res
	cp $(TARGET) $(RELEASE_DIR)/bin/
	cp -r res/* $(RELEASE_DIR)/res/ 2>/dev/null || true
	@echo "[musicplayer-awtk] Release in $(RELEASE_DIR)/"
