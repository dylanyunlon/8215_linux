################################################################################
#
# dvr app
#
################################################################################

    DVRAPP_VERSION = 1.0
    DVRAPP_SITE = $(TOPDIR)/../source/packages/application/dvrapp
    DVRAPP_SITE_METHOD = local
    DVRAPP_ALWAYS_BUILD = YES
    DVRAPP_DEPENDENCIES += -lpthread universal_utils tinyxml2 qt5declarative libdvr appobj apputils appcommon globalbus

    DVRAPP_CFLAGS += -funwind-tables
    DVRAPP_CPPFLAGS += -funwind-tables
    DVRAPP_LDFLAGS += -rdynamic

    ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
    DVRAPP_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
    DVRAPP_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
    else
    DVRAPP_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
    DVRAPP_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
    endif

    define DVRAPP_CONFIGURE_CMDS
        (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
    endef

    define DVRAPP_BUILD_CMDS
        $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
    endef

    define DVRAPP_INSTALL_TARGET_CMDS
        $(INSTALL) -D -m0755 $(@D)/libdvrapp.so $(TARGET_DIR)/usr/lib
    endef

    $(eval $(generic-package))
