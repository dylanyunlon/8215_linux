#!/bin/sh

PROGNAME="wpa_supplicant"
DAEMON="$(which $PROGNAME)"
DESC="$PROGNAME"
TAG="$PROGNAME"

WPA_MODE="-B -u -s -i wlan0 -D nl80211"
WPA_CONFIG="-c /data/misc/wifi/server/wpa_supplicant.conf"
# WPA_DEBUG="-d"
DAEMON_ENV=""
DAEMON_OPTS="$WPA_MODE $WPA_CONFIG $WPA_DEBUG"

pr_info() {
    echo "[$TAG] $@"
    logger -p info -t $TAG "$@"
}

pr_err() {
    echo "[$TAG] $@"
    logger -p info -t $TAG "$@"
}

usage() {
    pr_info "Usage: $0 <start|stop> [sta|ap]"
}

start() {
    if [ $# -lt 1 ]; then
        usage
        exit 1
    fi
    pr_info "Starting $DESC-$1..."
    eval $DAEMON_ENV start-stop-daemon --start --exec $DAEMON -- $DAEMON_OPTS
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
        start $2
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
