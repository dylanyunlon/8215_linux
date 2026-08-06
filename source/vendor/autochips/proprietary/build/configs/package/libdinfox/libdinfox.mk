################################################################################
#
# libdinfox for encrypt and decrypt private files
#
################################################################################

LIBDINFOX_VERSION = 1.0
LIBDINFOX_CODE_PATH = $(TOPDIR)/../vendor/autochips/proprietary/hardware/libdinfox
LIBDINFOX_PREBUILT_PATH = $(TOPDIR)/../prebuilt/libdinfox
LIBDINFOX_SITE=$(shell if [ -d $(LIBDINFOX_CODE_PATH) ]; then echo $(LIBDINFOX_CODE_PATH); else echo $(LIBDINFOX_PREBUILT_PATH); fi)
LIBDINFOX_SITE_METHOD = local
LIBDINFOX_INSTALL_STAGING = YES

define LIBDINFOX_BUILD_CMDS
	@if [ -d $(LIBDINFOX_CODE_PATH) ]; then \
		$(TARGET_MAKE_ENV) $(MAKE) CC=$(TARGET_CC) -C $(@D) -f Makefile.mk; \
	else \
		echo "libdinfox Prebuild"; \
	fi
endef

define LIBDINFOX_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0664 -D $(@D)/libdinfox.so $(TARGET_DIR)/usr/lib64
endef

define LIBDINFOX_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0664 -D $(@D)/libdinfox.so $(STAGING_DIR)/usr/lib64
	$(INSTALL) -m 0664 -D $(@D)/include/dinfox/dinfo.h $(STAGING_DIR)/usr/include
endef

$(eval $(generic-package))

