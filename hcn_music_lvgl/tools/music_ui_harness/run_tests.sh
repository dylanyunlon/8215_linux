#!/bin/bash
# One-shot verification on the host (x86).  Needs: gcc, git, python3.
#   LVGL_DIR / BOARD_LV_CONF : the board's LVGL fork + real lv_conf.h (see ../../README_MUSIC.md); default = vanilla v9.3.0 + bundled conf
#   MUSIC_SRC                : source/packages/application/musicplayer
set -e; source "$(dirname "$0")/env.sh"
step() { printf '\n[%s] %s\n' "$1" "$2"; }
SCR=$APP/lvgl/lvgl_app/screens; AD=$APP/adapters/music

step 1/8 "AWTK-primitive shim: stress (8 producer threads) under ASan+UBSan and TSan"
for san in address,undefined thread; do
  gcc -std=gnu11 -g -O1 -fsanitize=$san -Wall -Wextra -I"$AD/include" -I"$AD/include/compat" "$AD/test/test_mainloop.c" "$AD/src/music_mainloop.c" -o /tmp/tm_$$ -lpthread && /tmp/tm_$$ | tail -1
done

step 2/8 "Every non-ASCII character of every UI string is in the generated font"
python3 "$APP/tools/check_font_coverage.py"

step 3/8 "Pinyin-initial matching + search runner (heteronyms, malformed UTF-8, truncation) under ASan+UBSan"
gcc -std=gnu11 -g -O1 -fsanitize=address,undefined -Wall -Wextra -Wno-unused-parameter -D_GNU_SOURCE -I"$AD/include" -I"$AD/include/compat" -I$SCR -I$S -I$S/cmus -I$MS \
    "$AD/test/test_search.c" $SCR/music_search.c $SCR/music_pinyin.c $SCR/music_pinyin_table.c -o /tmp/ts_$$ && /tmp/ts_$$

step 4/8 "Album-art pipeline: decode, rounded corners, backdrop, async loader, LRU cache, shutdown races (ASan+UBSan, TSan)"
for san in address,undefined thread; do
  gcc -std=gnu11 -g -O1 -fsanitize=$san -Wall -Wextra -Wno-unused-parameter $INC $DEF test_art.c stb_impl.c $SCR/music_art.c "$AD/src/music_mainloop.c" -o /tmp/ta_$$ -lm -lpthread && /tmp/ta_$$ | tail -1
done

step 5/8 "UI scenario (real pointer: taps, drags, long-press, keyboard keys) + PNG frames in shots/"
./build.sh | tail -1; rm -rf shots; ./build/shots | grep -E 'checks passed|LVGL heap'

step 6/8 "Audio focus against the real audiofocusmanager.c (224 assertions)"
./build_focus.sh | tail -1; ./build/test_focus 2>&1 | grep -E 'PASS|FAILED'

step 7/8 "UI scenario + audio focus again under ASan+UBSan+LeakSanitizer (LVGL uses libc malloc on the board, so leaks are visible)"
SAN=1 ./build.sh | tail -1; SAN=1 ./build_focus.sh | tail -1
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 ./build_san/shots | grep -E 'checks passed'
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 ./build_san/test_focus 2>&1 | grep -E 'PASS|FAILED'

step 8/8 "Real music_app.c / favorite_manager.c / usb_monitor.c compile unchanged against the shim (C99)"
for f in music_app favorite_manager usb_monitor; do
  if gcc -std=gnu99 -Wall -Wno-format-truncation -fsyntax-only -D_GNU_SOURCE -I"$AD/include/compat" -I"$AD/include" -I$S -I$S/cmus -I$MS $S/$f.c; then echo "  $f.c ok"; else echo "  $f.c FAILED"; exit 1; fi
done
echo; echo "ALL GREEN"
