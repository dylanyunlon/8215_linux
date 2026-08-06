################################################################################
#
# iap
#
################################################################################
PKGDIR := $(pkgdir)
IAP_VERSION = 1.0
IAP_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/carlink/iap
IAP_PREBUILD_PATH = $(TOPDIR)/../prebuild/carlink/iap
IAP_SITE = $(shell if [ -d $(IAP_CODE_PATH) ]; then echo $(IAP_CODE_PATH); else echo $(IAP_PREBUILD_PATH); fi)
IAP_SITE_METHOD = local
IAP_ALWAYS_BUILD = YES
IAP_INSTALL_STAGING = YES
IAP_DEPENDENCIES = carlinkconfigs carlinkutils

IAP_MAKE_ARGS += \
    LIB_DIR=$(IAP_DIR) \
    SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
    RECURSION_MAK=$(TOPDIR)/../build/mak/recursion_build.mak

define IAP_BUILD_CMDS
	@if [ -d $(IAP_CODE_PATH) ]; then \
		echo "build iap"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(IAP_MAKE_ARGS) all; \
	else \
		echo "prebuild iap"; \
	fi
endef

define IAP_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0644 -D $(@D)/client/libiapclient.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/service/libiap/libiapimpl.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/service/server/iap $(STAGING_DIR)/usr/bin
endef

define IAP_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libiapclient.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(STAGING_DIR)/usr/lib/libiapimpl.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/bin/iap $(TARGET_DIR)/usr/bin

    $(INSTALL) -m 0755 -D $($(PKG)_PKGDIR)/iap.sh $(TARGET_DIR)/usr/bin/iap.sh
endef

$(eval $(generic-package))

