################################################################################
#
# libatccrypto for encrypt and decrypt private files, and read efuse config
#
################################################################################

LIBATCCRYPTO_VERSION = 1.0
LIBATCCRYPTO_CODE_PATH = $(TOPDIR)/../vendor/autochips/proprietary/hardware/libcrypto
LIBATCCRYPTO_PREBUILT_PATH = $(TOPDIR)/../prebuilt/libcrypto
LIBATCCRYPTO_SITE=$(shell if [ -d $(LIBATCCRYPTO_CODE_PATH) ]; then echo $(LIBATCCRYPTO_CODE_PATH); else echo $(LIBATCCRYPTO_PREBUILT_PATH); fi)
LIBATCCRYPTO_SITE_METHOD = local
LIBATCCRYPTO_INSTALL_STAGING = YES

define LIBATCCRYPTO_BUILD_CMDS
	@if [ -d $(LIBATCCRYPTO_CODE_PATH) ]; then \
		$(TARGET_MAKE_ENV) $(MAKE) CC=$(TARGET_CC) -C $(@D) -f Makefile.mk; \
	else \
		echo "libatccrypto Prebuild"; \
	fi
endef

define LIBATCCRYPTO_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0664 -D $(@D)/libatccrypto.so $(TARGET_DIR)/usr/lib64
endef

define LIBATCCRYPTO_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0664 -D $(@D)/libatccrypto.so $(STAGING_DIR)/usr/lib64
	$(INSTALL) -m 0664 -D $(@D)/atccrypto.h $(STAGING_DIR)/usr/include
endef

$(eval $(generic-package))

