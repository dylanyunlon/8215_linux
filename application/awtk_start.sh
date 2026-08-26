#! /bin/bash
#
# HCN application (awtk demo 二进制)
#

start() {
	echo "hcn application start"
	/usr/bin/awtk/release/bin/demo &
}

stop() {
	echo "hcn application stop"
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
