################################################################################
#
# videoin_di
#
################################################################################

VIDEOIN_DI_VERSION = 1.0.0
VIDEOIN_DI_CODE_PATH = $(TOPDIR)/../vendor/autochips/proprietary/hardware/videoin/libs/DI
VIDEOIN_DI_PREBUILD_PATH = $(TOPDIR)/../prebuilt/DI
VIDEOIN_DI_SITE = $(shell if [ -d $(VIDEOIN_DI_CODE_PATH) ]; then echo $(VIDEOIN_DI_CODE_PATH); else echo $(VIDEOIN_DI_PREBUILD_PATH); fi)
VIDEOIN_DI_SITE_METHOD = local
VIDEOIN_DI_ALWAYS_BUILD = YES
VIDEOIN_DI_INSTALL_STAGING = YES


define VIDEOIN_DI_BUILD_CMDS
	@if [ -d $(VIDEOIN_DI_CODE_PATH) ]; then \
		echo "build videoin_di"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) ;\
	else \
		echo "libvideoin_di Prebuild"; \
	fi
endef

define VIDEOIN_DI_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libvideoin_di.so $(STAGING_DIR)/usr/lib64
    $(INSTALL) -m 0755 -D $(@D)/inc/TopBottomDetect.h $(STAGING_DIR)/usr/include/
    $(INSTALL) -m 0755 -D $(@D)/inc/TopBottomDetectLog.h $(STAGING_DIR)/usr/include/
endef

define VIDEOIN_DI_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib64/libvideoin_di.so $(TARGET_DIR)/usr/lib64
endef

$(eval $(generic-package))
