#! /bin/bash
#
# S01awtk — Start AWTK music player application
#

TOUCH_DEV="/dev/input/event0"
WAIT_MAX=10

start() {
	echo "awtk start"

	# Wait for touch input device (gt9xx loaded by S70modutils)
	i=0
	while [ ! -e "$TOUCH_DEV" ] && [ $i -lt $WAIT_MAX ]; do
		echo "waiting for $TOUCH_DEV ($i/${WAIT_MAX}s)..."
		sleep 1
		i=$((i+1))
	done

	if [ ! -e "$TOUCH_DEV" ]; then
		echo "WARNING: $TOUCH_DEV not found after ${WAIT_MAX}s, starting anyway"
	fi

	mkdir -p /data/music
	cd /usr/lib/awtk/image
	/usr/bin/music_player &
}

stop() {
	echo "awtk stop"
	killall music_player 2>/dev/null
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

