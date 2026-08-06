################################################################################
#
# atc awtk test
#
################################################################################


AWTK_VERSION = 1.0
AWTK_SITE = $(TOPDIR)/../source/packages/cluster/awtk
AWTK_SITE_METHOD = local
AWTK_ALWAYS_BUILD = YES
AWTK_DEPENDENCIES += libsettings_atc
AWTK_DEPENDENCIES += mali-t400 vba
AWTK_DEPENDENCIES += host-scons

#AWTK_MAKE_OPTS = \
#	ATC_CLUSTER_SUPPORT=$(ATC_CLUSTER_SUPPORT)

define AWTK_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define AWTK_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/awtk_start.sh $(TARGET_DIR)/etc/init.d/S01awtk
	$(INSTALL) -m 0755 -D $(@D)/awtk-linux-fb/release/bin/libawtk.so $(TARGET_DIR)/usr/lib
	$(INSTALL) -m 0755 -D $(@D)/awtk-linux-fb/release/bin/libtkc.so $(TARGET_DIR)/usr/lib
	$(INSTALL) -m 0755 -D $(@D)/awtk-linux-fb/release/bin/libegl_devices.so $(TARGET_DIR)/usr/lib
	mkdir -p $(TARGET_DIR)/usr/bin/awtk/release/bin
	$(INSTALL) -m 0755 -D $(@D)/awtk-linux-fb/release/bin/demo $(TARGET_DIR)/usr/bin/awtk/release/bin
	mkdir -p ${TOPDIR}/../out/target/ac83xx/usr/lib/awtk/image/
	#cp -r $(@D)/awtk-linux-fb/release/assets $(TARGET_DIR)/data/awtk/image/
	cp -r $(@D)/awtk-linux-fb/release/assets ${TOPDIR}/../out/target/ac83xx/usr/lib/awtk/image/
endef

$(eval $(generic-package))
