################################################################################
#
# bt app
#
################################################################################

BTAPP_VERSION = 1.0
BTAPP_SITE = $(TOPDIR)/../source/packages/application/btapp
BTAPP_SITE_METHOD = local
BTAPP_ALWAYS_BUILD = YES
BTAPP_DEPENDENCIES += -lpthread universal_utils tinyxml2 libnl bluecommon blueclient qt5declarative

BTAPP_CFLAGS += -funwind-tables
BTAPP_CPPFLAGS += -funwind-tables
BTAPP_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
BTAPP_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
BTAPP_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
BTAPP_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
BTAPP_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define BTAPP_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define BTAPP_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define BTAPP_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m0755 $(@D)/libbtapp.so* $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))
