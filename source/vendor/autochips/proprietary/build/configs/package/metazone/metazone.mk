################################################################################
#
# metazone_index.h
#
################################################################################

#METAZONE_VERSION =
METAZONE_SITE = $(TOPDIR)/../source/packages/metazone/metazone
METAZONE_SITE_METHOD = local

#METAZONE_INSTALL_IMAGES = YES
#METAZONE_INSTALL_STAGING = YES

define METAZONE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define METAZONE_INSTALL_IMAGES_CMDS
	$(INSTALL) -m 0664 -D $(@D)/metazone.bin $(BINARIES_DIR)
endef

define METAZONE_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0664 -D $(@D)/metazone_index.h $(STAGING_DIR)/usr/include
endef

$(eval $(generic-package))
