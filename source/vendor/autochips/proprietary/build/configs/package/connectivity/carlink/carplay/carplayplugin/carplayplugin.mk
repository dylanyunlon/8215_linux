################################################################################
#
# carplayplugin
#
################################################################################
PKGDIR := $(pkgdir)
CARPLAYPLUGIN_VERSION = 1.0
CARPLAYPLUGIN_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/carlink/carplayplugin
CARPLAYPLUGIN_PREBUILD_PATH = $(TOPDIR)/../prebuild/carlink/carplayplugin
CARPLAYPLUGIN_SITE = $(shell if [ -d $(CARPLAYPLUGIN_CODE_PATH) ]; then echo $(CARPLAYPLUGIN_CODE_PATH); else echo $(CARPLAYPLUGIN_PREBUILD_PATH); fi)
CARPLAYPLUGIN_SITE_METHOD = local
CARPLAYPLUGIN_ALWAYS_BUILD = YES
CARPLAYPLUGIN_INSTALL_STAGING = YES
CARPLAYPLUGIN_DEPENDENCIES = iap appleplugin libatcsurface directrender

CARPLAYPLUGIN_MAKE_ARGS += \
    LIB_DIR=$(CARPLAYPLUGIN_DIR) \
    SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
    RECURSION_MAK=$(TOPDIR)/../build/mak/recursion_build.mak

define CARPLAYPLUGIN_BUILD_CMDS
	@if [ -d $(CARPLAYPLUGIN_CODE_PATH) ]; then \
		echo "build carplayplugin"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(CARPLAYPLUGIN_MAKE_ARGS) all; \
	else \
		echo "prebuild carplayplugin"; \
	fi
endef

define CARPLAYPLUGIN_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0644 -D $(@D)/client/libcarplaypluginclient.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/common/libcarplayplugincommon.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/server/hid/libcarplayhid.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/server/core/libcarplay_av_core.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/server/impl/libcarplay_plugin_impl.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/server/stub/audioconverter/libaudioconverter.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/server/stub/audioutils/libaudiostream.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/server/stub/screenutils/libscreenstream.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/server/service/carplaypluginservice $(STAGING_DIR)/usr/bin
endef

define CARPLAYPLUGIN_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libcarplaypluginclient.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libcarplayplugincommon.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libcarplayhid.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libcarplay_av_core.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libcarplay_plugin_impl.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libaudioconverter.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libaudiostream.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libscreenstream.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/bin/carplaypluginservice $(TARGET_DIR)/usr/bin

    $(INSTALL) -m 0755 -D $($(PKG)_PKGDIR)/carplayplugin.sh $(TARGET_DIR)/usr/bin/carplayplugin.sh
endef

$(eval $(generic-package))

