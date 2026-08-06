################################################################################
#
# bt pts tool
#
################################################################################

BTPTS_TOOL_VERSION = 1.0
BTPTS_TOOL_SITE = $(TOPDIR)/../source/packages/application/btpts_tool
BTPTS_TOOL_SITE_METHOD = local
BTPTS_TOOL_ALWAYS_BUILD = YES
BTPTS_TOOL_DEPENDENCIES += -lpthread universal_utils tinyxml2 libnl bluecommon blueclient

BTPTS_TOOL_CFLAGS += -funwind-tables
BTPTS_TOOL_CPPFLAGS += -funwind-tables
BTPTS_TOOL_LDFLAGS += -rdynamic

ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
BTPTS_TOOL_LICENSE = GPL-2.0+ or LGPL-3.0, GPL-3.0 with exception(tools), GFDL-1.3 (docs)
BTPTS_TOOL_LICENSE_FILES = LICENSE.GPL2 LICENSE.GPL3 LICENSE.GPL3-EXCEPT LICENSE.LGPL3 LICENSE.FDL
else
BTPTS_TOOL_LICENSE = GPL-3.0 or LGPL-2.1 with exception or LGPL-3.0, GFDL-1.3 (docs)
BTPTS_TOOL_LICENSE_FILES = LICENSE.GPLv3 LICENSE.LGPLv21 LGPL_EXCEPTION.txt LICENSE.LGPLv3 LICENSE.FDL
endif

define BTPTS_TOOL_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(HOST_DIR)/bin/qmake)
endef

define BTPTS_TOOL_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define BTPTS_TOOL_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m0755 $(@D)/btpts_tool $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
