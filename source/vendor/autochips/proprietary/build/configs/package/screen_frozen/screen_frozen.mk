################################################################################
#
# screen frozen config
#
################################################################################

SCREEN_FROZEN_VERSION = 1.0
SCREEN_FROZEN_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/samplecode/display/safety_display/screenfrozenconfig
SCREEN_FROZEN_SITE_METHOD = local
SCREEN_FROZEN_ALWAYS_BUILD = YES
SCREEN_FROZEN_INSTALL_STAGING = YES

SCREEN_FROZEN_MAKE_OPTS = \
	ATC_CLUSTER_SUPPORT=$(ATC_CLUSTER_SUPPORT)

define SCREEN_FROZEN_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(SCREEN_FROZEN_MAKE_OPTS)
endef

define SCREEN_FROZEN_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/screenfrozenconfig $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

