################################################################################
#
# vip_service
#
################################################################################

VIP_SERVICE_VERSION = 1.0.0
VIP_SERVICE_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/videoin/services/vip
VIP_SERVICE_SITE_METHOD = local
VIP_SERVICE_ALWAYS_BUILD = YES
VIP_SERVICE_DEPENDENCIES += buf_allocator libatcdi

define VIP_SERVICE_BUILD_CMDS
	 $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define VIP_SERVICE_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(@D)/vip_service $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

