################################################################################
#
# atc lvds test
#
################################################################################

LVDS_TEST_VERSION = 1.0
LVDS_TEST_SITE = $(TOPDIR)/../source/packages/samplecode/native-sample/graphics/test/lvds_test
LVDS_TEST_SITE_METHOD = local
LVDS_TEST_ALWAYS_BUILD = YES
LVDS_TEST_DEPENDENCIES += libsettings_atc

LVDS_TEST_MAKE_OPTS = \
	ATC_CLUSTER_SUPPORT=$(ATC_CLUSTER_SUPPORT)

define LVDS_TEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(LVDS_TEST_MAKE_OPTS)
endef

define LVDS_TEST_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/lvds_test $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

