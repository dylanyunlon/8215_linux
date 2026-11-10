# shared by build.sh / build_focus.sh / run_tests.sh
cd "$(dirname "${BASH_SOURCE[0]}")"
APP=$(cd ../.. && pwd)                                   # .../source/vendor/hcn/application
LV=${LVGL_DIR:-/tmp/lvh}                                 # holds a vanilla LVGL v9.3.0 checkout in $LV/lvgl
[ -d "$LV/lvgl/src" ] || git clone -q --depth 1 --branch v9.3.0 https://github.com/lvgl/lvgl.git "$LV/lvgl"
MS=${MUSIC_SRC:-$APP/../../../packages/application/musicplayer}   # source/packages/application/musicplayer
S=$MS/awtk_app/src
FW=$APP/lvgl/lvgl_app/frameworks                         # audiofocusmanager.c, displayfocusmanager.h, music_focus.c
B=build${SAN:+_san}; SANF=${SAN:+-fsanitize=address,undefined -fno-omit-frame-pointer}
INC="-I$B/conf -I. -I$LV -I$LV/lvgl -I$APP/adapters/music/include -I$APP/adapters/music/include/compat -I$APP/lvgl/lvgl_app/screens -I$FW -I$S -I$S/cmus -I$MS"
DEF="-DLV_CONF_INCLUDE_SIMPLE -D_GNU_SOURCE"            # NOTE: never -DAWTK_STUB (see adapters/music/include/compat/awtk.h)
CF="$SANF -std=gnu11 -O1 -g -Wall -Wextra -Wno-unused-parameter $INC $DEF"
mkdir -p "$B/lv" "$B/conf"

# lv_conf: the board's real file if BOARD_LV_CONF is set (only the host-incompatible switches are turned off),
# otherwise the bundled copy of the 9/21 conf.  LVGL_DIR may point at the board's LVGL fork (dir that contains lvgl/).
if [ -n "$BOARD_LV_CONF" ]; then
  sed -e 's/^\(\s*#define LV_USE_\(OPENGLES\|DRAW_OPENGLES\|LINUX_FBDEV\|EVDEV\|ATCIMAGE\|LIBPNG\|LIBJPEG_TURBO\|PERF_MONITOR\|MEM_MONITOR\|LOG\)\s\+\)1/\10/' \
      -e 's/^\(\s*#define\s\+LV_USE_DRAW_SW_ASM\s\+\)LV_DRAW_SW_ASM_[A-Z0-9]*/\1LV_DRAW_SW_ASM_NONE/' \
      "$BOARD_LV_CONF" > "$B/conf/lv_conf.h"
  echo "[env] using board lv_conf: $BOARD_LV_CONF"
else
  cp lv_conf.h "$B/conf/lv_conf.h"
fi
