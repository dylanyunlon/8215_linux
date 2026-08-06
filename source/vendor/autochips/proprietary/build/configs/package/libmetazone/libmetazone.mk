################################################################################
#
# libmetazone
#
################################################################################

LIBMETAZONE_SITE = $(TOPDIR)/../source/packages/metazone/libmetazone
LIBMETAZONE_SITE_METHOD = local
LIBMETAZONE_ALWAYS_BUILD = YES
LIBMETAZONE_INSTALL_STAGING = YES
#LIBMETAZONE_DEPENDENCIES += metazone

#LIBMETAZONE_MAKE_ARGS += STATIC_LIB=

define LIBMETAZONE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define LIBMETAZONE_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0755 -D $(@D)/libmetazone.so $(STAGING_DIR)/usr/lib
	$(INSTALL) -m 0644 -D $(LIBMETAZONE_SITE)/metazone.h $(STAGING_DIR)/usr/include
endef

define LIBMETAZONE_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/libmetazone.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))
