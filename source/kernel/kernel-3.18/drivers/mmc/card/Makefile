#
# Makefile for MMC/SD card drivers
#

PLATFORM_NAME := $(subst ",,$(CONFIG_ATC_PLATFORM))
ATC_OS_NAME := $(subst ",,$(CONFIG_ATC_OS))

obj-$(CONFIG_MMC_BLOCK)		+= mmc_block.o
mmc_block-objs			:= block.o queue.o
obj-$(CONFIG_MMC_TEST)		+= mmc_test.o

obj-$(CONFIG_SDIO_UART)		+= sdio_uart.o

