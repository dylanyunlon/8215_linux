################################################################################
#
# app carbitapp so
#
################################################################################

CARBITAPP_VERSION = 1.0
CARBITAPP_SITE = $(TOPDIR)/../source/packages/application/carbitapp
CARBITAPP_SITE_METHOD = local
CARBITAPP_DEPENDENCIES = universal_utils apputils globalbus appcommon qt5declarative carlinkutils bluecommon blueclient wifi-private wifi-public libatcsurface carlinkconfigs carbit protobuf tinyxml2 zlib directrender
CARBITAPP_ALWAYS_BUILD = YES

CARBITAPP_CFLAGS += -funwind-tables
CARBITAPP_CPPFLAGS += -funwind-tables
CARBITAPP_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
CARBITAPP_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
CARBITAPP_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
CARBITAPP_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
CARBITAPP_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define CARBITAPP_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define CARBITAPP_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define CARBITAPP_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m0755 $(@D)/lib/libcarbitapp.so* $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/lib/libcarbitapp.so* $(STAGING_DIR)/usr/lib
endef

$(eval $(generic-package))
