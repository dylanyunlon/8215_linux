ATCCODEC_VERSION = 1.0
ATCCODEC_CODE_PATH = $(TOPDIR)/../source/packages/multimedia/libatccodec
ATCCODEC_PREBUILD_PATH = $(TOPDIR)/../prebuild/multimedia/libatccodec
ATCCODEC_SITE = $(shell if [ -d $(ATCCODEC_CODE_PATH) ]; then echo $(ATCCODEC_CODE_PATH); else echo $(ATCCODEC_PREBUILD_PATH); fi)
ATCCODEC_SITE_METHOD = local
ATCCODEC_ALWAYS_BUILD = YES
ATCCODEC_INSTALL_STAGING = YES
ATCCODEC_DEPENDENCIES += mmisc 

ATCCODEC_MAKE_ARGS += STATIC_LIB=

ATCCODEC_MAKE_OPTS = \
	TOPDIR=$(TOPDIR)/.. KERNEL_TOPDIR=$(TOPDIR)/../source/kernel/kernel-3.18 \
	CC=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-gcc \
	SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot


define ATCCODEC_BUILD_CMDS
	@if [ -d $(ATCCODEC_CODE_PATH) ]; then \
		echo "build atccodec"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(ATCCODEC_MAKE_OPTS) all; \
	else \
		echo "atccodec Prebuild"; \
	fi
endef

define ATCCODEC_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libatcvideodecoder.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/VDecoder.h $(STAGING_DIR)/usr/include
endef

define ATCCODEC_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libatcvideodecoder.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/include/VDecoder.h $(TARGET_DIR)/usr/include
endef

$(eval $(generic-package))

