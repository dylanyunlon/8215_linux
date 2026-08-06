################################################################################
#
# car libcluster
#
################################################################################

DISPLAYCTRL_VERSION = 1.0
DISPLAYCTRL_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/display_setting/test
DISPLAYCTRL_SITE_METHOD = local
DISPLAYCTRL_ALWAYS_BUILD = YES
DISPLAYCTRL_INSTALL_STAGING = YES
DISPLAYCTRL_DEPENDENCIES += libdisplaysetting glibc libmetazone
DISPLAYCTRL_MAKE_ARGS += STATIC_LIB=

DISPLAYCTRL_MAKE_OPTS = \
	DISPLAY_HEADER_PATH=$(TOPDIR)/../vendor/autochips/proprietary/hardware/display_setting

define DISPLAYCTRL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(DISPLAYCTRL_MAKE_OPTS)
endef

define DISPLAYCTRL_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/displayctrl $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))

