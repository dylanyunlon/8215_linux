#! /bin/bash
#
# module driver utils
#
# description: install or remove module drivers


start() {
	#Install drivers
	modprobe mtz_drv.ko
	modprobe adec.ko
#	modprobe gt9xx.ko
	modprobe arm2system_service.ko
}

stop() {
	#Remove drivers
	echo "stop"
#	modprobe -r bootanidrv.ko
#	modprobe -r mali.ko
#	modprobe -r ump.ko
#	modprobe -r mtz_drv.ko
#	modprobe -r adec.ko
#	modprobe -r gt9xx.ko
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

