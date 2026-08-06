################################################################################
#
# app libappobj lib
#
################################################################################

APPOBJ_VERSION = 1.0
APPOBJ_SITE = $(TOPDIR)/../source/packages/application/appobj/appobj
APPOBJ_SITE_METHOD = local
APPOBJ_DEPENDENCIES = universal_utils apputils
APPOBJ_ALWAYS_BUILD = YES

APPOBJ_CFLAGS += -funwind-tables
APPOBJ_CPPFLAGS += -funwind-tables
APPOBJ_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
APPOBJ_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
APPOBJ_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
APPOBJ_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
APPOBJ_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define APPOBJ_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define APPOBJ_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define APPOBJ_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m0755 $(@D)/lib/libappobj.so* $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/lib/libappobj.so* $(STAGING_DIR)/usr/lib
endef

$(eval $(generic-package))
