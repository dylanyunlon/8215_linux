ATCOMX_VERSION = 1.0
ATCOMX_CODE_PATH = $(TOPDIR)/../source/packages/multimedia/omx
ATCOMX_PREBUILD_PATH = $(TOPDIR)/../prebuild/multimedia/omx
ATCOMX_SITE = $(shell if [ -d $(ATCOMX_CODE_PATH) ]; then echo $(ATCOMX_CODE_PATH); else echo $(ATCOMX_PREBUILD_PATH); fi)
ATCOMX_SITE_METHOD = local
ATCOMX_ALWAYS_BUILD = YES
ATCOMX_INSTALL_STAGING = YES
ATCOMX_DEPENDENCIES += mmisc atccodec 

ATCOMX_MAKE_ARGS += STATIC_LIB=

ATCOMX_MAKE_OPTS = \
	TOPDIR=$(TOPDIR)/.. KERNEL_TOPDIR=$(TOPDIR)/../source/kernel/kernel-3.18 \
	CC=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-g++


define ATCOMX_BUILD_CMDS
	@if [ -d $(ATCOMX_CODE_PATH) ]; then \
		echo "build atcomx"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(ATCOMX_MAKE_OPTS) all; \
	else \
		echo "atcomx Prebuild"; \
	fi
endef

define ATCOMX_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libatcomxcore.so $(STAGING_DIR)/usr/lib
	$(INSTALL) -m 0755 -D $(@D)/libatcomxvdec.so $(STAGING_DIR)/usr/lib
	$(INSTALL) $(@D)/openmax/* $(STAGING_DIR)/usr/include/
endef

define ATCOMX_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libatcomxcore.so $(TARGET_DIR)/usr/lib
	$(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libatcomxvdec.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

