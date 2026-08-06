################################################################################
#
# UPDATE
#
################################################################################

UPDATE_VERSION = 1.0
UPDATE_SITE = $(TOPDIR)/../source/packages/ab_update/update/1.0
UPDATE_SITE_METHOD = local
UPDATE_DEPENDENCIES = tinyxml2 libbzip2 libminizip
UPDATE_ALWAYS_BUILD = YES
UPDATE_INSTALL_STAGING = YES
UPDATE_INSTALL_TARGET = YES

#$(error  target cross is $(TARGET_CROSS))

UPDATE_TARGET := update.bin
BOOTCTRL_TARGET := bootctrl_tool

define UPDATE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define UPDATE_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/$(UPDATE_TARGET) $(TARGET_DIR)/usr/bin/
endef

define UPDATE_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0644 -D $(@D)/include/UpdateMessageType.hpp $(STAGING_DIR)/usr/include
	$(INSTALL) -m 0755 -D $(@D)/$(BOOTCTRL_TARGET) $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))
