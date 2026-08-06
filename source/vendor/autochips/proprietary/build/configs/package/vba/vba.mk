VBA_VERSION = 1.0
VBA_CODE_PATH = $(TOPDIR)/../source/packages/multimedia/vba
VBA_PREBUILD_PATH = $(TOPDIR)/../prebuild/multimedia/vba
VBA_SITE = $(shell if [ -d $(VBA_CODE_PATH) ]; then echo $(VBA_CODE_PATH); else echo $(VBA_PREBUILD_PATH); fi)
VBA_SITE_METHOD = local
VBA_ALWAYS_BUILD = YES
VBA_INSTALL_STAGING = YES
VBA_DEPENDENCIES += mmisc atccodec libatcsurface libsettings_atc libimgresz

VBA_MAKE_ARGS += STATIC_LIB=

VBA_MAKE_OPTS = \
	TOPDIR=$(TOPDIR)/.. KERNEL_TOPDIR=$(TOPDIR)/../source/kernel/kernel-3.18 \
	SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
	CC=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-gcc


define VBA_BUILD_CMDS
	@if [ -d $(VBA_CODE_PATH) ]; then \
		echo "build vba"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(VBA_MAKE_OPTS) all; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/daemon $(VBA_MAKE_OPTS) all; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/aniexit $(VBA_MAKE_OPTS) all; \
	else \
		echo "vba Prebuild"; \
	fi
endef

define VBA_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libbootaniplayctrl.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/libatcbootanicom.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/videobootanimation $(STAGING_DIR)/usr/bin
    $(INSTALL) -m 0755 -D $(@D)/include/* $(STAGING_DIR)/usr/include/
endef

define VBA_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libbootaniplayctrl.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libatcbootanicom.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/bin/videobootanimation $(TARGET_DIR)/usr/bin
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/include/atcbootanicom_c.h $(TARGET_DIR)/usr/include
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/include/atcbootanicom.h $(TARGET_DIR)/usr/include
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/include/atcbootanidefs.h $(TARGET_DIR)/usr/include
endef

$(eval $(generic-package))

