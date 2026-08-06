################################################################################
#
# UNIVERSAL_UTILS
#
################################################################################

UNIVERSAL_UTILS_SITE = $(TOPDIR)/../source/packages/connectivity/universal_utils
UNIVERSAL_UTILS_SITE_METHOD = local
UNIVERSAL_UTILS_ALWAYS_BUILD = YES
UNIVERSAL_UTILS_INSTALL_STAGING = YES
UNIVERSAL_UTILS_DEPENDENCIES += -lpthread
UNIVERSAL_UTILS_MAKE_ARGS = \
    CC=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-g++ \
    SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot

define UNIVERSAL_UTILS_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(UNIVERSAL_UTILS_MAKE_ARGS) all;
endef

define UNIVERSAL_UTILS_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libuniversal_utils.so $(STAGING_DIR)/usr/lib
    rsync -au $(UNIVERSAL_UTILS_SITE)/include/ $(STAGING_DIR)/usr/include
    $(INSTALL) -m 0644 -D $(UNIVERSAL_UTILS_SITE)/sharememory/sharememory.inl $(STAGING_DIR)/usr/include
endef

define UNIVERSAL_UTILS_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libuniversal_utils.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

