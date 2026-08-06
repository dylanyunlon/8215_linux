################################################################################
#
# app appmanager bin
#
################################################################################

APPMANAGER_VERSION = 1.0
APPMANAGER_SITE = $(TOPDIR)/../source/packages/application/appmanager/appmanager
APPMANAGER_SITE_METHOD = local
APPMANAGER_DEPENDENCIES = universal_utils apputils tinyxml2 globalbus
APPMANAGER_ALWAYS_BUILD = YES

APPMANAGER_CFLAGS += -funwind-tables
APPMANAGER_CPPFLAGS += -funwind-tables
APPMANAGER_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
APPMANAGER_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
APPMANAGER_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
APPMANAGER_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
APPMANAGER_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

ifeq ($(AC83XX_BOOT_DEVICE),emmc)
APPMANAGER_QMAKE_DEFINES = "DEFINES += APP_SUPPORT"
endif

define APPMANAGER_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake $(APPMANAGER_QMAKE_DEFINES))
endef


define APPMANAGER_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define APPMANAGER_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m0755 $(@D)/appmanager $(TARGET_DIR)/usr/bin
    $(INSTALL) -D -m0755 $(TOPDIR)/../source/packages/application/appmanager/appmanager/appobj.xml $(TARGET_DIR)/usr/bin
    $(INSTALL) -m 0755 -D $($(PKG)_PKGDIR)/appmanager.sh $(TARGET_DIR)/etc/init.d/S28appmanager
    ln -sf /etc/init.d/S28appmanager $(TARGET_DIR)/usr/bin/appmanager.sh
endef

$(eval $(generic-package))
