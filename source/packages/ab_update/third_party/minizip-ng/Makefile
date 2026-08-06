# Minizip Makefile
MAKEFILE      = Makefile
CXX := arm-buildroot-linux-gnueabi-g++
CC := arm-buildroot-linux-gnueabi-gcc
# Compiler flags
CFLAGS += -DMZ_ZIP_NO_ENCRYPTION -DMZ_ZLIB -D_POSIX_C_SOURCE=200809L\
          -DMZ_ZIP_NO_CRYPTO  -DHAVE_ZLIB  -DHAVE_STDINT_H -DHAVE_INTTYPE_H \
          -DZLIB_COMPAT -D_LARGEFILE64_SOURCE -I. -Icompat -fPIC

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

#MINIZIP_LDFLAGS = -L./../zlib-1.2.3 -lz

# Default target
all: $(TARGET_LIB)

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create static library
$(TARGET_LIB): $(OBJECTS)
	$(CC) -shared -fPIC  $^ -o $@

# Install library and headers
install:$(TARGET_LIB)
	install -d -m 0755 $(TARGET_DIR)/usr/lib
	install -m 0777 $(TARGET_DIR)/usr/lib

clean:
	@echo "Cleaning minizip build files..."
	@rm -f $(OBJECTS) $(TARGET_LIB)

