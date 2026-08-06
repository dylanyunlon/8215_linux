#!/bin/sh

DAEMON="$(which appmanager)"
DESC="appmanager"
TAG="appmanager"

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
    pr_info "Starting $DESC..."
    modprobe gt9xx.ko
    eval start-stop-daemon --start --background --exec $DAEMON -- $DAEMON_OPTS
    ret=$?
    if [ $ret -eq 0 ]; then
        pr_info "$DESC started successfully."
    else
        pr_err "Failed to start $DESC. Error code: $ret."
    fi

    return $ret
}

stop() {
    pr_info "Stopping $DESC..."
    start-stop-daemon --stop --exec $DAEMON
    ret=$?
    if [ $ret -eq 0 ]; then
        pr_info "$DESC stopped successfully."
    else
        pr_err "Failed to stop $DESC. Error code: $ret."
    fi

    return $ret
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    *)
        usage
        exit 1
        ;;
esac

exit $?
