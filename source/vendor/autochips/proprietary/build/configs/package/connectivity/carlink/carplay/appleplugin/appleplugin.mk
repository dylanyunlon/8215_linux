################################################################################
#
# AppleCarplay_CommunicationPlugin
#
################################################################################
APPLEPLUGIN_VERSION = 1.0
APPLEPLUGIN_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/carlink/external/AppleCarplay_CommunicationPlugin
APPLEPLUGIN_PREBUILD_PATH = $(TOPDIR)/../prebuild/carlink/appleplugin
APPLEPLUGIN_SITE = $(shell if [ -d $(APPLEPLUGIN_CODE_PATH) ]; then echo $(APPLEPLUGIN_CODE_PATH); else echo $(APPLEPLUGIN_PREBUILD_PATH); fi)
APPLEPLUGIN_SITE_METHOD = local
APPLEPLUGIN_ALWAYS_BUILD = YES
APPLEPLUGIN_INSTALL_STAGING = YES
APPLEPLUGIN_DEPENDENCIES = carlinkconfigs mdnsresponder

APPLEPLUGIN_MAKE_ARGS := \
    SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
    RECURSION_MAK=$(TOPDIR)/../build/mak/recursion_build.mak

define APPLEPLUGIN_BUILD_CMDS
	@if [ -d $(APPLEPLUGIN_CODE_PATH) ]; then \
		echo "build appleplugin"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(APPLEPLUGIN_MAKE_ARGS) all; \
	else \
		echo "prebuild appleplugin"; \
	fi
endef

define APPLEPLUGIN_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0644 -D $(@D)/AccessorySDK/libcoreutils.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/CommSDK/libcarplayplugin.so $(STAGING_DIR)/usr/lib
endef

define APPLEPLUGIN_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libcoreutils.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libcarplayplugin.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

