#!/bin/bash
# Headless build: vanilla LVGL v9.3.0 + the board lv_conf (minus host-incompatible drivers) + the real ui_Music.c
set -e; source "$(dirname "$0")/env.sh"
if [ ! -f $B/liblvgl_hl.a ]; then
  export B SANF INC DEF
  find $LV/lvgl/src -name '*.c' -not -path '*/src/drivers/*' | xargs -P"$(nproc)" -I{} sh -c 'gcc -O1 -w $SANF -c $INC $DEF {} -o $B/lv/$(echo {} | md5sum | cut -c1-12).o'
  ar rcs $B/liblvgl_hl.a $B/lv/*.o
fi
for f in ui_Music ui_Music_cf music_art music_search music_pinyin; do gcc $CF -c $APP/lvgl/lvgl_app/screens/$f.c -o $B/$f.o; done
gcc $CF -w -c $APP/lvgl/lvgl_app/screens/music_pinyin_table.c -o $B/music_pinyin_table.o
gcc $CF -c fake_music_app.c -o $B/fake.o
gcc $CF -w -c stb_impl.c -o $B/stb.o
gcc $CF -c $APP/adapters/music/src/music_mainloop.c -o $B/mm_ui.o
gcc $CF -c shots.c -o $B/shots.o
for s in 20 32; do [ -f $B/f$s.o ] || gcc -O1 -w $INC $DEF -c $APP/lvgl/lvgl_app/fonts/music_font_$s.c -o $B/f$s.o; done
gcc $SANF -o $B/shots $B/shots.o $B/ui_Music.o $B/ui_Music_cf.o $B/music_art.o $B/music_search.o $B/music_pinyin.o $B/music_pinyin_table.o $B/fake.o $B/stb.o $B/mm_ui.o $B/f20.o $B/f32.o $B/liblvgl_hl.a -lm -lpthread
echo BUILD OK
