EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/inc
atc-vout-y += atc_vout.o atc_voutlib.o libhvdetect.a_shipped
obj-$(CONFIG_VIDEO_ATC_VOUT) += atc-vout.o

.PHONY:libhvdetect.a_shipped screen_hvdetect.h
drivers/media/platform/atc/vout/libhvdetect.a_shipped:
	cp $(TOPDIR)/../prebuild/screen-hvdetect/image/usr/include/screen_hvdetect.h $(objtree)/drivers/media/platform/atc/vout 
	cp $(TOPDIR)/../prebuild/screen-hvdetect/image/usr/lib/libhvdetect.a $(objtree)/drivers/media/platform/atc/vout/libhvdetect.a_shipped

drivers/media/platform/atc/vout/screen_hvdetect.h:
	cp $(TOPDIR)/../prebuild/screen-hvdetect/image/usr/include/screen_hvdetect.h $(objtree)/drivers/media/platform/atc/vout 
