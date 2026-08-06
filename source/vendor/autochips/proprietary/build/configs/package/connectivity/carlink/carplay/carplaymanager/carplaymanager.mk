################################################################################
#
# carplaymanagerservice
#
################################################################################
PKGDIR := $(pkgdir)
CARPLAYMANAGER_VERSION = 1.0
CARPLAYMANAGER_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/carlink/carplaymanager
CARPLAYMANAGER_PREBUILD_PATH = $(TOPDIR)/../prebuild/carlink/carplaymanager
CARPLAYMANAGER_SITE = $(shell if [ -d $(CARPLAYMANAGER_CODE_PATH) ]; then echo $(CARPLAYMANAGER_CODE_PATH); else echo $(CARPLAYMANAGER_PREBUILD_PATH); fi)
CARPLAYMANAGER_SITE_METHOD = local
CARPLAYMANAGER_ALWAYS_BUILD = YES
CARPLAYMANAGER_INSTALL_STAGING = YES
CARPLAYMANAGER_DEPENDENCIES = carplayservice

CARPLAYMANAGER_MAKE_ARGS += \
    LIB_DIR=$(CARPLAYMANAGER_DIR) \
    SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
    RECURSION_MAK=$(TOPDIR)/../build/mak/recursion_build.mak

define CARPLAYMANAGER_BUILD_CMDS
	@if [ -d $(CARPLAYMANAGER_CODE_PATH) ]; then \
		echo "build carplaymanagerservice"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(CARPLAYMANAGER_MAKE_ARGS) all; \
	else \
		echo "prebuild carplaymanagerservice"; \
	fi
endef

define CARPLAYMANAGER_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/carplaymanagerservice $(STAGING_DIR)/usr/bin
endef

define CARPLAYMANAGER_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/bin/carplaymanagerservice $(TARGET_DIR)/usr/bin

    $(INSTALL) -m 0755 -D $($(PKG)_PKGDIR)/carplaymanager.sh $(TARGET_DIR)/usr/bin/carplaymanager.sh
endef

$(eval $(generic-package))

