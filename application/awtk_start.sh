#! /bin/bash
#
# Music Player (AWTK Linux-FB)
#

start() {
	echo "musicplayer start"
	mkdir -p /data/music
	cd /usr/lib/awtk/image
	/usr/bin/music_player &
}

stop() {
	echo "musicplayer stop"
	killall music_player
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
  status)
	;;
  condrestart)
	stop
	start
	;;
  *)
	echo $"Usage: $0 {start|stop|status|restart|condrestart}"
	exit 1
esac
