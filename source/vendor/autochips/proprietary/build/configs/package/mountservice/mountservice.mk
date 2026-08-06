################################################################################
#
# MountService
#
################################################################################

MOUNTSERVICE_VERSION = 1.0.0
MOUNTSERVICE_SITE = $(TOPDIR)/../source/packages/mountservice
MOUNTSERVICE_SITE_METHOD = local
MOUNTSERVICE_ALWAYS_BUILD = YES
MOUNTSERVICE_DEPENDENCIES += glibc

define MOUNTSERVICE_BUILD_CMDS
	 $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define MOUNTSERVICE_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(@D)/mountservice $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

