################################################################################
#
# car libcluster
#
################################################################################

LIBBZIP2_VERSION = 1.0
LIBBZIP2_SITE = $(TOPDIR)/../source/packages/ab_update/third_party/bzip2
LIBBZIP2_SITE_METHOD = local
LIBBZIP2_ALWAYS_BUILD = YES

LIBBZIP2_INSTALL_STAGING = YES

LIBBZIP2_TARGET := libbzip2.so
LIBBZIP2_MAKE_ARGS += SHARED_LIB=

define LIBBZIP2_BUILD_CMDS
         $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define LIBBZIP2_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/$(LIBBZIP2_TARGET) $(TARGET_DIR)/usr/lib
endef

define LIBBZIP2_INSTALL_STAGING_CMDS
         $(INSTALL) -m 0644 -D $(@D)/bzlib.h $(STAGING_DIR)/usr/include
endef

$(eval $(generic-package))
