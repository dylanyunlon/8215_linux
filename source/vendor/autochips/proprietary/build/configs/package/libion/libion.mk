################################################################################
#
# car libcluster
#
################################################################################

LIBION_VERSION = 1.0
LIBION_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/fastdisplay/libion
LIBION_SITE_METHOD = local
LIBION_ALWAYS_BUILD = YES
LIBION_INSTALL_STAGING = YES

LIBION_MAKE_ARGS += STATIC_LIB=

define LIBION_BUILD_CMDS
         $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define LIBION_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/libion.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

