#! /bin/bash
#
# cluster-app
#
# description: Run cluster-app


start() {
	echo "cluster-app start"
	/usr/bin/cluster-app &
}

stop() {
	echo "cluster-app stop"
	killall cluster-app
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

