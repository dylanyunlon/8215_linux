#!/bin/bash
# 滑动列表渲染基准 + gprof 剖析。需先 ./build.sh 成功（或用 da_bench.sh 一键完成）。
#   BENCH_FRAMES=1500 ./bench.sh      TOP=25 ./bench.sh
set -e; source "$(dirname "$0")/env.sh"
gcc $CF -pg -c bench_scroll.c -o $B/bench.o
gcc -pg -o $B/bench $B/bench.o $B/ui_Music.o $B/ui_Music_cf.o $B/music_art.o $B/music_search.o $B/music_pinyin.o \
    $B/music_pinyin_table.o $B/fake.o $B/stb.o $B/mm_ui.o $B/f20.o $B/f32.o $B/liblvgl_hl.a -lm -lpthread
cd $B && rm -f gmon.out && BENCH_FRAMES=${BENCH_FRAMES:-1500} ./bench && echo && gprof -b -p ./bench gmon.out 2>/dev/null | head -n $((${TOP:-20}+5))
