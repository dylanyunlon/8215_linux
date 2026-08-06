
ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))
ifeq "$(ATC_OS_NAME)" "android"
EXTRA_CFLAGS += -DAEE_FOR_ANDROID
endif


obj-$(CONFIG_MTK_AEE_IPANIC)	+= ipanic.o
ipanic-y	:= ipanic_log.o ipanic_rom.o

ipanic-y	+= ipanic_emmc.o

ipanic-y	+= ipanic_vfs.o
ipanic-y	+= ipanic_pstore.o
