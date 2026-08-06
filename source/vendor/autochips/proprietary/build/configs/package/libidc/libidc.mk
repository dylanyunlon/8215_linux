################################################################################
#
# libidc
#
################################################################################

LIBIDC_VERSION = 1.0
LIBIDC_SITE = $(TOPDIR)/../source/packages/cluster/libidc
LIBIDC_SITE_METHOD = local
LIBIDC_ALWAYS_BUILD = YES
LIBIDC_DEPENDENCIES += glibc 

define LIBIDC_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define LIBIDC_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@STAGING_DIR)/libidc.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0664 -D $(@STAGING_DIR)/utils/*.h $(STAGING_DIR)/usr/include
	$(INSTALL) -m 0664 -D $(@STAGING_DIR)/service/icluster.h $(STAGING_DIR)/usr/include
endef

define LIBIDC_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/libidc.so $(TARGET_DIR)/usr/lib
	$(INSTALL) -m 0755 -D $(@D)/libidc.so $(STAGING_DIR)/usr/lib
	$(INSTALL) -m 0664 -D $(@D)/utils/*.hpp $(STAGING_DIR)/usr/include
endef

$(eval $(generic-package))

