#! /bin/bash
#
# awtk
#
# description: Run awtk


start() {
	echo "awtk start"
	/usr/bin/awtk/release/bin/demo &
}

stop() {
	echo "awtk stop"
	killall demo
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

