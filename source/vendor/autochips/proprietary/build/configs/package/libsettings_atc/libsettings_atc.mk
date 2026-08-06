################################################################################
#
# car libsettings_atc
#
################################################################################

LIBSETTINGS_ATC_VERSION = 1.0
LIBSETTINGS_ATC_CODE_PATH = $(TOPDIR)/../source/packages/graphics/libsettings_atc
LIBSETTINGS_ATC_PREBUILD_PATH = $(TOPDIR)/../prebuild/libsettings_atc
LIBSETTINGS_ATC_SITE = $(shell if [ -d $(LIBSETTINGS_ATC_CODE_PATH) ]; then echo $(LIBSETTINGS_ATC_CODE_PATH); else echo $(LIBSETTINGS_ATC_PREBUILD_PATH); fi)
LIBSETTINGS_ATC_SITE_METHOD = local
LIBSETTINGS_ATC_ALWAYS_BUILD = YES
LIBSETTINGS_ATC_INSTALL_STAGING = YES

LIBSETTINGS_ATC_MAKE_ARGS += STATIC_LIB=

LIBSETTINGS_ATC_MAKE_OPTS = \
	KENREL_HEADER_PATH=$(TOPDIR)/../source/kernel/kernel-3.18


define LIBSETTINGS_ATC_BUILD_CMDS
	@if [ -d $(LIBSETTINGS_ATC_CODE_PATH) ]; then \
		echo "build libsettings_atc"; \
		$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(LIBSETTINGS_ATC_MAKE_OPTS) -f Makefile; \
	else \
		echo "libsettings_atc Prebuild"; \
	fi
endef

define LIBSETTINGS_ATC_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0755 -D $(@D)/libsettings_atc.so $(STAGING_DIR)/usr/lib
	$(INSTALL) -m 0755 -D $(@D)/include/AtcDisplaySettings.h $(STAGING_DIR)/usr/include/
	$(INSTALL) -m 0755 -D $(@D)/include/BootAnimationDrv.h $(STAGING_DIR)/usr/include/
	$(INSTALL) -m 0755 -D $(@D)/include/DualarmDriver.h $(STAGING_DIR)/usr/include/
endef

define LIBSETTINGS_ATC_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libsettings_atc.so $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))

