################################################################################
#
# camerasource
#
################################################################################

CAMERASOURCE_VERSION = 1.0.0
CAMERASOURCE_CODE_PATH = $(TOPDIR)/../vendor/autochips/proprietary/hardware/videoin/libs/camerasource
CAMERASOURCE_PREBUILD_PATH = $(TOPDIR)/../prebuilt/camerasource
CAMERASOURCE_SITE = $(shell if [ -d $(CAMERASOURCE_CODE_PATH) ]; then echo $(CAMERASOURCE_CODE_PATH); else echo $(CAMERASOURCE_PREBUILD_PATH); fi)
CAMERASOURCE_SITE_METHOD = local
CAMERASOURCE_ALWAYS_BUILD = YES
CAMERASOURCE_INSTALL_STAGING = YES
CAMERASOURCE_DEPENDENCIES += videoin_v4l2 tinyxml2 buf_allocator videoin_common

define CAMERASOURCE_BUILD_CMDS
	@if [ -d $(CAMERASOURCE_CODE_PATH) ]; then \
		echo "build libcamera.source"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) ;\
	else \
		echo "libcamera.source Prebuild"; \
	fi
endef

define CAMERASOURCE_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libcamerasource.so $(STAGING_DIR)/usr/lib64
endef

define CAMERASOURCE_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libcamerasource.so $(TARGET_DIR)/usr/lib64
endef

$(eval $(generic-package))

