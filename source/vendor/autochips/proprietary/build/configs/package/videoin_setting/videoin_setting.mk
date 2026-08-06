################################################################################
#
# videoin_setting
#
################################################################################

VIDEOIN_SETTING_VERSION = 1.0.0
VIDEOIN_SETTING_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/videoin/libs/utils/setting
VIDEOIN_SETTING_SITE_METHOD = local
VIDEOIN_SETTING_ALWAYS_BUILD = YES
VIDEOIN_SETTING_DEPENDENCIES += videoin_common libmetazone

define VIDEOIN_SETTING_BUILD_CMDS
	 $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define VIDEOIN_SETTING_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(@D)/libvideoin_setting.so $(TARGET_DIR)/usr/lib64
endef

$(eval $(generic-package))

