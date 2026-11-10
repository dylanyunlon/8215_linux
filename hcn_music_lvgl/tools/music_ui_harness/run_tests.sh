#!/bin/bash
# One-shot verification on the host (x86, vanilla LVGL v9.3.0 + board lv_conf).  Needs: gcc, git, python3 (PNG export only).
#   MUSIC_SRC=<.../source/packages/application/musicplayer>   LVGL_DIR=/tmp/lvh   (cloned automatically)
set -e; source "$(dirname "$0")/env.sh"
step() { printf '\n[%s] %s\n' "$1" "$2"; }

step 1/5 "AWTK-primitive shim: stress (8 producer threads) under ASan+UBSan and TSan"
for san in address,undefined thread; do
  gcc -std=gnu11 -g -O1 -fsanitize=$san -Wall -Wextra -I"$APP/adapters/music/include" -I"$APP/adapters/music/include/compat" \
      "$APP/adapters/music/test/test_mainloop.c" "$APP/adapters/music/src/music_mainloop.c" -o /tmp/tm_$$ -lpthread && /tmp/tm_$$ | tail -1
done

step 2/5 "UI scenario (33 assertions: lists, tabs, drill-down, lyrics, empty states, LVGL-heap leak loop) + PNG frames in shots/"
./build.sh | tail -1; rm -rf shots; ./build/shots | grep -E 'checks passed|LVGL heap'

step 3/5 "Audio focus against the real audiofocusmanager.c (224 assertions)"
./build_focus.sh | tail -1; ./build/test_focus 2>&1 | grep -E 'PASS|FAILED'

step 4/5 "Everything above again under ASan+UBSan+LeakSanitizer"
SAN=1 ./build.sh | tail -1; SAN=1 ./build_focus.sh | tail -1
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 ./build_san/shots | grep -E 'checks passed'
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 ./build_san/test_focus 2>&1 | grep -E 'PASS|FAILED'

step 5/5 "Real music_app.c / favorite_manager.c / usb_monitor.c compile unchanged against the shim (C99)"
for f in music_app favorite_manager usb_monitor; do
  if gcc -std=gnu99 -Wall -Wno-format-truncation -fsyntax-only -D_GNU_SOURCE -I"$APP/adapters/music/include/compat" \
         -I"$APP/adapters/music/include" -I$S -I$S/cmus -I$MS $S/$f.c; then echo "  $f.c ok"; else echo "  $f.c FAILED"; exit 1; fi
done
echo; echo "ALL GREEN"
