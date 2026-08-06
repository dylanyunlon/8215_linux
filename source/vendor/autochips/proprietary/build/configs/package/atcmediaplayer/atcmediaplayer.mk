ifneq ($(AC83XX_BOOT_DEVICE_SIZE),128)
ATCMEDIAPLAYER_VERSION = 1.0
ATCMEDIAPLAYER_CODE_PATH = $(TOPDIR)/../source/packages/multimedia/mediaplayerserver
ATCMEDIAPLAYER_PREBUILD_PATH = $(TOPDIR)/../prebuild/multimedia/mediaplayerserver
ATCMEDIAPLAYER_SITE = $(shell if [ -d $(ATCMEDIAPLAYER_CODE_PATH) ]; then echo $(ATCMEDIAPLAYER_CODE_PATH); else echo $(ATCMEDIAPLAYER_PREBUILD_PATH); fi)
ATCMEDIAPLAYER_SITE_METHOD = local
ATCMEDIAPLAYER_ALWAYS_BUILD = YES
ATCMEDIAPLAYER_INSTALL_STAGING = YES
ATCMEDIAPLAYER_DEPENDENCIES += mmisc atcomx libatcsurface directrender atcmultimedia

ATCMEDIAPLAYER_MAKE_ARGS += STATIC_LIB=

ATCMEDIAPLAYER_MAKE_OPTS = \
	TOPDIR=$(TOPDIR)/.. KERNEL_TOPDIR=$(TOPDIR)/../source/kernel/kernel-3.18 \
	CC=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-g++ \
	SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot

define ATCMEDIAPLAYER_BUILD_CMDS
	@if [ -d $(ATCMEDIAPLAYER_CODE_PATH) ]; then \
		echo "build atcmediaplayer"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(ATCMEDIAPLAYER_MAKE_OPTS) all; \
	else \
		echo "atcmediaplayer Prebuild"; \
	fi
endef

define ATCMEDIAPLAYER_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libatcmediaplayer.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/include/* $(STAGING_DIR)/usr/include
endef

define ATCMEDIAPLAYER_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libatcmediaplayer.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))
else
    $(warning atcmediaplayer is disabled beacause AC83XX_BOOT_DEVICE_SIZE is 128)
endif

