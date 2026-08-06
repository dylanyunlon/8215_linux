#workaround for include mmc/card/queue.h
ccflags-y += -I$(srctree)/drivers/mmc/card
PLATFORM_NAME := $(subst ",,$(CONFIG_ATC_PLATFORM))
ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))

ifeq "$(PLATFORM_NAME)" "ac83xx"
ifeq "$(ATC_OS_NAME)" "linux"
    EXTRA_CFLAGS += -DATC_LINUX_PLATFORM
endif
endif

obj-$(CONFIG_MMC_ATC)		+= atc_msdc.o atc_msdc_dbg.o atc_msdc_misc.o atc_msdc_dump.o
obj-$(CONFIG_MSDC_ETT_SUPPORT)	+= ettagent.o
obj-$(CONFIG_SDIOAUTOK_SUPPORT)	+= atc_sdio_autok.o  atc_sdio_autok_proc.o
