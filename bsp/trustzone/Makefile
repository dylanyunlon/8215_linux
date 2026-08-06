#Compiler
export TOPDIR    = $(CURDIR)
CROSS_COMPILE   := $(TOPDIR)/i686/bin
export CC        = $(CROSS_COMPILE)/armv7a-mediatek451_001_vfp-linux-gnueabi-gcc
export OBJCPY		 =  $(CROSS_COMPILE)/armv7a-mediatek451_001_vfp-linux-gnueabi-objcopy
#Linker
export LINK      =  $(CROSS_COMPILE)/armv7a-mediatek451_001_vfp-linux-gnueabi-ld
export AR          = $(CROSS_COMPILE)/armv7a-mediatek451_001_vfp-linux-gnueabi-ar
export RANLIB = $(CROSS_COMPILE)/armv7a-mediatek451_001_vfp-linux-gnueabi-ranlib
export NM	= $(CROSS_COMPILE)/armv7a-mediatek451_001_vfp-linux-gnueabi-nm
export OBJDUMP = $(CROSS_COMPILE)/armv7a-mediatek451_001_vfp-linux-gnueabi-objdump

export LIBGCC   =$(shell $(CC) --print-libgcc-file-name)
export LIBC     =$(shell $(CC) --print-file-name=libc.a)
export LIBM     =$(shell $(CC) --print-file-name=libm.a)

#Directory
export TOPDIR    = $(CURDIR)
export SRCDIR    = $(TOPDIR)/src
export OBJDIR    = $(TOPDIR)/obj
export HDCPDIR    = $(TOPDIR)/hdcpdecrypt
export TARGETDIR = $(TOPDIR)/target
export INFODIR   = $(TOPDIR)/info
export INCODIR   = $(TOPDIR)/inc
export LIBDIR   = $(TOPDIR)/lib
export RSVDIR   = $(TOPDIR)/rsv
export RSVOBJ   = $(OBJDIR)/lib/rsv

ifeq ($(CONFIG_ARGS_START),)
$(error ***CONFIG_ARGS_START is NULL,please check***)
endif

CFLAGS += -w -g
CFLAGS += -marm

CFLAGS += -DTRUSTZONE
CFLAGS += -DCONFIG_ARGS_START=$(CONFIG_ARGS_START)

#CFLAGS += -I$(TOPDIR)/../kernel/kernel-3.18/drivers/misc/atc/inc
#CFLAGS += -I$(TOPDIR)/../kernel/kernel-3.18/arch/arm/mach-ac83xx/include/mach
#CFLAGS += -I$(TOPDIR)/../bootloader/uboot/uboot-83xx/include/asm-arm/arch-ac83xx
LDFLAGS:=-L $(shell dirname `$(CC) -print-libgcc-file-name`) -lgcc
GLOBAL_INC += -I$(TOPDIR)/inc

OUTPUT  := tz.bin

SUBDIRS   = $(TOPDIR)/serial \
			$(TOPDIR)/src    \
			
default: RSV COMPILER
	@echo "Link..." 
	$(LINK) $(wildcard $(OBJDIR)/*.o) -T $(TOPDIR)/tz.lds $(LDFLAGS) -Bstatic -Map $(INFODIR)/map.txt --verbose --stats --start-group $(RSVOBJ)/librsv.a $(LIBDIR)/libhdcpdecrypt.a $(LIBDIR)/libefuse.a $(LIBDIR)/libtomcrypt.a $(LIBDIR)/libtommath.a $(LIBDIR)/libserial.a  --end-group -o $(TARGETDIR)/tz
	${OBJCPY} -O binary -R .comment -R .note -S -g $(TARGETDIR)/tz $(TARGETDIR)/tz.bin
	$(OBJDUMP) -d $(TARGETDIR)/tz > $(TARGETDIR)/tz.dls
	$(NM) -n $(TARGETDIR)/tz > $(TARGETDIR)/tz.map

RSV:
	echo "rsv begin"
	mkdir -p $(RSVOBJ)
	$(MAKE) -C $(RSVDIR) libs "O=$(RSVOBJ)" "G_INCLUDE=$(GLOBAL_INC)" "SHELL=$(SHELL)" "COMPILER=$(CC)" "ARCHIVER=$(AR)" "COMPILER_FLAGS=$(CFLAGS)  -O2" "EXTRA_COMPILER_FLAGS=-g"
	echo "rsv end"


COMPILER:
	@echo Compiler...
	mkdir -p $(OBJDIR)
	mkdir -p $(TARGETDIR)
	mkdir -p $(INFODIR)
	@for i in $(SUBDIRS); \
	  do \
	    cd $$i  && make  || exit; \
	  done
	  
sub_clean:
	@for i in $(SUBDIRS); \
  do \
    cd $$i  && make clean  || exit; \
  done

clean: sub_clean
	-rm -rf $(OBJDIR)
	-rm -rf $(TARGETDIR)
	-rm -rf $(INFODIR)	

