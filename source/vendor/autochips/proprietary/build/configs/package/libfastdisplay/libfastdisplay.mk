################################################################################
#
# car libcluster
#
################################################################################

LIBFASTDISPLAY_VERSION = 1.0
LIBFASTDISPLAY_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/fastdisplay/lib
LIBFASTDISPLAY_SITE_METHOD = local
LIBFASTDISPLAY_ALWAYS_BUILD = YES
LIBFASTDISPLAY_INSTALL_STAGING = YES
LIBFASTDISPLAY_DEPENDENCIES += libion

LIBFASTDISPLAY_MAKE_ARGS += STATIC_LIB=

define LIBFASTDISPLAY_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define LIBFASTDISPLAY_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/libfastdisplay.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

