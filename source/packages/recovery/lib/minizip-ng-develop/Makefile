# Minizip Makefile

MAKEFILE      = Makefile


CXX := arm-buildroot-linux-gnueabi-g++
CC := arm-buildroot-linux-gnueabi-gcc
# Compiler flags
CFLAGS += -DMZ_ZIP_NO_ENCRYPTION -DMZ_ZLIB -D_POSIX_C_SOURCE=200809L\
          -DMZ_ZIP_NO_CRYPTO  -DHAVE_ZLIB  -DHAVE_STDINT_H -DHAVE_INTTYPE_H \
          -DZLIB_COMPAT -D_LARGEFILE64_SOURCE -I. -Icompat  -I../recovery -fPIC

export recovery_build := $(recovery_top)/build
export recovery_update := $(recovery_top)/recovery-update
export recovery_os := $(recovery_top)/os
export recovery_out := $(recovery_top)/out
export recovery_lib := $(recovery_top)/lib
export recovery_sysroot := $(recovery_out)/sysroot

# Source files
SOURCES = \
    compat/ioapi.c \
    compat/unzip.c \
    compat/zip.c \
    mz_os.c \
    mz_os_posix.c \
    mz_strm.c \
    mz_strm_buf.c \
    mz_strm_mem.c \
    mz_strm_os_posix.c \
    mz_strm_zlib.c \
    mz_zip.c \
    mz_zip_rw.c \
    mz_crypt.c \
    mz_strm_split.c 

OBJECTS = $(SOURCES:.c=.o)
TARGET_LIB = libminizip.so

# Installation directories

LIBDIR ?= $(recovery_sysroot)/lib

# Default target
all: $(TARGET_LIB)

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create static library
$(TARGET_LIB): $(OBJECTS)
	$(AR) rcs $@ $^

# Install library and headers
install:$(TARGET_LIB)
	install -d -m 0755 $(recovery_sysroot)/lib
	install -m 0777 $(TARGET_LIB) $(recovery_sysroot)/lib

# Clean build files
clean:
	@echo "Cleaning minizip build files..."
	@rm -f $(OBJECTS) $(TARGET_LIB)

# Create build directory and run cmake (to match your project's pattern)
build:
	@echo "Creating build directory..."
	@mkdir -p build
	@cd build