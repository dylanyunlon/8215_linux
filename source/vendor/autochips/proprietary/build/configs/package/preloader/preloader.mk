################################################################################
#
# preloader
#
################################################################################

PRELOADER_SRC = $(TOPDIR)/../bsp/preloader
PRELOADER_PREBUILD_SRC = $(TOPDIR)/../prebuild/preloader
PRELOADER_TARGET_PREFIX = $(@D)/target/3363_Preloader_realchip_
PRELOADER_IMAG_NAME = preloader_$(AC83XX_BOOT_DEVICE)$(AC83XX_BOOT_DEVICE_SIZE)_ddr$(ATC_DDR_SIZE)_AB$(ATC_AB_PARTITION_SUPPORT)
PRELOADER_IMG_PREFIX = $(BINARIES_DIR)/$(PRELOADER_IMAG_NAME)_
PRELOADER_RELEASE_IMG_PREFIX = $(BINARIES_DIR)/83XX_Preloader_realchip_
LIB_SRC = $(TOPDIR)/../bsp/lib/
PREBUILSRC = $(TOPDIR)/../prebuild/preloader_hardware
PREBUILDDIR = $(TOPDIR)/../prebuild/preloader
BOOTDEVICES = sd emmc nand nor

PRELOADER_VERSION =
ifeq ($(PRELOADER_SRC), $(wildcard $(PRELOADER_SRC)))
PRELOADER_SITE = $(PRELOADER_SRC)
PRELOADER_SITE_METHOD = local
endif

#PRELOADER_LICENSE = GPL-2.0
#PRELOADER_LICENSE_FILES = COPYING
#PRELOADER_DEPENDENCIES =
bootdevice = $(AC83XX_BOOT_DEVICE)
PRELOADER_INSTALL_IMAGES = YES

ifeq ($(bootdevice), all)
PRELOADER_DEFAULT_IMG = $(PRELOADER_IMG_PREFIX)nand.bin
else
PRELOADER_TARGET = $(PRELOADER_TARGET_PREFIX)$(bootdevice).bin
PRELOADER_IMG = $(PRELOADER_IMG_PREFIX)$(bootdevice).bin
PRELOADER_DEFAULT_IMG = $(PRELOADER_IMG)
endif

PRELOADER_MAKE_OPTS = \
	target=realchip \
	bootdevice=$(bootdevice) \
	prebuilddir=$(PREBUILDDIR) \
	os=linux
PRELOADER_SD_MAKE_OPTS = \
	target=realchip \
	bootdevice=sd \
	prebuilddir=$(PREBUILDDIR) \
	os=linux

# bootdevice is all ,just for test,do not care.
define PRELOADER_BUILD_CMDS
	test ! -d $(@D)/../lib || rm -rf $(@D)/../lib
	test ! -d $(LIB_SRC) || cp -arf $(LIB_SRC) $(@D)/../
	$(PRELOADER_SRC)/tools/gen_autover.sh $(@D)/src/include/
	if [ "$(bootdevice)" = "all" ]; then \
		for dev in $(BOOTDEVICES); do \
			test ! -d $(PRELOADER_SRC) || $(MAKE) -C $(@D) target=realchip bootdevice=$$dev os=linux; \
			test ! -e $(PRELOADER_TARGET_PREFIX)$$dev.bin || cp -f $(PRELOADER_TARGET_PREFIX)$$dev.bin $(PRELOADER_IMG_PREFIX)$$dev.bin; \
			if [ "$$dev" != "$(lastword $(BOOTDEVICES))" ]; then \
				test ! -d $(PRELOADER_SRC) || $(MAKE) -C $(@D) clean; \
			fi; \
		done; \
	else \
		if echo "$(BOOTDEVICES)" | grep -wq "$(bootdevice)"; then \
			test ! -d $(PRELOADER_SRC) || $(MAKE) -C $(@D) $(PRELOADER_SD_MAKE_OPTS); \
			test ! -e $(PRELOADER_TARGET_PREFIX)sd.bin || cp -f $(PRELOADER_TARGET_PREFIX)sd.bin $(PRELOADER_IMG_PREFIX)sd.bin; \
			test ! -d $(PRELOADER_SRC) || $(MAKE) -C $(@D) clean; \
			test ! -d $(PRELOADER_SRC) || $(MAKE) -C $(@D) $(PRELOADER_MAKE_OPTS); \
			test ! -e $(PRELOADER_TARGET) || cp -f $(PRELOADER_TARGET) $(PRELOADER_IMG); \
		else \
			echo "do not support this device :$(bootdevice)"; exit -1; \
		fi; \
	fi
endef

define PRELOADER_INSTALL_IMAGES_CMDS
	test  -e $(PRELOADER_DEFAULT_IMG) || exit -1; 
	cp -arf $(BINARIES_DIR)/$(PRELOADER_IMAG_NAME)_sd.bin $(PRELOADER_RELEASE_IMG_PREFIX)sd.bin;
	cp -arf $(BINARIES_DIR)/$(PRELOADER_IMAG_NAME)_$(bootdevice).bin $(PRELOADER_RELEASE_IMG_PREFIX)$(bootdevice).bin;
endef

$(eval $(generic-package))
