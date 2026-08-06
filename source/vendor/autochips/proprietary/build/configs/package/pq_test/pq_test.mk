################################################################################
#
# atc pq test
#
################################################################################

PQ_TEST_VERSION = 1.0
PQ_TEST_SITE = $(TOPDIR)/../source/packages/samplecode/native-sample/graphics/test/pq_test
PQ_TEST_SITE_METHOD = local
PQ_TEST_ALWAYS_BUILD = YES
PQ_TEST_DEPENDENCIES += libsettings_atc

PQ_TEST_MAKE_OPTS = \
	ATC_CLUSTER_SUPPORT=$(ATC_CLUSTER_SUPPORT)

define PQ_TEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(PQ_TEST_MAKE_OPTS)
endef

define PQ_TEST_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/pq_test $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

