DIRECTRENDER_VERSION = 1.0
DIRECTRENDER_CODE_PATH = $(TOPDIR)/../source/packages/multimedia/directrender
DIRECTRENDER_PREBUILD_PATH = $(TOPDIR)/../prebuilt/multimedia
DIRECTRENDER_SITE = $(shell if [ -d $(DIRECTRENDER_CODE_PATH) ]; then echo $(DIRECTRENDER_CODE_PATH); else echo $(DIRECTRENDER_PREBUILD_PATH); fi)
DIRECTRENDER_SITE_METHOD = local
DIRECTRENDER_ALWAYS_BUILD = YES
DIRECTRENDER_INSTALL_STAGING = YES
DIRECTRENDER_DEPENDENCIES += mmisc atcomx libatcsurface

DIRECTRENDER_MAKE_ARGS += STATIC_LIB=

DIRECTRENDER_MAKE_OPTS = \
	TOPDIR=$(TOPDIR)/.. KERNEL_TOPDIR=$(TOPDIR)/../source/kernel/kernel-3.18 \
	CC=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-g++ \
	SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot

define DIRECTRENDER_BUILD_CMDS
	@if [ -d $(DIRECTRENDER_CODE_PATH) ]; then \
		echo "build directrender"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(DIRECTRENDER_MAKE_OPTS) all; \
	else \
		echo "directrender Prebuild"; \
	fi
endef

define DIRECTRENDER_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libdirectrender.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/include/* $(STAGING_DIR)/usr/include/
    $(INSTALL) -m 0755 -D $(@D)/src/utils/async_queue.h $(STAGING_DIR)/usr/include/
endef

define DIRECTRENDER_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libdirectrender.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

