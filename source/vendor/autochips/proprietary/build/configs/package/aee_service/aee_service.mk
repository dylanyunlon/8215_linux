################################################################################
#
# aee service: Dump information for NE and KE.
#
################################################################################

AEE_SERVICE_VERSION = 1.0.0
AEE_SERVICE_SITE = $(TOPDIR)/../source/packages/aee_server
AEE_SERVICE_SITE_METHOD = local
AEE_SERVICE_ALWAYS_BUILD = YES
AEE_SERVICE_DEPENDENCIES += glibc

define AEE_SERVICE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define AEE_SERVICE_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(@D)/aee $(TARGET_DIR)/usr/bin
       $(INSTALL) -m 0755 -D $(@D)/aee_start.sh $(TARGET_DIR)/etc/init.d/aee_start
endef

$(eval $(generic-package))

