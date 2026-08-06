################################################################################
#
# libatcdi
#
################################################################################

LIBATCDI_VERSION = 1.0.0
LIBATCDI_CODE_PATH = $(TOPDIR)/../vendor/autochips/proprietary/hardware/videoin/libs/atcdi
LIBATCDI_PREBUILD_PATH = $(TOPDIR)/../prebuilt/libatcdi
LIBATCDI_SITE = $(shell if [ -d $(LIBATCDI_CODE_PATH) ]; then echo $(LIBATCDI_CODE_PATH); else echo $(LIBATCDI_PREBUILD_PATH); fi)
LIBATCDI_SITE_METHOD = local
LIBATCDI_ALWAYS_BUILD = YES
LIBATCDI_INSTALL_STAGING = YES

LIBATCDI_MAKE_OPTS = \
	ATC_CLUSTER_SUPPORT=$(ATC_CLUSTER_SUPPORT)

define LIBATCDI_BUILD_CMDS
	@if [ -d $(LIBATCDI_CODE_PATH) ]; then \
		echo "build libatcdi"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(LIBATCDI_MAKE_OPTS);\
	else \
		echo "libatcdi Prebuild"; \
	fi
endef

define LIBATCDI_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libatcdi.so $(STAGING_DIR)/usr/lib64
    $(INSTALL) -m 0755 -D $(@D)/inc/TopBottomDetect.h $(STAGING_DIR)/usr/include/
endef

define LIBATCDI_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib64/libatcdi.so $(TARGET_DIR)/usr/lib64
endef

$(eval $(generic-package))
