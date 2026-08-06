################################################################################
#
# videoin_preview
#
################################################################################

VIDEOIN_PREVIEW_VERSION = 1.0.0
VIDEOIN_PREVIEW_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/videoin/libs/preview
VIDEOIN_PREVIEW_SITE_METHOD = local
VIDEOIN_PREVIEW_ALWAYS_BUILD = YES
VIDEOIN_PREVIEW_DEPENDENCIES += libatcsurface buf_allocator videoin_common camerasource

define VIDEOIN_PREVIEW_BUILD_CMDS
	 $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define VIDEOIN_PREVIEW_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/libvideoin_preview.so $(TARGET_DIR)/usr/lib64
	$(INSTALL) -m 0664 -D $(@D)/inc/atcrvc.h $(STAGING_DIR)/usr/include
endef

$(eval $(generic-package))

