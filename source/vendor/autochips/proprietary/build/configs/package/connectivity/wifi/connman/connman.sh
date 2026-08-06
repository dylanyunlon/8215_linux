#!/bin/sh

PROGNAME="connmand"
DAEMON="$(which $PROGNAME)"
DESC="$PROGNAME"
TAG="$PROGNAME"

# DAEMON_ENV+=" CONNMAN_SUPPLICANT_DEBUG=1"
DAEMON_ENV+=" CONNMAN_DHCP_DEBUG=1"

# DAEMON_OPTS+=" -d"

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
    eval $DAEMON_ENV start-stop-daemon --start --exec $DAEMON -- $DAEMON_OPTS
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
    eval $DAEMON_ENV exec start-stop-daemon --start --exec $DAEMON -- -n $DAEMON_OPTS
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
    for i in $(seq 20); do  # 2s
        if ! killall -0 $PROGNAME 2>/dev/null; then
            break
        fi
        sleep 0.1
    done
    if killall -0 $PROGNAME 2>/dev/null; then
        pr_err "Failed to stop $DESC, kill -9 ..."
        killall -9 $PROGNAME
        for i in $(seq 20); do  # 2s
            if ! killall -0 $PROGNAME 2>/dev/null; then
                break
            fi
            sleep 0.1
        done
    fi
    ! killall -0 $PROGNAME 2>/dev/null
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
