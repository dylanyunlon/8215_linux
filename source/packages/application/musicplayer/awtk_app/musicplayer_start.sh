#!/bin/bash
#
# musicplayer - AWTK 本地音乐播放器
#
# 注意: 需要先停止 demo (S01awtk)，因为同一时间只能有一个 AWTK 应用
#       占用 framebuffer + 触摸设备。
#
# 两种使用方式:
#   方式1: 替代 demo 开机启动 (修改 S01awtk 指向 music_player)
#   方式2: 手动启动 (先 killall demo，再运行本脚本)

MUSIC_PLAYER=/usr/bin/music_player

start() {
    echo "[musicplayer] starting..."

    # 确保没有其他 AWTK 应用占用 framebuffer
    killall demo 2>/dev/null
    sleep 0.5

    # 确保数据目录存在
    mkdir -p /data/music

    # cd 到 AWTK 资源目录 (app_root=./ 需要在 assets/ 所在目录启动)
    cd /usr/lib/awtk/image

    # 启动音乐播放器 (后台运行)
    $MUSIC_PLAYER &

    echo "[musicplayer] started (pid=$!)"
}

stop() {
    echo "[musicplayer] stopping..."
    killall music_player 2>/dev/null
    echo "[musicplayer] stopped"
}

case "$1" in
  start)
    start
    ;;
  stop)
    stop
    ;;
  restart)
    stop
    sleep 1
    start
    ;;
  *)
    echo "Usage: $0 {start|stop|restart}"
    exit 1
esac
