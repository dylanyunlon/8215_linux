################################################################################
#
# videoin_common
#
################################################################################

VIDEOIN_COMMON_VERSION = 1.0.0
VIDEOIN_COMMON_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/videoin/libs/utils/common
VIDEOIN_COMMON_SITE_METHOD = local
VIDEOIN_COMMON_ALWAYS_BUILD = YES

define VIDEOIN_COMMON_BUILD_CMDS
	 $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define VIDEOIN_COMMON_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(@D)/libvideoin_common.so $(TARGET_DIR)/usr/lib64
endef

$(eval $(generic-package))

