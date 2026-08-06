################################################################################
#
# car libcluster
#
################################################################################

LIBUPDATE_VERSION = 1.0
LIBUPDATE_SITE = $(TOPDIR)/../source/packages/ab_update/libupdate/1.0
LIBUPDATE_SITE_METHOD = local
LIBUPDATE_ALWAYS_BUILD = YES
LIBUPDATE_DEPENDENCIES += update

LIBUPDATE_INSTALL_STAGING = YES

UPDATE_SERVICE_DIR = $(TOPDIR)/../source/packages/ab_update/
LIBUPDATE_TARGET := libupdate.so
LIBUPDATE_MAKE_ARGS += SHARED_LIB=

define LIBUPDATE_BUILD_CMDS
         $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define LIBUPDATE_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/$(LIBUPDATE_TARGET) $(TARGET_DIR)/usr/lib
endef

define LIBUPDATE_INSTALL_STAGING_CMDS
         $(INSTALL) -m 0644 -D $(@D)/include/ATCUpdateClient.hpp $(STAGING_DIR)/usr/include
endef

$(eval $(generic-package))
