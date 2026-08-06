################################################################################
#
# camera
#
################################################################################

CAMERA_VERSION = 1.0.0
CAMERA_SITE = $(TOPDIR)/../vendor/autochips/proprietary/packages/camera
CAMERA_SITE_METHOD = local
CAMERA_ALWAYS_BUILD = YES
CAMERA_DEPENDENCIES += glibc libfastdisplay libavm  mali-t82x

define CAMERA_BUILD_CMDS
	 $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef


define CAMERA_INSTALL_TARGET_CMDS
       $(INSTALL) -m 0755 -D $(@D)/app/avm $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

