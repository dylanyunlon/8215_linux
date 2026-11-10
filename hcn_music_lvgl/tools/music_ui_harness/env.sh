# shared by build.sh / build_focus.sh / run_tests.sh
cd "$(dirname "${BASH_SOURCE[0]}")"
APP=$(cd ../.. && pwd)                                   # .../source/vendor/hcn/application
LV=${LVGL_DIR:-/tmp/lvh}                                 # holds a vanilla LVGL v9.3.0 checkout in $LV/lvgl
[ -d "$LV/lvgl/src" ] || git clone -q --depth 1 --branch v9.3.0 https://github.com/lvgl/lvgl.git "$LV/lvgl"
MS=${MUSIC_SRC:-$APP/../../../packages/application/musicplayer}   # source/packages/application/musicplayer
S=$MS/awtk_app/src
FW=$APP/lvgl/lvgl_app/frameworks                         # audiofocusmanager.c, displayfocusmanager.h, music_focus.c
B=build${SAN:+_san}; SANF=${SAN:+-fsanitize=address,undefined -fno-omit-frame-pointer}
INC="-I. -I$LV -I$LV/lvgl -I$APP/adapters/music/include -I$APP/adapters/music/include/compat -I$APP/lvgl/lvgl_app/screens -I$FW -I$S -I$S/cmus -I$MS"
DEF="-DLV_CONF_INCLUDE_SIMPLE -D_GNU_SOURCE"            # NOTE: never -DAWTK_STUB (see adapters/music/include/compat/awtk.h)
CF="$SANF -std=gnu11 -O1 -g -Wall -Wextra -Wno-unused-parameter $INC $DEF"
mkdir -p "$B/lv"
