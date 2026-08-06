################################################################################
#
# cluster-service
#
################################################################################

CLUSTER_SERVICE_VERSION = 1.0
CLUSTER_SERVICE_SITE = $(TOPDIR)/../source/packages/cluster/cluster-service
CLUSTER_SERVICE_SITE_METHOD = local
CLUSTER_SERVICE_ALWAYS_BUILD = YES
CLUSTER_SERVICE_DEPENDENCIES +=universal_utils glibc libatcsurface libidc

define CLUSTER_SERVICE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define CLUSTER_SERVICE_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@STAGING_DIR)/libcluster-service.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0664 -D $(@STAGING_DIR)/utils/*.h $(STAGING_DIR)/usr/include
	$(INSTALL) -m 0664 -D $(@STAGING_DIR)/service/icluster.h $(STAGING_DIR)/usr/include
endef

define CLUSTER_SERVICE_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/libcluster-service.so $(TARGET_DIR)/usr/lib
	$(INSTALL) -m 0755 -D $(@D)/libcluster-service.so $(STAGING_DIR)/usr/lib
	$(INSTALL) -m 0664 -D $(@D)/utils/*.h $(STAGING_DIR)/usr/include
	$(INSTALL) -m 0664 -D $(@D)/service/icluster.h $(STAGING_DIR)/usr/include
endef

$(eval $(generic-package))

