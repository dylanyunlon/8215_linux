################################################################################
#
# car libcluster
#
################################################################################

LIBDISPLAYSETTING_VERSION = 1.0
LIBDISPLAYSETTING_SITE = $(TOPDIR)/../vendor/autochips/proprietary/hardware/display_setting
LIBDISPLAYSETTING_SITE_METHOD = local
LIBDISPLAYSETTING_ALWAYS_BUILD = YES
LIBDISPLAYSETTING_INSTALL_STAGING = YES
LIBDISPLAYSETTING_DEPENDENCIES += libmetazone
LIBDISPLAYSETTING_MAKE_ARGS += STATIC_LIB=

LIBDISPLAYSETTING_MAKE_OPTS = \
	ATC_CLUSTER_SUPPORT=$(ATC_CLUSTER_SUPPORT) \
	KENREL_HEADER_PATH=$(TOPDIR)/../kernel/kernel-4.9 \
	METAZONE_HEADER_PATH=$(TOPDIR)/../vendor/autochips/proprietary

define LIBDISPLAYSETTING_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(LIBDISPLAYSETTING_MAKE_OPTS)
endef

define LIBDISPLAYSETTING_INSTALL_TARGET_CMDS
         $(INSTALL) -m 0755 -D $(@D)/libdisplaysetting.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

