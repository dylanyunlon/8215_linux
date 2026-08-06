################################################################################
#
# atc qt test
#
################################################################################
QT_TEST_VERSION = 1.0
QT_TEST_SITE = $(TOPDIR)/../source/packages/samplecode/native-sample/graphics/test/qt_test
QT_TEST_SITE_METHOD = local
QT_TEST_ALWAYS_BUILD = YES
QT_TEST_DEPENDENCIES += libsettings_atc

QT_TEST_MAKE_OPTS = \
	ATC_CLUSTER_SUPPORT=$(ATC_CLUSTER_SUPPORT)

define QT_TEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(QT_TEST_MAKE_OPTS)
endef

define QT_TEST_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(@D)/qt_test $(TARGET_DIR)/usr/bin
    $(INSTALL) -m 0755 -D $(@D)/basicdrawing $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

