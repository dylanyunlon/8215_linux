ifneq ($(AC83XX_BOOT_DEVICE_SIZE),128)
ATCMEDIASCANNER_VERSION = 1.0
ATCMEDIASCANNER_CODE_PATH = $(TOPDIR)/../source/packages/multimedia/libatcmediascanner
ATCMEDIASCANNER_PREBUILD_PATH = $(TOPDIR)/../prebuild/multimedia/libatcmediascanner
ATCMEDIASCANNER_SITE = $(shell if [ -d $(ATCMEDIASCANNER_CODE_PATH) ]; then echo $(ATCMEDIASCANNER_CODE_PATH); else echo $(ATCMEDIASCANNER_PREBUILD_PATH); fi)
ATCMEDIASCANNER_SITE_METHOD = local
ATCMEDIASCANNER_ALWAYS_BUILD = YES
ATCMEDIASCANNER_INSTALL_STAGING = YES
ATCMEDIASCANNER_DEPENDENCIES += mmisc atcmultimedia

ATCMEDIASCANNER_MAKE_ARGS += STATIC_LIB=

ATCMEDIASCANNER_MAKE_OPTS = \
	TOPDIR=$(TOPDIR)/.. KERNEL_TOPDIR=$(TOPDIR)/../source/kernel/kernel-3.18 \
	CC=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-g++ \
	SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot

define ATCMEDIASCANNER_BUILD_CMDS
	@if [ -d $(ATCMEDIASCANNER_CODE_PATH) ]; then \
		echo "build atcmediascanner"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(ATCMEDIASCANNER_MAKE_OPTS) all; \
	else \
		echo "atcmediascanner Prebuild"; \
	fi
endef

define ATCMEDIASCANNER_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libatcmediascanner.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/include/* $(STAGING_DIR)/usr/include
endef

define ATCMEDIASCANNER_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libatcmediascanner.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

else
    $(warning atcmediascanner is disabled beacause AC83XX_BOOT_DEVICE_SIZE is 128)
endif