################################################################################
#
# mdnsresponder
#
################################################################################
MDNSRESPONDER_VERSION = 1.0
MDNSRESPONDER_CODE_PATH = $(TOPDIR)/../source/packages/connectivity/carlink/external/mdnsresponder
MDNSRESPONDER_SITE = $(shell echo $(MDNSRESPONDER_CODE_PATH))
MDNSRESPONDER_SITE_METHOD = local
MDNSRESPONDER_ALWAYS_BUILD = YES
MDNSRESPONDER_INSTALL_STAGING = YES

MDNSRESPONDER_MAKE_ARGS := \
    SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
    RECURSION_MAK=$(TOPDIR)/../build/mak/recursion_build.mak

define MDNSRESPONDER_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(MDNSRESPONDER_MAKE_ARGS) all;
endef

define MDNSRESPONDER_INSTALL_STAGING_CMDS
    $(INSTALL) -m 0444 -D $(@D)/mDNSShared/libmdnssd.so $(STAGING_DIR)/usr/lib
    $(INSTALL) -m 0555 -D $(@D)/mDNSCore/mdnsd $(STAGING_DIR)/usr/bin
endef

define MDNSRESPONDER_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0444 -D $(STAGING_DIR)/usr/lib/libmdnssd.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0555 -D $(STAGING_DIR)/usr/bin/mdnsd $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
