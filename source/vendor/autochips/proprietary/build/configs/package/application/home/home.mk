################################################################################
#
# app home so
#
################################################################################
HOME_VERSION = 1.0
HOME_SITE = $(TOPDIR)/../source/packages/application/home
HOME_SITE_METHOD = local
HOME_DEPENDENCIES = universal_utils apputils globalbus appcommon qt5declarative
HOME_ALWAYS_BUILD = YES

HOME_CFLAGS += -funwind-tables
HOME_CPPFLAGS += -funwind-tables
HOME_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
HOME_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
HOME_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
HOME_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
HOME_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define HOME_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define HOME_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define HOME_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m0755 $(@D)/lib/libhome.so* $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/lib/libhome.so* $(STAGING_DIR)/usr/lib
    $(INSTALL) -D -m0755 $(TOPDIR)/../source/packages/application/home/appListItem.xml $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
