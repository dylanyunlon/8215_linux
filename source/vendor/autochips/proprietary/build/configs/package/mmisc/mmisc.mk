MMISC_VERSION = 1.0
MMISC_CODE_PATH = $(TOPDIR)/../source/packages/multimedia/mmisc
MMISC_PREBUILD_PATH = $(TOPDIR)/../prebuild/multimedia/mmisc
MMISC_SITE = $(shell if [ -d $(MMISC_CODE_PATH) ]; then echo $(MMISC_CODE_PATH); else echo $(MMISC_PREBUILD_PATH); fi)
MMISC_SITE_METHOD = local
MMISC_ALWAYS_BUILD = YES
MMISC_INSTALL_STAGING = YES
#MMISC_DEPENDENCIES += libdrm 

MMISC_MAKE_ARGS += STATIC_LIB=

MMISC_MAKE_OPTS = \
	TOPDIR=$(TOPDIR)/.. KERNEL_TOPDIR=$(TOPDIR)/../source/kernel/kernel-3.18 \
	CC=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-gcc


define MMISC_BUILD_CMDS
	@if [ -d $(MMISC_CODE_PATH) ]; then \
		echo "build mmisc"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(MMISC_MAKE_OPTS) all; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/osal $(MMISC_MAKE_OPTS) all; \
	else \
		echo "mmisc Prebuild"; \
	fi
endef

define MMISC_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libatcmmisc.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/osal/libosal.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/include/mmisc_pub.h $(STAGING_DIR)/usr/include/
    $(INSTALL) -m 0755 -D $(@D)/include/mm_log.h $(STAGING_DIR)/usr/include/
    $(INSTALL) -m 0755 -D $(@D)/include/winnls.h $(STAGING_DIR)/usr/include/
    $(INSTALL) -m 0755 -D $(@D)/osal/include/* $(STAGING_DIR)/usr/include/
endef

define MMISC_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libatcmmisc.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libosal.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

