ifneq ($(AC83XX_BOOT_DEVICE_SIZE),128)
ATCMULTIMEDIA_VERSION = 1.0
ATCMULTIMEDIA_CODE_PATH = $(TOPDIR)/../source/packages/multimedia/libatcmultimedia
ATCMULTIMEDIA_PREBUILD_PATH = $(TOPDIR)/../prebuild/multimedia/libatcmultimedia
ATCMULTIMEDIA_SITE = $(shell if [ -d $(ATCMULTIMEDIA_CODE_PATH) ]; then echo $(ATCMULTIMEDIA_CODE_PATH); else echo $(ATCMULTIMEDIA_PREBUILD_PATH); fi)
ATCMULTIMEDIA_SITE_METHOD = local
ATCMULTIMEDIA_ALWAYS_BUILD = YES
ATCMULTIMEDIA_INSTALL_STAGING = YES
ATCMULTIMEDIA_DEPENDENCIES += mmisc

ATCMULTIMEDIA_MAKE_ARGS += STATIC_LIB=

ATCMULTIMEDIA_MAKE_OPTS = \
	TOPDIR=$(TOPDIR)/.. KERNEL_TOPDIR=$(TOPDIR)/../source/kernel/kernel-3.18 \
	CC=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-g++ \
	SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot

define ATCMULTIMEDIA_BUILD_CMDS
	@if [ -d $(ATCMULTIMEDIA_CODE_PATH) ]; then \
		echo "build atcmultimedia"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(ATCMULTIMEDIA_MAKE_OPTS) all; \
	else \
		echo "atcmultimedia Prebuild"; \
	fi
endef

define ATCMULTIMEDIA_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libatcmultimedia.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/include/* $(STAGING_DIR)/usr/include
endef

define ATCMULTIMEDIA_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libatcmultimedia.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))
else
    $(warning atcmultimedia is disabled beacause AC83XX_BOOT_DEVICE_SIZE is 128)
endif

