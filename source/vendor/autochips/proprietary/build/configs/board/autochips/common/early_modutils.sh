#! /bin/bash
#
# module driver utils
#
# description: install or remove module drivers


start() {
	#Install drivers
	modprobe dualarmdrv.ko
	modprobe atc_bl.ko
	modprobe vcp.ko
	modprobe atcfb.ko
	modprobe atc-vout.ko
	modprobe bootanidrv.ko
	modprobe ump.ko
	modprobe mali.ko
}

stop() {
	#Remove drivers
	echo "stop"
#	modprobe -r atc-vout.ko
#	modprobe -r atcfb.ko
#	modprobe -r vcp.ko
#	modprobe -r atc_bl.ko
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

