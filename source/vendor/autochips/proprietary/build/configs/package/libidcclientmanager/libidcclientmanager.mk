################################################################################
#
# libidcclientmanager
#
################################################################################
LIBIDCCLIENTMANAGER_VERSION = 1.0
LIBIDCCLIENTMANAGER_SITE = $(TOPDIR)/../source/packages/cluster/idc
LIBIDCCLIENTMANAGER_SITE_METHOD = local
LIBIDCCLIENTMANAGER_ALWAYS_BUILD = YES
LIBIDCCLIENTMANAGER_DEPENDENCIES += glibc libidc

define LIBIDCCLIENTMANAGER_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define LIBIDCCLIENTMANAGER_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0755 -D $(@STAGING_DIR)/libidcclientmanager.so $(STAGING_DIR)/usr/lib
	$(INSTALL) -m 0664 -D $(@STAGING_DIR)/*.h $(STAGING_DIR)/usr/include
	# $(INSTALL) -m 0664 -D $(@STAGING_DIR)/service/icluster.h $(STAGING_DIR)/usr/include
endef

define LIBIDCCLIENTMANAGER_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/libidcclientmanager.so $(TARGET_DIR)/usr/lib
	$(INSTALL) -m 0755 -D $(@D)/libidcclientmanager.so $(STAGING_DIR)/usr/lib
	# $(INSTALL) -m 0664 -D $(@D)/*.hpp $(STAGING_DIR)/usr/include
endef

$(eval $(generic-package))