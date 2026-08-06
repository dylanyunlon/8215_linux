################################################################################
#
# videoin_v4l2
#
################################################################################

VIDEOIN_V4L2_VERSION = 1.0.0
VIDEOIN_V4L2_CODE_PATH = $(TOPDIR)/../vendor/autochips/proprietary/hardware/videoin/libs/videoinv4l2
VIDEOIN_V4L2_PREBUILD_PATH = $(TOPDIR)/../prebuilt/videoin_v4l2
VIDEOIN_V4L2_SITE = $(shell if [ -d $(VIDEOIN_V4L2_CODE_PATH) ]; then echo $(VIDEOIN_V4L2_CODE_PATH); else echo $(VIDEOIN_V4L2_PREBUILD_PATH); fi)
VIDEOIN_V4L2_SITE_METHOD = local
VIDEOIN_V4L2_ALWAYS_BUILD = YES
VIDEOIN_V4L2_INSTALL_STAGING = YES
VIDEOIN_V4L2_DEPENDENCIES += libmetazone


define VIDEOIN_V4L2_BUILD_CMDS
	@if [ -d $(VIDEOIN_V4L2_CODE_PATH) ]; then \
		echo "build videoin_v4l2"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) ;\
	else \
		echo "libvideoin_v4l2 Prebuild"; \
	fi
endef

define VIDEOIN_V4L2_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libvideoin_v4l2.so $(STAGING_DIR)/usr/lib64
    $(INSTALL) -m 0755 -D $(@D)/inc/atccapture.h $(STAGING_DIR)/usr/include/
    $(INSTALL) -m 0755 -D $(@D)/inc/VideoinLog.h $(STAGING_DIR)/usr/include/
endef

define VIDEOIN_V4L2_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib64/libvideoin_v4l2.so $(TARGET_DIR)/usr/lib64
endef

$(eval $(generic-package))

