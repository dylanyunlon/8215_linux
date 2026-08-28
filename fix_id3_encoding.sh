#!/bin/bash
# fix_id3_encoding.sh — 修复音乐播放器 ??? 乱码问题
#
# 在 192.168.70.17 跳板机上执行
# 用法: bash fix_id3_encoding.sh
#
# 做的事:
#   1. 跳板机 git pull 拉最新代码
#   2. scp 三个改动文件到 126 整编机
#   3. 126 整编机清除 musicplayer 缓存并重编
#
set -e

JUMP_REPO="/home/dylan/Desktop/0520/8215_linux"   # ← 改成跳板机上的实际路径
REMOTE="tanyunlong@192.168.0.126"
REMOTE_BASE="/work2/tanyunlong"

echo "========== Step 1: git pull on jump host =========="
cd "$JUMP_REPO"
git pull origin main
echo ""

echo "========== Step 2: scp changed files to 126 =========="
# 只传改动的3个文件，不传整个项目
scp source/packages/application/musicplayer/awtk_app/src/cmus/convert.c \
    ${REMOTE}:${REMOTE_BASE}/8215_linux/source/packages/application/musicplayer/awtk_app/src/cmus/convert.c

scp source/packages/application/musicplayer/awtk_app/src/cmus/id3.c \
    ${REMOTE}:${REMOTE_BASE}/8215_linux/source/packages/application/musicplayer/awtk_app/src/cmus/id3.c

scp source/packages/application/musicplayer/music_player.cpp \
    ${REMOTE}:${REMOTE_BASE}/8215_linux/source/packages/application/musicplayer/music_player.cpp

echo "3 files synced."
echo ""

echo "========== Step 3: rebuild on 126 =========="
ssh ${REMOTE} << 'EOF'
cd /work2/tanyunlong/8215_linux

# 只清除 musicplayer 相关的编译缓存
rm -rf ~/out/build/ac83xx/musicplayer-1.0/
rm -rf ~/out/build/ac83xx/musicplayer_awtk-1.0/

# 编译
./allmake.sh -a nand-512-ddr-512 -d userdebug -m false
EOF

echo ""
echo "========== Done =========="
echo "修改内容:"
echo "  1. convert.c: iconv 失败时走纯 C 的 Latin-1→UTF-8 转换"
echo "  2. id3.c: ISO-8859-1/UTF-8 标签解析失败时尝试 GBK→UTF-8 (国产 MP3 常见)"
echo "  3. id3.c: ID3v1 标签同样加了 GBK fallback"
echo "  4. music_player.cpp: 修复软解模式下 next() 的段错误"
