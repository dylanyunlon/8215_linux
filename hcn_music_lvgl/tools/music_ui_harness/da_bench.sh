#!/bin/bash
# 用 126 树(da_tree_126)里板上实际跑的界面文件 + hcn_music_lvgl 的字体，在宿主机上构建 harness 并跑滑动基准。
# 原因：hcn_music_lvgl/lvgl/lvgl_app/frameworks 里没有重构后的 displayfocusmanager.h，harness 直接在原位置构建会失败。
# 用法:  ./da_bench.sh               (需要 gcc、git、python3；首次会 clone LVGL v9.3.0 到 /tmp/lvh)
#        FONTS=<字体目录> ./da_bench.sh   比较别的字体版本
set -e
HERE=$(cd "$(dirname "$0")" && pwd); REPO=$(cd "$HERE/../../.." && pwd)
D=$REPO/da_tree_126/source/vendor/hcn/application
W=${WORK:-/tmp/hcn_harness_app}
rm -rf "$W"; mkdir -p "$W/lvgl/lvgl_app" "$W/tools"
ln -s "$D/lvgl/lvgl_app/screens"    "$W/lvgl/lvgl_app/screens"
ln -s "$D/lvgl/lvgl_app/frameworks" "$W/lvgl/lvgl_app/frameworks"
ln -s "${FONTS:-$REPO/hcn_music_lvgl/lvgl/lvgl_app/fonts}" "$W/lvgl/lvgl_app/fonts"
ln -s "$D/adapters" "$W/adapters"
cp -r "$HERE" "$W/tools/music_ui_harness"; cp "$HERE/../check_font_coverage.py" "$W/tools/"
export MUSIC_SRC=$REPO/source/packages/application/musicplayer LVGL_DIR=${LVGL_DIR:-/tmp/lvh}
cd "$W/tools/music_ui_harness" && ./build.sh | tail -1 && ./bench.sh
