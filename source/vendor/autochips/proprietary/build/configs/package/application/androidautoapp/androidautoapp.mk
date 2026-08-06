################################################################################
#
# app androidautoapp so
#
################################################################################
ifneq ($(ATC_AB_PARTITION_SUPPORT),true)
ifneq ($(AC83XX_BOOT_DEVICE_SIZE),128)

ANDROIDAUTOAPP_VERSION = 1.0
ANDROIDAUTOAPP_SITE = $(TOPDIR)/../source/packages/application/androidautoapp
ANDROIDAUTOAPP_SITE_METHOD = local
ANDROIDAUTOAPP_DEPENDENCIES = universal_utils apputils globalbus appcommon appobj qt5declarative androidauto
ANDROIDAUTOAPP_ALWAYS_BUILD = YES

ANDROIDAUTOAPP_CFLAGS += -funwind-tables
ANDROIDAUTOAPP_CPPFLAGS += -funwind-tables
ANDROIDAUTOAPP_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
ANDROIDAUTOAPP_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
ANDROIDAUTOAPP_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
ANDROIDAUTOAPP_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
ANDROIDAUTOAPP_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define ANDROIDAUTOAPP_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define ANDROIDAUTOAPP_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define ANDROIDAUTOAPP_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m0755 $(@D)/lib/libandroidautoapp.so* $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/lib/libandroidautoapp.so* $(STAGING_DIR)/usr/lib
endef

$(eval $(generic-package))

endif
endif