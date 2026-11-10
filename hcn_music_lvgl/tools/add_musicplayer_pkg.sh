#!/bin/bash
# STEP 1: put the musicplayer package (libmusicplayer.so + CLI test tools) into a musl/Buildroot-2019.02 DA tree.
#   usage:  add_musicplayer_pkg.sh <tree> <musicplayer_src_backup.tgz>        (idempotent; originals saved as *.orig)
#   - source   : unpack source/packages/application/musicplayer from the backup if the directory is missing
#   - package  : configs/package/application/musicplayer/{Config.in,musicplayer.mk}  (toolchain-neutral rewrite:
#                the old .mk hard-coded arm-buildroot-linux-gnueabi, this tree is arm-buildroot-linux-musleabihf)
#   - Config.in: one `source` line after the last package/application/ line
#   - defconfig: BR2_PACKAGE_MUSICPLAYER=y   (musicplayer_awtk is NOT added: it needs AWTK)
set -e
TREE=${1:?usage: $0 <tree> <musicplayer_src_backup.tgz>}; SRC_TGZ=${2:?missing backup tgz}
B=$TREE/source/vendor/autochips/proprietary/build/configs
[ -f "$B/Config.in" ] && [ -f "$B/configs/ac83xx_defconfig" ] || { echo "not a DA tree: $B"; exit 1; }

if [ ! -d "$TREE/source/packages/application/musicplayer" ]; then
  tar -xzf "$SRC_TGZ" -C "$TREE" && echo "[src] unpacked source/packages/application/musicplayer"
else echo "[src] already present"; fi
[ -f "$TREE/source/packages/application/musicplayer/Makefile" ] || { echo "musicplayer/Makefile missing after unpack"; exit 1; }

PKG=$B/package/application/musicplayer; mkdir -p "$PKG"
[ -f "$PKG/musicplayer.mk" ] && [ ! -f "$PKG/musicplayer.mk.orig" ] && cp "$PKG/musicplayer.mk" "$PKG/musicplayer.mk.orig"
cat > "$PKG/Config.in" <<'EOF'
config BR2_PACKAGE_MUSICPLAYER
	bool "musicplayer"
	help
	  Local music engine (libmusicplayer.so): ID3 parsing, scanner, playlist,
	  musikcube soft player (FFmpeg + ALSA). Uses libatcmediaplayer.
EOF
cat > "$PKG/musicplayer.mk" <<'EOF'
################################################################################
#
# musicplayer - local music engine (libmusicplayer.so) + CLI test tools
#
# Toolchain-neutral: uses Buildroot's TARGET_* variables, so it works for the musl/hard-float tree
# (arm-buildroot-linux-musleabihf) as well as the old glibc one.  Needs FFmpeg + alsa-lib in the sysroot
# (libavformat libavcodec libavutil libswresample libasound libatomic).
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
	CC="$(TARGET_CXX)" \
	CC_C="$(TARGET_CC)" \
	SYSROOT_DIR=$(STAGING_DIR)

define MUSICPLAYER_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(MUSICPLAYER_MAKE_OPTS) all
	@echo "Building musicplayer test binary..."
	$(TARGET_CXX) \
		--sysroot=$(STAGING_DIR) \
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
		CROSS_COMPILE=$(TARGET_CROSS) \
		SYSROOT_DIR=$(STAGING_DIR)
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
EOF
echo "[pkg] wrote $PKG"

CI=$B/Config.in; LINE='source "$BR2_EXTERNAL_AC8X_PATH/package/application/musicplayer/Config.in"'
if grep -qF 'package/application/musicplayer/Config.in' "$CI"; then echo "[Config.in] already has musicplayer"; else
  [ -f "$CI.orig" ] || cp "$CI" "$CI.orig"
  N=$(grep -n 'package/application/.*Config.in' "$CI" | tail -1 | cut -d: -f1)
  [ -n "$N" ] || { echo "no package/application/ line in Config.in to anchor on"; exit 1; }
  sed -i "${N}a $LINE" "$CI" && echo "[Config.in] added after line $N"
fi
DC=$B/configs/ac83xx_defconfig
if grep -q '^BR2_PACKAGE_MUSICPLAYER=y' "$DC"; then echo "[defconfig] already set"; else
  [ -f "$DC.orig" ] || cp "$DC" "$DC.orig"
  grep -q '^BR2_PACKAGE_MUSICPLAYER_AWTK=y' "$DC" && sed -i 's/^BR2_PACKAGE_MUSICPLAYER_AWTK=y/BR2_PACKAGE_MUSICPLAYER_AWTK=n/' "$DC"
  echo 'BR2_PACKAGE_MUSICPLAYER=y' >> "$DC" && echo "[defconfig] BR2_PACKAGE_MUSICPLAYER=y"
fi

echo; echo "--- readiness (needs a previous build in $TREE/out) ---"
for l in libavformat libavcodec libavutil libswresample libasound libatcmediaplayer libatomic; do
  f=$(ls "$TREE"/out/staging/usr/lib/${l}.so* "$TREE"/out/host/*/sysroot/usr/lib/${l}.* 2>/dev/null | head -1); printf '  %-18s %s\n' $l "${f:-MISSING}"; done
for h in libavformat/avformat.h alsa/asoundlib.h; do
  [ -f "$TREE/out/staging/usr/include/$h" ] && echo "  header $h ok" || echo "  header $h MISSING"; done
echo "next: rebuild the tree, then check out/staging/usr/lib/libmusicplayer.so"
