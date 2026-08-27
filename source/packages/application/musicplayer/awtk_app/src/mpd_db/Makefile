# Makefile for libmpd_db_bridge.so
#
# Build: shared library exporting the C API in mpd_db_bridge.h
# Consumers: libmusicplayer.so, music_player (awtk_app binary)
#
# Contains: MPD SimpleDatabase C++ core + SQLite persistence + C bridge

TOPDIR      ?= $(shell cd ../../../../../../.. && pwd)

ifndef HOST_BUILD
  CXX         = $(TOPDIR)/out/host/bin/arm-buildroot-linux-gnueabi-g++
  SYSROOT_DIR = $(TOPDIR)/out/host/arm-buildroot-linux-gnueabi/sysroot
else
  CXX         = g++
  SYSROOT_DIR =
endif

TARGET      = libmpd_db_bridge.so

# All .cxx sources under this directory tree (excluding test files)
CXX_SRCS    = $(filter-out test_debug.cxx test_mpd_db.c, \
                $(shell find . -name '*.cxx' | sed 's|^\./||'))

CXX_OBJS    = $(CXX_SRCS:.cxx=.o)

# Compiler flags
CXXFLAGS    = -Wall -Wextra -Wno-unused-parameter -O2 -fPIC
CXXFLAGS   += -std=c++17 -fexceptions -funwind-tables
CXXFLAGS   += -I.
CXXFLAGS   += -I$(TOPDIR)/source/packages/application/musicplayer/awtk_app/src/sqlite3

ifndef HOST_BUILD
  CXXFLAGS += --sysroot=$(SYSROOT_DIR)
  CXXFLAGS += -I$(SYSROOT_DIR)/usr/include
endif

LDFLAGS     = -shared -fPIC
LDFLAGS    += -lsqlite3 -lstdc++ -lpthread

ifndef HOST_BUILD
  LDFLAGS  += --sysroot=$(SYSROOT_DIR)
  LDFLAGS  += -L$(SYSROOT_DIR)/usr/lib
endif

.PHONY: all clean install

all: $(TARGET)
	@echo "[mpd_db_bridge] Built: $(TARGET)"

$(TARGET): $(CXX_OBJS)
	$(CXX) $(CXX_OBJS) -o $@ $(LDFLAGS)

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	find . -name '*.o' -delete
	rm -f $(TARGET)

install: $(TARGET)
	install -m 0755 -D $(TARGET) $(STAGING_DIR)/usr/lib/$(TARGET)
	install -m 0644 -D mpd_db_bridge.h $(STAGING_DIR)/usr/include/mpd_db_bridge.h