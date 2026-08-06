################################################################################
#
# preloader
#
################################################################################

BOOTLOADER_SRC = $(TOPDIR)/../bsp/bootloader
LIB_SRC = $(TOPDIR)/../bsp/lib/
BOOTLOADER_TARGET = $(@D)/uboot/u-boot.bin
BOOTLOADER_IMG = $(BINARIES_DIR)/u-boot.bin

BOOTLOADER_VERSION = 
ifeq ($(BOOTLOADER_SRC), $(wildcard $(BOOTLOADER_SRC)))
BOOTLOADER_SITE = $(BOOTLOADER_SRC)
BOOTLOADER_SITE_METHOD = local
endif

#BOOTLOADER_LICENSE = GPL-2.0
#BOOTLOADER_LICENSE_FILES = COPYING
#BOOTLOADER_DEPENDENCIES =

BOOTLOADER_INSTALL_IMAGES = YES

BOOTLOADER_MAKE_OPTS = 

define BOOTLOADER_BUILD_CMDS
	test ! -d $(@D)/../lib || rm -rf $(@D)/../lib
	test ! -d $(LIB_SRC) || cp -arf $(LIB_SRC) $(@D)/../
	test ! -d $(BOOTLOADER_SRC) || $(MAKE) -C $(@D)/uboot $(BOOTLOADER_MAKE_OPTS)
endef

define BOOTLOADER_INSTALL_IMAGES_CMDS
	test ! -e $(BOOTLOADER_TARGET) || cp -f $(BOOTLOADER_TARGET) $(BOOTLOADER_IMG)
endef

$(eval $(generic-package))
