#!/bin/bash
# 把 DA 树还原成"不带我们任何改动"的纯基线，再用 nand-256-ddr-256 从零整编。
# 用法: bash da_pure_baseline.sh            (默认 T=/work2/tanyunlong/dylan/AC8215_DA, D=~/dylan)
# 任何一步不符合预期就停下，不会启动整编。改动前先把当前状态整体备份到 $D/music_state_backup_<时间>.tgz。
set -e
T=${T:-/work2/tanyunlong/dylan/AC8215_DA}; D=${D:-$HOME/dylan}
B=$T/source/vendor/autochips/proprietary/build/configs; A=$T/source/vendor/hcn/application
P=$D/music_pack_clean; TS=$(date +%m%d_%H%M)
die(){ echo "!! $*  —— 已停止，没有整编"; exit 1; }

echo "== 0) 前置检查"
for f in "$D/ac83xx_defconfig.before_music" "$B/Config.in.orig" "$B/package/hcn_application/hcn_application.mk.orig" \
         "$D/alsa-plugins.mk.orig" "$P/integration.patch"; do [ -f "$f" ] || die "缺备份: $f"; done
pgrep -f '\./allmake\.sh -a' >/dev/null && die "还有 allmake.sh 在跑"

echo "== 1) 备份当前(带音乐)状态"
BK=(); for x in "$B/configs/ac83xx_defconfig" "$B/Config.in" "$B/package/hcn_application/hcn_application.mk" \
              "$B/package/alsa-plugins/alsa-plugins.mk" "$B/package/application/musicplayer" "$A"; do [ -e "$x" ] && BK+=("${x#/}"); done
tar czf "$D/music_state_backup_$TS.tgz" --exclude=.git -C / "${BK[@]}" || die "备份失败"
ls -l "$D/music_state_backup_$TS.tgz"

echo "== 2) 构建配置还原"
cp "$D/ac83xx_defconfig.before_music"                  "$B/configs/ac83xx_defconfig"
cp "$B/Config.in.orig"                                 "$B/Config.in"
cp "$B/package/hcn_application/hcn_application.mk.orig" "$B/package/hcn_application/hcn_application.mk"
cp "$D/alsa-plugins.mk.orig"                           "$B/package/alsa-plugins/alsa-plugins.mk"
mkdir -p "$D/removed_$TS"; [ -d "$B/package/application/musicplayer" ] && mv "$B/package/application/musicplayer" "$D/removed_$TS/musicplayer_pkg"; true

echo "== 3) application 还原 (撤 integration.patch + 移走我们加的 23 个文件)"
if grep -q 'HCN_APP_MUSIC' "$A/CMakeLists.txt"; then
  (cd "$A" && patch -R -p1 --dry-run -s < "$P/integration.patch") || die "integration.patch 撤不干净"
  (cd "$A" && patch -R -p1 -s < "$P/integration.patch")
fi
(cd "$P" && find adapters/music lvgl -type f) | while read f; do
  if [ -e "$A/$f" ]; then mkdir -p "$D/removed_$TS/app/$(dirname "$f")"; mv "$A/$f" "$D/removed_$TS/app/$f"; fi; done
rm -rf "$A/adapters/music"

echo "== 4) 核对：不应再有任何音乐痕迹"
bad=0
grep -qE '^BR2_PACKAGE_(FFMPEG|MUSICPLAYER)=' "$B/configs/ac83xx_defconfig" && { echo "defconfig 里还有 FFMPEG/MUSICPLAYER"; bad=1; }
grep -q '^BR2_PACKAGE_LVGL_HOME=y' "$B/configs/ac83xx_defconfig" || { echo "LVGL_HOME 不是 y（纯基线应保持原样）"; bad=1; }
grep -q musicplayer "$B/Config.in" && { echo "Config.in 里还有 musicplayer"; bad=1; }
grep -qE 'musicplayer|HCN_APP_MUSIC' "$B/package/hcn_application/hcn_application.mk" && { echo "hcn_application.mk 还有音乐"; bad=1; }
grep -q disable-avcodec "$B/package/alsa-plugins/alsa-plugins.mk" && { echo "alsa-plugins.mk 还有 --disable-avcodec"; bad=1; }
grep -rlE 'HCN_APP_MUSIC|HCN_APP_CFG_MUSIC|music_adapter|ui_Music' "$A" --include='*.c' --include='*.h' --include='*.txt' --include='*.cmake' && bad=1
find "$A" -name '*.rej' | grep . && bad=1
[ $bad = 0 ] || die "还原不完整"
echo "参考哈希(仓库 b301aad 的 da_min 快照；不一致不一定是错，可能是快照时间不同):"
echo "  defconfig 5e2330b1edb06957  Config.in 832105453561ec42  hcn_application.mk 5904e2e81f160962"
sha256sum "$B/configs/ac83xx_defconfig" "$B/Config.in" "$B/package/hcn_application/hcn_application.mk" | awk '{print "  "substr($1,1,16), $2}'

echo "== 5) 全新 out/ 从零整编"
[ -d "$T/out" ] && mv "$T/out" "$D/out_nand256_music_$TS"
[ -d "$T/release_images" ] && mv "$T/release_images" "$D/release_images_nand256_music_$TS"
cd "$T" && nohup ./allmake.sh -a nand-256-ddr-256 -d userdebug -m false > "$D/build_pure_nand256.log" 2>&1 &
echo "已启动整编，日志 $D/build_pure_nand256.log"
