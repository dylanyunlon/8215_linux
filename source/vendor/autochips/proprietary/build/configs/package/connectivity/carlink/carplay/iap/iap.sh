#!/bin/sh

DAEMON="$(which iap)"
DESC="iap"
TAG="iap"

pr_info() {
    echo "[$TAG] $@"
    logger -p info -t $TAG "$@"
}

pr_err() {
    echo "[$TAG] $@"
    logger -p info -t $TAG "$@"
}

usage() {
    pr_info "Usage: $0 <start|start_fg|stop>"
}

start() {
    pr_info "Starting $DESC..."
    eval start-stop-daemon --start --background --exec $DAEMON -- $DAEMON_OPTS
    ret=$?
    if [ $ret -eq 0 ]; then
        pr_info "$DESC started successfully."
    else
        pr_err "Failed to start $DESC. Error code: $ret."
    fi

    return $ret
}

start_fg() {
    pr_info "Starting $DESC..."
    eval exec start-stop-daemon --start --exec $DAEMON -- $DAEMON_OPTS
    ret=$?
    if [ $ret -eq 0 ]; then
        pr_info "$DESC exit code: $ret."
    else
        pr_err "Failed to start $DESC or exit with Error code: $ret."
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
    start_fg)
        start_fg
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
