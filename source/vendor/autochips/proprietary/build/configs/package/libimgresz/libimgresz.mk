################################################################################
#
# car libimgresz
#
################################################################################

LIBIMGRESZ_VERSION = 1.0
LIBIMGRESZ_CODE_PATH = $(TOPDIR)/../source/packages/graphics/libimgresz
LIBIMGRESZ_PREBUILD_PATH = $(TOPDIR)/../prebuild/imgresz
LIBIMGRESZ_SITE = $(shell if [ -d $(LIBIMGRESZ_CODE_PATH) ]; then echo $(LIBIMGRESZ_CODE_PATH); else echo $(LIBIMGRESZ_PREBUILD_PATH); fi)
LIBIMGRESZ_SITE_METHOD = local
LIBIMGRESZ_ALWAYS_BUILD = YES
LIBIMGRESZ_INSTALL_STAGING = YES


LIBIMGRESZ_MAKE_ARGS += STATIC_LIB=

LIBIMGRESZ_MAKE_OPTS = \
	KENREL_HEADER_PATH=$(TOPDIR)/../source/kernel/kernel-3.18/drivers/misc/atc/inc


define LIBIMGRESZ_BUILD_CMDS
	@if [ -d $(LIBIMGRESZ_CODE_PATH) ]; then \
		echo "build libimgresz"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(LIBIMGRESZ_MAKE_OPTS) -f Makefile; \
	else \
		echo "libimgresz Prebuild"; \
	fi
endef

define LIBIMGRESZ_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libimgresz.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0755 -D $(@D)/include/imgresz.h $(STAGING_DIR)/usr/include/
endef

define LIBIMGRESZ_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libimgresz.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

