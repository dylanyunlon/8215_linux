################################################################################
#
# libdvr
#
################################################################################LIBDVR_VERSION = 1.0
LIBDVR_CODE_PATH = $(TOPDIR)/../source/packages/multimedia/libdvr
LIBDVR_PREBUILD_PATH = $(TOPDIR)/../prebuild/multimedia/libdvr
LIBDVR_SITE = $(shell if [ -d $(LIBDVR_CODE_PATH) ]; then echo $(LIBDVR_CODE_PATH); else echo $(LIBDVR_PREBUILD_PATH); fi)
LIBDVR_SITE_METHOD = local
LIBDVR_ALWAYS_BUILD = YES
LIBDVR_INSTALL_STAGING = YES
LIBDVR_DEPENDENCIES += atcmultimedia atcmediascanner mmisc atccodec libatcsurface atcmediaplayer

LIBDVR_MAKE_ARGS += STATIC_LIB=

LIBDVR_MAKE_OPTS = \
	TOPDIR=$(TOPDIR)/..  \
	KERNEL_TOPDIR=$(TOPDIR)/../source/kernel/kernel-3.18 \
	SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
	CC=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-gcc


define LIBDVR_BUILD_CMDS
	@if [ -d $(LIBDVR_CODE_PATH) ]; then \
		echo "build libdvr"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(LIBDVR_MAKE_OPTS) all; \
	else \
		echo "libdvr Prebuild"; \
	fi
endef

define LIBDVR_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libdvr.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/dvr.h $(STAGING_DIR)/usr/include/
    $(INSTALL) -m 0755 -D $(@D)/dvr_types.h $(STAGING_DIR)/usr/include/
    $(INSTALL) -m 0755 -D $(@D)/dvr_mediascanner.h $(STAGING_DIR)/usr/include/

endef

define LIBDVR_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libdvr.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))
