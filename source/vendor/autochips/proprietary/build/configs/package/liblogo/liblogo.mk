################################################################################
#
# liblogo
#
################################################################################

LIBLOGO_SITE = $(TOPDIR)/../source/packages/logo/liblogo
LIBLOGO_SITE_METHOD = local
LIBLOGO_ALWAYS_BUILD = YES

#LIBLOGO_MAKE_ARGS += STATIC_LIB=

define LIBLOGO_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define LIBLOGO_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/liblogo.so $(TARGET_DIR)/usr/lib
         $(INSTALL) -m 0644 -D $(@D)/logorw.h $(STAGING_DIR)/usr/include
endef

$(eval $(generic-package))
