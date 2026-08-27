################################################################################
#
# musicplayer_awtk - AWTK 本地音乐播放器应用 (触摸 UI)
#
# 依赖:
#   - awtk (libawtk.so, AWTK 框架)
#   - musicplayer (libmusicplayer.so, 扫描+播放引擎)
#   - atcmediaplayer (libatcmediaplayer.so, 杰发媒体播放器)
#
# 编译输出: /usr/bin/music_player (AWTK Linux-FB 可执行文件)
# 启动方式: 修改 S01awtk 或新增 S02musicplayer 启动脚本
#
################################################################################

MUSICPLAYER_AWTK_VERSION = 1.0
MUSICPLAYER_AWTK_SITE = $(TOPDIR)/../source/packages/application/musicplayer/awtk_app
MUSICPLAYER_AWTK_SITE_METHOD = local
MUSICPLAYER_AWTK_ALWAYS_BUILD = YES
MUSICPLAYER_AWTK_DEPENDENCIES += awtk musicplayer atcmediaplayer

MUSICPLAYER_AWTK_TOPDIR = $(TOPDIR)/..
MUSICPLAYER_AWTK_SYSROOT = $(MUSICPLAYER_AWTK_TOPDIR)/out/host/arm-buildroot-linux-gnueabi/sysroot

define MUSICPLAYER_AWTK_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) \
		TOPDIR=$(MUSICPLAYER_AWTK_TOPDIR) \
		CC=$(MUSICPLAYER_AWTK_TOPDIR)/out/host/bin/arm-buildroot-linux-gnueabi-gcc \
		CXX=$(MUSICPLAYER_AWTK_TOPDIR)/out/host/bin/arm-buildroot-linux-gnueabi-g++ \
		SYSROOT_DIR=$(MUSICPLAYER_AWTK_SYSROOT) \
		all
endef

define MUSICPLAYER_AWTK_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/music_player $(TARGET_DIR)/usr/bin/music_player
	mkdir -p $(TARGET_DIR)/data/music
	mkdir -p $(TARGET_DIR)/usr/lib/awtk/image/assets/default/raw
	cp -r $(@D)/assets/default/raw/images $(TARGET_DIR)/usr/lib/awtk/image/assets/default/raw/
	cp -r $(@D)/assets/default/raw/styles $(TARGET_DIR)/usr/lib/awtk/image/assets/default/raw/
	cp -r $(@D)/assets/default/raw/fonts  $(TARGET_DIR)/usr/lib/awtk/image/assets/default/raw/
	@echo "[musicplayer_awtk] Installed binary + assets"
endef

define MUSICPLAYER_AWTK_UNINSTALL_TARGET_CMDS
	rm -f $(TARGET_DIR)/usr/bin/music_player
endef

$(eval $(generic-package))
