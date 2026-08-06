################################################################################
#
# atc egl test
#
################################################################################

EGL_TEST_VERSION = 1.0
EGL_TEST_SITE = $(TOPDIR)/../source/packages/samplecode/native-sample/graphics/test/egl_test
EGL_TEST_SITE_METHOD = local
EGL_TEST_ALWAYS_BUILD = YES
EGL_TEST_DEPENDENCIES += mali-t400

EGL_TEST_MAKE_OPTS = \
	ATC_CLUSTER_SUPPORT=$(ATC_CLUSTER_SUPPORT)

define EGL_TEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(EGL_TEST_MAKE_OPTS)
endef

define EGL_TEST_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/egl_test $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

