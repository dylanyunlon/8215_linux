################################################################################
#
# storage_utils
#
################################################################################

STORAGE_UTILS_VERSION = 1.0.0
STORAGE_UTILS_SITE = $(TOPDIR)/../source/packages/mountservice/storage_utils
STORAGE_UTILS_SITE_METHOD = local
STORAGE_UTILS_ALWAYS_BUILD = YES
STORAGE_UTILS_DEPENDENCIES += glibc

define STORAGE_UTILS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define STORAGE_UTILS_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(@D)/storage_utils $(TARGET_DIR)/usr/bin
       $(INSTALL) -m 0755 -D $(@D)/libstorage_utils.so $(TARGET_DIR)/usr/lib
       $(INSTALL) -m 0755 -D $(@D)/include/storage_utils.h $(TARGET_DIR)/usr/include
endef

$(eval $(generic-package))

