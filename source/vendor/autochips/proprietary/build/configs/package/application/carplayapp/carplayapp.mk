################################################################################
#
# app carplayapp so
#
################################################################################

CARPLAYAPP_VERSION = 1.0
CARPLAYAPP_SITE = $(TOPDIR)/../source/packages/application/carplayapp
CARPLAYAPP_SITE_METHOD = local
CARPLAYAPP_DEPENDENCIES = universal_utils apputils globalbus appcommon qt5declarative carplaymanager
CARPLAYAPP_ALWAYS_BUILD = YES

CARPLAYAPP_CFLAGS += -funwind-tables
CARPLAYAPP_CPPFLAGS += -funwind-tables
CARPLAYAPP_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
CARPLAYAPP_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
CARPLAYAPP_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
CARPLAYAPP_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
CARPLAYAPP_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define CARPLAYAPP_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define CARPLAYAPP_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define CARPLAYAPP_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m0755 $(@D)/lib/libcarplayapp.so* $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/lib/libcarplayapp.so* $(STAGING_DIR)/usr/lib
endef

$(eval $(generic-package))
