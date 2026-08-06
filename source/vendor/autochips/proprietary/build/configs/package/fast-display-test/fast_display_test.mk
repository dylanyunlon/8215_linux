################################################################################
#
# fast display test 
#
################################################################################

FAST_DISPLAY_TEST_VERSION = 1.0.0
FAST_DISPLAY_TEST_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/fastdisplay/test
FAST_DISPLAY_TEST_SITE_METHOD = local
FAST_DISPLAY_TEST_ALWAYS_BUILD = YES
FAST_DISPLAY_TEST_DEPENDENCIES += libfastdisplay glibc libion

define FAST_DISPLAY_TEST_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

#ifeq ($(BR2_PACKAGE_READLINE),y)
#FAST_DISPLAY_TEST_DEPENDENCIES = readline
#else
#FAST_DISPLAY_TEST_CONF_OPTS = --without-readline
#endif

define FAST_DISPLAY_TEST_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(@D)/fast_display_test $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

