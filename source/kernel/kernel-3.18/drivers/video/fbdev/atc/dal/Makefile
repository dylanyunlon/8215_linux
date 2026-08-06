obj-y += dal.o


EXTRA_CFLAGS += -I$(srctree)/arch/arm/mach-ac83xx/include/mach/
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/libosal/$(CONFIG_ATC_PLATFORM)/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/misc/atc/private_inc

EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/atc/ac83xx/inc
EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/atc/ac83xx/pmx
EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/atc/ac83xx/osd
EXTRA_CFLAGS += -I$(srctree)/drivers/video/fbdev/atc/ac83xx

EXTRA_CFLAGS += -I$(srctree)/drivers/staging/android
EXTRA_CFLAGS += -I$(srctree)/include/media/atc
#osd_files = $(patsubst $(srctree)/$(src)/../ac83xx/osd/hal/%.c,osd/hal/%.o,$(wildcard $(srctree)/$(src)/../ac83xx/osd/hal/*.c)) ../ac83xx/osd/osd_misc.o
dal_files = $(patsubst $(srctree)/$(src)/dal/%.c,dal/%.o,$(wildcard $(srctree)/$(src)/dal/*.c))
osd_files = $(patsubst $(srctree)/$(src)/osd/hal/%.c,osd/hal/%.o,$(wildcard $(srctree)/$(src)/osd/hal/*.c)) osd/osd_misc.o

dal-y :=$(dal_files) $(osd_files) dal-main.o
