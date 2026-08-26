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
  CC           = $(TOPDIR)/out/host/bin/arm-buildroot-linux-gnueabi-gcc
  CXX          = $(TOPDIR)/out/host/bin/arm-buildroot-linux-gnueabi-g++
  SYSROOT_DIR  = $(TOPDIR)/out/host/arm-buildroot-linux-gnueabi/sysroot
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
              src/usb_monitor.c \
              src/favorite_manager.c \
              src/cmus_bridge.c

# cmus ported modules (id3/cue/charset — excludes test file)
CMUS_C_SRCS = $(filter-out src/cmus/test_cmus_port.c, $(wildcard src/cmus/*.c))

# music_scanner.c and music_player.cpp are compiled into libmusicplayer.so
# by the musicplayer package. We link against it, not compile them again.

C_SRCS      = $(APP_C_SRCS) $(CMUS_C_SRCS)

# C++ sources (bt_cxx pattern: extern "C" wrapper + STL business logic)
# Issue #A1: C++ adapter layer for music app
CPP_SRCS    = src/music_app_cxx.cpp \
              src/music_app_impl.cpp

C_OBJS      = $(C_SRCS:.c=.o)
CPP_OBJS    = $(CPP_SRCS:.cpp=.o)
OBJS        = $(C_OBJS) $(CPP_OBJS)

# ===========================================================================
# Compiler & linker flags
# ===========================================================================
COMMON_FLAGS = -Wall -Wextra -Wno-unused-parameter -O2 -fPIC
COMMON_FLAGS += -DAWTK_LINUX_FB
COMMON_FLAGS += -DWITH_FS_RES  # Load assets from filesystem, not compiled-in
COMMON_FLAGS += -DUSE_CMUS_ID3 # Enable cmus id3/cue parser via cmus_bridge

# Include paths
INC_FLAGS    = -Isrc
INC_FLAGS   += -Isrc/cmus
INC_FLAGS   += -I$(AWTK_DIR)/src
# OSAL C++ wrappers — 等 lib_mw.a 链入后启用
# INC_FLAGS   += -I$(TOPDIR)/middleware/hal/osal
# INC_FLAGS   += -I$(TOPDIR)/middleware/common/log
INC_FLAGS   += -I$(AWTK_DIR)/src/ext_widgets
INC_FLAGS   += -I$(AWTK_DIR)/3rd

# AWTK Linux-FB port headers (main_loop_linux.h, lcd_linux_fb.h, etc.)
AWTK_PORT_DIR ?= $(TOPDIR)/source/packages/cluster/awtk/awtk-linux-fb/awtk-port
INC_FLAGS   += -I$(AWTK_PORT_DIR)
INC_FLAGS   += -I$(AWTK_PORT_DIR)/input_thread

ifndef HOST_BUILD
  INC_FLAGS += --sysroot=$(SYSROOT_DIR)
  INC_FLAGS += -I$(SYSROOT_DIR)/usr/include
  INC_FLAGS += -I$(TOPDIR)/prebuild/multimedia/mediaplayerserver/include
  INC_FLAGS += -I$(TOPDIR)/prebuild/multimedia/libatcmultimedia/include
  INC_FLAGS += -I$(TOPDIR)/prebuild/multimedia/mmisc/include
  INC_FLAGS += -I$(TOPDIR)/prebuild/multimedia/mmisc/osal/include
  # display.h lives in kernel headers on 126 server
  INC_FLAGS += -I$(TOPDIR)/source/kernel/kernel-3.18/include/media/atc
  # types.h (included by display.h) lives in kernel driver headers
  INC_FLAGS += -I$(TOPDIR)/source/kernel/kernel-3.18/drivers/misc/atc/inc
  # directrender (may exist on some SDK versions)
  INC_FLAGS += -I$(TOPDIR)/source/packages/multimedia/directrender/include
  INC_FLAGS += -I$(TOPDIR)/source/packages/multimedia/directrender/src/utils
endif

CFLAGS      = $(COMMON_FLAGS) $(INC_FLAGS) -std=c11
# -fexceptions -funwind-tables: ARM32 异常展开必须显式保证（与 middleware bt_cxx 一致）
CXXFLAGS    = $(COMMON_FLAGS) $(INC_FLAGS) -std=c++14 -fexceptions -funwind-tables

# Linker flags
LDFLAGS     =

ifndef HOST_BUILD
  LDFLAGS  += --sysroot=$(SYSROOT_DIR)
  LDFLAGS  += -L$(SYSROOT_DIR)/usr/lib
  LDFLAGS  += -L$(AWTK_DIR)/lib
  # libawtk.so / libtkc.so / libegl_devices.so (allmake 编译产物)
  LDFLAGS  += -L$(TOPDIR)/out/build/ac83xx/awtk-1.0/awtk-linux-fb/bin
  LDFLAGS  += -L$(TOPDIR)/out/target/ac83xx/usr/lib
  LDFLAGS  += -lmusicplayer -latcmediaplayer
endif

LDFLAGS    += -lawtk -ltkc -legl_devices -lEGL -lGLESv2 -lpthread -lm -ldl -lstdc++

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