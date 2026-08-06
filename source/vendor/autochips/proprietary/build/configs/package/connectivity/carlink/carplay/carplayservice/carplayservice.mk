################################################################################
#
# carplayservice
#
################################################################################
CARPLAYSERVICE_VERSION = 1.0
CARPLAYSERVICE_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/carlink/carplay
CARPLAYSERVICE_PREBUILD_PATH = $(TOPDIR)/../prebuild/carlink/carplay
CARPLAYSERVICE_SITE = $(shell if [ -d $(CARPLAYSERVICE_CODE_PATH) ]; then echo $(CARPLAYSERVICE_CODE_PATH); else echo $(CARPLAYSERVICE_PREBUILD_PATH); fi)
CARPLAYSERVICE_SITE_METHOD = local
CARPLAYSERVICE_ALWAYS_BUILD = YES
CARPLAYSERVICE_INSTALL_STAGING = YES
CARPLAYSERVICE_DEPENDENCIES = iap carplayplugin carlinkmanager

CARPLAYSERVICE_MAKE_ARGS += \
    LIB_DIR=$(CARPLAYSERVICE_DIR) \
    SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
    RECURSION_MAK=$(TOPDIR)/../build/mak/recursion_build.mak

define CARPLAYSERVICE_BUILD_CMDS
	@if [ -d $(CARPLAYSERVICE_CODE_PATH) ]; then \
		echo "build carplayservice"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(CARPLAYSERVICE_MAKE_ARGS) all; \
	else \
		echo "prebuild carplayservice"; \
	fi
endef

define CARPLAYSERVICE_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0644 -D $(@D)/client/libcarplayclient.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/common/libcarplaycommon.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/service/carplayservice $(STAGING_DIR)/usr/bin
endef

define CARPLAYSERVICE_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libcarplayclient.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libcarplaycommon.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/bin/carplayservice $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

