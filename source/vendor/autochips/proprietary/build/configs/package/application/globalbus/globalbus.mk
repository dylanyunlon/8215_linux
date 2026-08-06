################################################################################
#
# app libglobalbus lib
#
################################################################################

GLOBALBUS_VERSION = 1.0
GLOBALBUS_SITE = $(TOPDIR)/../source/packages/application/globalbus/globalbus
GLOBALBUS_SITE_METHOD = local
GLOBALBUS_DEPENDENCIES = universal_utils apputils
GLOBALBUS_ALWAYS_BUILD = YES

GLOBALBUS_CFLAGS += -funwind-tables
GLOBALBUS_CPPFLAGS += -funwind-tables
GLOBALBUS_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
GLOBALBUS_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
GLOBALBUS_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
GLOBALBUS_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
GLOBALBUS_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define GLOBALBUS_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define GLOBALBUS_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define GLOBALBUS_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m0755 $(@D)/lib/libglobalbus.so* $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/lib/libglobalbus.so* $(STAGING_DIR)/usr/lib
endef

$(eval $(generic-package))
