#! /bin/bash
#
# cluster-app
#
# description: Run cluster-app


start() {
	echo "cluster-qd start"
	/usr/bin/QD_demo &
}

stop() {
	echo "cluster-qd stop"
	killall QD_demo
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

