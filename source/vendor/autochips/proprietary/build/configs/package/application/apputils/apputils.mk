################################################################################
#
# app libutils lib
#
################################################################################

APPUTILS_VERSION = 1.0
APPUTILS_SITE = $(TOPDIR)/../source/packages/application/apputils
APPUTILS_SITE_METHOD = local
APPUTILS_DEPENDENCIES = universal_utils
APPUTILS_ALWAYS_BUILD = YES

APPUTILS_CFLAGS += -funwind-tables
APPUTILS_CPPFLAGS += -funwind-tables
APPUTILS_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
APPUTILS_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
APPUTILS_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
APPUTILS_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
APPUTILS_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define APPUTILS_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define APPUTILS_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define APPUTILS_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m0755 $(@D)/lib/libapputils.so* $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/lib/libapputils.so* $(STAGING_DIR)/usr/lib
endef

$(eval $(generic-package))

