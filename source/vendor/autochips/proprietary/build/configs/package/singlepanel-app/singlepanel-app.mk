################################################################################
#
# qt singlepanel-app
#
################################################################################

SINGLEPANEL_APP_VERSION = $(QT5_VERSION)
SINGLEPANEL_APP_SITE = $(TOPDIR)/../cluster/singlepanel-app
SINGLEPANEL_APP_SITE_METHOD = local
SINGLEPANEL_APP_DEPENDENCIES = qt5base
SINGLEPANEL_APP_ALWAYS_BUILD = YES

SINGLEPANEL_APP_CFLAGS += -funwind-tables
SINGLEPANEL_APP_CPPFLAGS += -funwind-tables
SINGLEPANEL_APP_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
SINGLEPANEL_APP_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
SINGLEPANEL_APP_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
SINGLEPANEL_APP_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
SINGLEPANEL_APP_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define SINGLEPANEL_APP_CONFIGURE_CMDS
	(cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define SINGLEPANEL_APP_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define SINGLEPANEL_APP_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m0755 $(@D)/singlepanel-app $(TARGET_DIR)/usr/bin
	$(INSTALL) -D -m0755 $(@D)/singlepanel-app_start.sh $(TARGET_DIR)/etc/init.d/singlepanel-app_start
endef

$(eval $(generic-package))

