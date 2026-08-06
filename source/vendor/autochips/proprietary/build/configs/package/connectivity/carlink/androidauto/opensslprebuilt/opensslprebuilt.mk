################################################################################
#
# opensslprebuilt
#
################################################################################

OPENSSLPREBUILT_VERSION = 1.1.1g
OPENSSLPREBUILT_PREBUILT_PATH = $(TOPDIR)/../source/packages/connectivity/carlink/external/openssl_1.1.1g/prebuilt
OPENSSLPREBUILT_SITE = $(shell echo $(OPENSSLPREBUILT_PREBUILT_PATH))
OPENSSLPREBUILT_SITE_METHOD = local
OPENSSLPREBUILT_INSTALL_STAGING = YES
OPENSSLPREBUILT_INSTALL_TARGET = YES
OPENSSLPREBUILT_DEPENDENCIES =

# Prebuilt library - no configuration needed
define OPENSSLPREBUILT_CONFIGURE_CMDS
    echo "Using prebuilt OpenSSL library"
endef

# Prebuilt library - no build needed
define OPENSSLPREBUILT_BUILD_CMDS
    echo "Using prebuilt OpenSSL library - skip build"
endef

define OPENSSLPREBUILT_INSTALL_STAGING_CMDS
    $(INSTALL) -d $(STAGING_DIR)/usr/lib
    $(INSTALL) -d $(STAGING_DIR)/usr/include
    
    # Copy prebuilt libraries directly to /usr/lib
    cp -dpfr $(@D)/lib/libcrypto_1_1_1g.so $(STAGING_DIR)/usr/lib/
    cp -dpfr $(@D)/lib/libssl_1_1_1g.so $(STAGING_DIR)/usr/lib/
    
    # Copy headers
    cp -dpfr $(@D)/include/openssl-1.1.1g $(STAGING_DIR)/usr/include/
endef

define OPENSSLPREBUILT_INSTALL_TARGET_CMDS
    $(INSTALL) -m 0644 -D $(@D)/lib/libcrypto_1_1_1g.so $(TARGET_DIR)/usr/lib
    $(INSTALL) -m 0644 -D $(@D)/lib/libssl_1_1_1g.so $(TARGET_DIR)/usr/lib
endef



$(eval $(generic-package))
