MAKEFILE      = Makefile


CXX := arm-buildroot-linux-gnueabi-g++
CC := arm-buildroot-linux-gnueabi-gcc
export recovery_build := $(recovery_top)/build
export recovery_update := $(recovery_top)/recovery-update
export recovery_os := $(recovery_top)/os
export recovery_out := $(recovery_top)/out
export recovery_lib := $(recovery_top)/lib
export recovery_sysroot := $(recovery_out)/sysroot

CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT -fPIC --sysroot=$(recovery_sysroot)
CXXFLAGS      = -pipe -O2 -Wall -W -D_REENTRANT -fPIC --sysroot=$(recovery_sysroot) -D_GLIBCXX_USE_CXX11_ABI=0


LINK          = $(CXX)
LFLAGS        = -Wl,-O1 --sysroot=$(recovery_sysroot)  -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -static-libgcc
LIBS          = -L$(recovery_lib)/recovery \
                -L$(recovery_lib)/mtdutils \
                -L$(recovery_lib)/ext4_utils \
                -L$(recovery_lib)/libsparse \
                -L$(recovery_lib)/zlib-1.2.3 \
                -L$(recovery_sysroot)/usr/lib \
                -lrecovery -lmtdutils -lext4_utils -lsparse -lz -lpthread -lminizip


SOURCES       = main.cpp
OBJECTS       = main.o

QMAKE_TARGET  = recovery-update
TARGET        = recovery-update.bin

first: all

main.o: main.cpp
	$(CXX) -c $(CXXFLAGS) -I. \
                -I$(recovery_lib)/recovery \
                -I$(recovery_lib)/recovery/include \
                -I$(recovery_lib)/mtdutils \
                -I$(recovery_lib)/ext4_utils \
                -I$(recovery_lib)/libsparse/include \
                -I$(recovery_lib)/zlib-1.2.3 \
                -o "$@" "$<"

$(TARGET):$(OBJECTS)
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

all: $(TARGET)

clean: 
	rm -f $(OBJECTS)
	rm -f $(TARGET)
	rm -f *~ core *.core

install:$(TARGET)
	install -d -m 0755 $(recovery_sysroot)/bin
	install -m 0777 $(TARGET) $(recovery_sysroot)/bin

.PHONY: all clean install first