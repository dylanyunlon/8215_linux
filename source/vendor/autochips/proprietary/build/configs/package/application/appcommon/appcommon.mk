###############################################################################
#
# app libappcommon lib
#
###############################################################################
APPCOMMON_VERSION = 1.0
APPCOMMON_SITE = $(TOPDIR)/../source/packages/application/appcommon
APPCOMMON_SITE_METHOD = local
APPCOMMON_DEPENDENCIES = qt5declarative
APPCOMMON_ALWAYS_BUILD = YES

APPCOMMON_CFLAGS += -funwind-tables
APPCOMMON_CPPFLAGS += -funwind-tables
APPCOMMON_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
APPCOMMON_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
APPCOMMON_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
APPCOMMON_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
APPCOMMON_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define APPCOMMON_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define APPCOMMON_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define APPCOMMON_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m0755 $(@D)/lib/libappcommon.so* $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/lib/libappcommon.so* $(STAGING_DIR)/usr/lib
endef

$(eval $(generic-package))