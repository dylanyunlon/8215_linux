################################################################################
#
# app mainapplication bin
#
################################################################################

MAINAPPLICATION_VERSION = 1.0
MAINAPPLICATION_SITE = $(TOPDIR)/../source/packages/application/mainapplication
MAINAPPLICATION_SITE_METHOD = local
MAINAPPLICATION_DEPENDENCIES = universal_utils apputils globalbus
MAINAPPLICATION_ALWAYS_BUILD = YES

MAINAPPLICATION_CFLAGS += -funwind-tables
MAINAPPLICATION_CPPFLAGS += -funwind-tables
MAINAPPLICATION_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
MAINAPPLICATION_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
MAINAPPLICATION_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
MAINAPPLICATION_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
MAINAPPLICATION_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define MAINAPPLICATION_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define MAINAPPLICATION_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define MAINAPPLICATION_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m0755 $(@D)/mainapplication $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
