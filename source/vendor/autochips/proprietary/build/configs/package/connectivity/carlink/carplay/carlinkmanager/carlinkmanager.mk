################################################################################
#
# carlinkmanager
#
################################################################################
PKGDIR := $(pkgdir)
CARLINKMANAGER_VERSION = 1.0
CARLINKMANAGER_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/carlink/carlinkmanager
CARLINKMANAGER_PREBUILD_PATH = $(TOPDIR)/../prebuild/carlink/carlinkmanager
CARLINKMANAGER_SITE = $(shell if [ -d $(CARLINKMANAGER_CODE_PATH) ]; then echo $(CARLINKMANAGER_CODE_PATH); else echo $(CARLINKMANAGER_PREBUILD_PATH); fi)
CARLINKMANAGER_SITE_METHOD = local
CARLINKMANAGER_ALWAYS_BUILD = YES
CARLINKMANAGER_INSTALL_STAGING = YES
CARLINKMANAGER_DEPENDENCIES = iap libconfig

CARLINKMANAGER_MAKE_ARGS += \
    LIB_DIR=$(CARLINKMANAGER_DIR) \
    SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
    RECURSION_MAK=$(TOPDIR)/../build/mak/recursion_build.mak

define CARLINKMANAGER_BUILD_CMDS
	@if [ -d $(CARLINKMANAGER_CODE_PATH) ]; then \
		echo "build carlinkmanager"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(CARLINKMANAGER_MAKE_ARGS) all; \
	else \
		echo "prebuild carlinkmanager"; \
	fi
endef

define CARLINKMANAGER_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0644 -D $(@D)/client/libcarlinkclient.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/libusbg/libusbg.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/service/native/libcarlinkimpl.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/service/server/carlinkmanager $(STAGING_DIR)/usr/bin
endef

define CARLINKMANAGER_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libcarlinkclient.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libusbg.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libcarlinkimpl.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/bin/carlinkmanager $(TARGET_DIR)/usr/bin

    $(INSTALL) -m 0755 -D $($(PKG)_PKGDIR)/carlink.sh $(TARGET_DIR)/usr/bin/carlink.sh
endef

$(eval $(generic-package))

