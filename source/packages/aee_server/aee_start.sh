#! /bin/bash
#
# AEE Service
#
# description: Run AEE Service


start() {
	echo "aee start for dump KE if needed."
	/usr/bin/aee &
}

stop() {
	echo "aee service stop"
	killall aee
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

