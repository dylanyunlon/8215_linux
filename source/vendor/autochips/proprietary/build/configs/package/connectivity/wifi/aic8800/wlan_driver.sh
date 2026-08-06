#!/bin/sh

DESC="wlan driver"
TAG="wlan"

pr_info() {
    echo "[$TAG] $@"
    logger -p info -t $TAG "$@"
}

pr_err() {
    echo "[$TAG] $@"
    logger -p info -t $TAG "$@"
}

usage() {
    pr_info "Usage: $0 <start|stop>"
}

start() {
    pr_info "insmod $DESC..."
    #modprobe atc_combo.ko
    modprobe aic8800_bsp.ko
    modprobe aic8800_fdrv.ko
    modprobe aic8800_btlpm.ko
    ret=$?
    if [ $ret -eq 0 ]; then
        pr_info "insmod $DESC successfully."
    else
        pr_err "Failed to insmod $DESC. Error code: $ret."
    fi

    return $ret
}

stop() {
    if [ $# -lt 1 ]; then
        pr_info "skip rmmod $DESC"
        return 0
    fi
    pr_info "rmmod $DESC..."
    modprobe -r aic8800_btlpm.ko
    modprobe -r aic8800_fdrv.ko
    modprobe -r aic8800_bsp.ko
    modprobe -r atc_combo.ko
    ret=$?
    if [ $ret -eq 0 ]; then
        pr_info "rmmod $DESC successfully."
    else
        pr_err "Failed to rmmod $DESC. Error code: $ret."
    fi

    return $ret
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop $2
        ;;
    *)
        usage
        exit 1
        ;;
esac

exit $?
