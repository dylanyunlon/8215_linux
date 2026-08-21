################################################################################
#
# musicplayer - Local music player service
#
################################################################################

MUSICPLAYER_VERSION = 1.0
MUSICPLAYER_SITE = $(TOPDIR)/../source/packages/application/musicplayer
MUSICPLAYER_SITE_METHOD = local
MUSICPLAYER_ALWAYS_BUILD = YES
MUSICPLAYER_INSTALL_STAGING = YES
MUSICPLAYER_DEPENDENCIES += atcmediaplayer

MUSICPLAYER_MAKE_OPTS = \
	TOPDIR=$(TOPDIR)/.. \
	CC=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-g++ \
	SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot

define MUSICPLAYER_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(MUSICPLAYER_MAKE_OPTS) all
	@echo "Building musicplayer test binary..."
	$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi-g++ \
		--sysroot=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot \
		-I$(TOPDIR)/../prebuild/multimedia/mediaplayerserver/include \
		-I$(TOPDIR)/../prebuild/multimedia/libatcmultimedia/include \
		-I$(TOPDIR)/../prebuild/multimedia/mmisc/include \
		-I$(TOPDIR)/../prebuild/multimedia/mmisc/osal/include \
		-I$(TOPDIR)/../source/packages/multimedia/directrender/include \
		-I$(TOPDIR)/../source/packages/multimedia/directrender/src/utils \
		-std=c++11 -O2 \
		$(@D)/music_main.cpp \
		-o $(@D)/musicplayer_test \
		-L$(@D) -lmusicplayer \
		-L$(STAGING_DIR)/usr/lib -latcmediaplayer \
		-lpthread -Wl,-rpath,/usr/lib
	@echo "Building music_app_cli (full stack CLI test tool)..."
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) -f Makefile.cli \
		TOPDIR=$(TOPDIR)/.. \
		CROSS_COMPILE=$(TOPDIR)/../out/host/bin/arm-buildroot-linux-gnueabi- \
		SYSROOT_DIR=$(TOPDIR)/../out/host/arm-buildroot-linux-gnueabi/sysroot
endef

define MUSICPLAYER_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0755 -D $(@D)/libmusicplayer.so $(STAGING_DIR)/usr/lib/libmusicplayer.so
	$(INSTALL) -m 0644 -D $(@D)/music_scanner.h $(STAGING_DIR)/usr/include/music_scanner.h
	$(INSTALL) -m 0644 -D $(@D)/music_player.h $(STAGING_DIR)/usr/include/music_player.h
endef

define MUSICPLAYER_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(STAGING_DIR)/usr/lib/libmusicplayer.so $(TARGET_DIR)/usr/lib/libmusicplayer.so
	$(INSTALL) -m 0755 -D $(@D)/musicplayer_test $(TARGET_DIR)/usr/bin/musicplayer_test
	$(INSTALL) -m 0755 -D $(@D)/music_app_cli $(TARGET_DIR)/usr/bin/music_app_cli
	mkdir -p $(TARGET_DIR)/data/music
endef

$(eval $(generic-package))
