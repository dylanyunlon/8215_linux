#Use absolute path for axf finding the source code
NOWDIR  = $(CURDIR)
OBJDIR = $(NOWDIR)

CFLAGS += -g
CFLAGS += -I../inc
CFLAGS += -I./include
#CFLAGS += -I$(CURDIR)/../../kernel/kernel-3.18/arch/arm/mach-ac83xx/include/mach
#CFLAGS += -I$(HDCPDIR)/include

INCLUDE += -I../inc
INCLUDE += -I./include

LIBDIR ?= ../lib
AR ?= armv7a-mediatek451_001_vfp-linux-gnueabi-ar
RANLIB ?= armv7a-mediatek451_001_vfp-linux-gnueabi-ranlib

src_file := serial.o printf.o

all: $(src_file)
	$(AR) cr libserial.a $^ 
	$(RANLIB) libserial.a
	cp libserial.a $(LIBDIR)
	 
	
clean:
	rm -rf $(src_file)
	rm -f libserial.a
