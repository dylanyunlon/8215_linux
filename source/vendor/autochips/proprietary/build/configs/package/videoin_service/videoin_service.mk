################################################################################
#
# videoin_service
#
################################################################################

VIDEOIN_SERVICE_VERSION = 1.0.0
VIDEOIN_SERVICE_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/videoin/services/rvc/videoin_service
VIDEOIN_SERVICE_SITE_METHOD = local
VIDEOIN_SERVICE_ALWAYS_BUILD = YES
VIDEOIN_SERVICE_DEPENDENCIES += videoin_preview camerasource

define VIDEOIN_SERVICE_BUILD_CMDS
	 $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define VIDEOIN_SERVICE_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(@D)/videoin_service $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

