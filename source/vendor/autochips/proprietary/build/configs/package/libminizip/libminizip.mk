################################################################################
#
# car libcluster
#
################################################################################

LIBMINIZIP_VERSION = 1.0
LIBMINIZIP_SITE = $(TOPDIR)/../source/packages/ab_update/third_party/minizip-ng
LIBMINIZIP_SITE_METHOD = local
LIBMINIZIP_ALWAYS_BUILD = YES

LIBMINIZIP_INSTALL_STAGING = YES

LIBMINIZIP_TARGET := libminizip.so
LIBMINIZIP_MAKE_ARGS += SHARED_LIB=

define LIBMINIZIP_BUILD_CMDS
         $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define LIBMINIZIP_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/$(LIBMINIZIP_TARGET) $(TARGET_DIR)/usr/lib
endef

define UPDATE_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0644 -D $(@D)/minizip-ng/*.h $(STAGING_DIR)/usr/include
endef


$(eval $(generic-package))