################################################################################
#
# androidauto
#
################################################################################
PKGDIR := $(pkgdir)
ANDROIDAUTO_VERSION = 1.0
ANDROIDAUTO_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/carlink/androidauto
ANDROIDAUTO_PREBUILD_PATH = $(TOPDIR)/../prebuild/carlink/androidauto
ANDROIDAUTO_SITE = $(shell if [ -d $(ANDROIDAUTO_CODE_PATH) ]; then echo $(ANDROIDAUTO_CODE_PATH); else echo $(ANDROIDAUTO_PREBUILD_PATH); fi)
ANDROIDAUTO_SITE_METHOD = local
ANDROIDAUTO_ALWAYS_BUILD = YES
ANDROIDAUTO_INSTALL_STAGING = YES
ANDROIDAUTO_DEPENDENCIES = opensslprebuilt libusb carlinkutils carlinkconfigs boost

ANDROIDAUTO_MAKE_ARGS += \
    LIB_DIR=$(ANDROIDAUTO_DIR) \
    DA_SYSROOT=$(STAGING_DIR) \
    DA_LIBDIR=$(STAGING_DIR) \
    RECURSION_MAK=$(TOPDIR)/../build/mak/recursion_build.mak \
    CARLINK_SIZE_OPTIMIZE=$(if $(CARLINK_SIZE_OPTIMIZE),$(CARLINK_SIZE_OPTIMIZE),1) \
    CARLINK_ENABLE_DEBUG=$(if $(CARLINK_ENABLE_DEBUG),$(CARLINK_ENABLE_DEBUG),0)

ANDROIDAUTO_MAKE_ENV := \
    $(TARGET_CONFIGURE_OPTS) \
    CARLINK_SIZE_OPTIMIZE=$(if $(CARLINK_SIZE_OPTIMIZE),$(CARLINK_SIZE_OPTIMIZE),1) \
    CARLINK_ENABLE_DEBUG=$(if $(CARLINK_ENABLE_DEBUG),$(CARLINK_ENABLE_DEBUG),0)

define ANDROIDAUTO_BUILD_CMDS
	@if [ -d $(ANDROIDAUTO_CODE_PATH) ]; then \
		echo "build androidauto"; \
		$(ANDROIDAUTO_MAKE_ENV) $(MAKE) -C $(@D) $(ANDROIDAUTO_MAKE_ARGS) all; \
	else \
		echo "prebuild androidauto"; \
	fi
endef

define ANDROIDAUTO_STRIP_FILE
	@if [ "$(if $(CARLINK_ENABLE_DEBUG),$(CARLINK_ENABLE_DEBUG),0)" != "1" ] && [ -f $(1) ]; then \
		$(TARGET_STRIP) --strip-unneeded $(1); \
	fi
endef

define ANDROIDAUTO_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0644 -D $(@D)/client/libandroidautoclient.so $(STAGING_DIR)/usr/lib
    $(call ANDROIDAUTO_STRIP_FILE,$(STAGING_DIR)/usr/lib/libandroidautoclient.so)
    $(INSTALL) -m 0644 -D $(@D)/common/libandroidautocommon.so $(STAGING_DIR)/usr/lib
    $(call ANDROIDAUTO_STRIP_FILE,$(STAGING_DIR)/usr/lib/libandroidautocommon.so)
    $(INSTALL) -m 0755 -D $(@D)/server/androidautoserver $(STAGING_DIR)/usr/bin
    $(call ANDROIDAUTO_STRIP_FILE,$(STAGING_DIR)/usr/bin/androidautoserver)
endef

define ANDROIDAUTO_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libandroidautoclient.so $(TARGET_DIR)/usr/lib
    $(call ANDROIDAUTO_STRIP_FILE,$(TARGET_DIR)/usr/lib/libandroidautoclient.so)
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libandroidautocommon.so $(TARGET_DIR)/usr/lib
    $(call ANDROIDAUTO_STRIP_FILE,$(TARGET_DIR)/usr/lib/libandroidautocommon.so)
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/bin/androidautoserver $(TARGET_DIR)/usr/bin
    $(call ANDROIDAUTO_STRIP_FILE,$(TARGET_DIR)/usr/bin/androidautoserver)

    $(INSTALL) -m 0755 -D $($(PKG)_PKGDIR)/androidauto.sh $(TARGET_DIR)/usr/bin/androidauto.sh
endef

$(eval $(generic-package))
