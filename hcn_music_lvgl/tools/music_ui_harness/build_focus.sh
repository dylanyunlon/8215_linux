#!/bin/bash
# music_focus.c against the REAL audiofocusmanager.c; run build.sh first (it builds the LVGL archive)
set -e; source "$(dirname "$0")/env.sh"
gcc $CF -c test_focus.c -o $B/tf.o
gcc $CF -c $FW/music_focus.c -o $B/mf.o
gcc $CF -c $APP/adapters/music/src/music_adapter.c -o $B/ma.o
gcc $CF -c $APP/adapters/music/src/music_mainloop.c -o $B/mm.o
gcc $CF -w -c $FW/audiofocusmanager.c -o $B/afm.o
gcc $CF -c fake_music_app.c -o $B/fake_f.o
gcc $SANF -o $B/test_focus $B/tf.o $B/mf.o $B/ma.o $B/mm.o $B/afm.o $B/fake_f.o $B/liblvgl_hl.a -lm -lpthread
echo FOCUS BUILD OK
