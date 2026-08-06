################################################################################
#
# bt phone
#
################################################################################

BTPHONE_VERSION = 1.0
BTPHONE_SITE = $(TOPDIR)/../source/packages/application/btphone
BTPHONE_SITE_METHOD = local
BTPHONE_ALWAYS_BUILD = YES
BTPHONE_DEPENDENCIES += -lpthread universal_utils tinyxml2 libnl qt5declarative

BTPHONE_CFLAGS += -funwind-tables
BTPHONE_CPPFLAGS += -funwind-tables
BTPHONE_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
BTPHONE_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
BTPHONE_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
BTPHONE_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
BTPHONE_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define BTPHONE_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define BTPHONE_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define BTPHONE_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m0755 $(@D)/libbtphone.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))
