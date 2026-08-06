#!/bin/sh


###########configs##############
LOG_PATH="/data/power_off_ot_test"
TEST_LOG_FILE_NAME="$LOG_PATH""/linux_mm_poweroff_test_log.log"
MODE_TEST_PATH="/app/power_off_ot_test"
MODE_TEST_FILE_TYPE=".otmode"
POWEROFF_GPIO="/sys/class/gpio/gpio136/value"
DEFAULT_DEALY_POWER_OFF_TIME=5

lscmd="busybox ls"
grepcmd="busybox grep"
awkcmd="busybox awk"
sedcmd="busybox sed"
printfcmd="busybox printf"
tarcmd="busybox tar"

ALL_LOGS=""

###########configs##############

print_log()
{
    currentdata=`date +[%Y-%m-%d:%H:%M:%S]`
    echo "$currentdata:  $1"
    #if [ ! -d "$LOG_PATH" ];then
    #   echo  "$currentdata:  $1" >> $TEST_LOG_FILE_NAME
    #else
    #   echo "$currentdata:  $1"
    #fi
}

mode_test_file_exist()
{
    if [ ! -n "$1" ];then
        print_log "mode_test_file_exist fail, invalid param"
        return 1
    fi
    
    tmpfile="$MODE_TEST_PATH""/""$1""$MODE_TEST_FILE_TYPE"
    if [ ! -f "$tmpfile" ];then
        return 2
    fi
    
    return 0
}

power_off()
{
    if [ ! -f "$POWEROFF_GPIO" ]; then
        print_log "power off fail, $POWEROFF_GPIO not exist"
        return 2
    fi
    
    if [ ! -n "$1" ];then 
        print_log "power off fail, invalid param"
        return 1
    fi
    
    if [ ! -n "$2" ];then
        print_log "power off fail, not set target"
        return 1
    fi
    
    mode_test_file_exist $2
    if [  $? != 0 ];then
        print_log " $2 power off fail, mode test file not exist"
        return 2
    fi
    
    sleep $1
    print_log " $2 power off now"
    echo 1 > $POWEROFF_GPIO
    
    return 0
}

delete_mode_test_file()
{
    mount -o remount rw /app
    if [ $? != 0 ]; then
        print_log "delete_mode_test_file remount /app fail, error($?)"
        return 1
    fi
    
    if [ -d "$MODE_TEST_PATH" ];then
        rm -rf $MODE_TEST_PATH
        if [ $? != 0 ]; then
            print_log "delete_mode_test_file rm $MODE_TEST_PATH fail,  error($?)"
            return 1
        else
            sync
        fi
    fi
    
    return 0
}

create_mode_test_file()
{
    if [ ! -n "$1" ];then
        print_log "create_mode_test_file fail, not set target"
        return 1
    fi
    
    delete_mode_test_file
    if [ $? != 0 ]; then
        print_log "create_mode_test_file rm $MODE_TEST_PATH fail,  error($?)"
        return 1
    fi
    
    if [ ! -d "$MODE_TEST_PATH" ];then
        mkdir $MODE_TEST_PATH
        if [ $? != 0 ]; then
            print_log "start_power_off_test create $MODE_TEST_PATH fail"
            return 1;
        fi
    fi

    tmpfile="$MODE_TEST_PATH""/""$1""$MODE_TEST_FILE_TYPE"
    if [ ! -f "$tmpfile" ];then
        touch $tmpfile
        if [ $? != 0 ]; then
            print_log "create_mode_test_file create mode file fail,  $tmpfile"
            return 1;
        else
            sync
        fi
    fi
    
    return 0
}

start_power_off_test()
{
    #if [ ! -d "$LOG_PATH" ];then
    #   echo "create poweroff of dir $LOG_PATH"
    #   mkdir $LOG_PATH
    #   if [ $? != 0 ]; then
    #       print_log "start_power_off_test create dir fail,  $LOG_PATH"
    #       return -1;
    #   fi
    #fi
            
    if [ ! -n "$1" ];then 
        print_log  "start power off test fail, not set test target"
        return 2
    fi
    
    create_mode_test_file $1
    if [ $? != 0 ];then 
        print_log  " $1 start power off test create test mode fail, error ($?)"
        return 2
    fi
    
    print_log " $1 start power off test"
    power_off 1 $1
    if [ $? != 0 ]; then
        print_log "start_power_off_test fail,  error($?)"
        delete_mode_test_file
        return 1
    fi
    return 0
}

stop_power_off_test()
{
    delete_mode_test_file
    if [ $? != 0 ]; then
        print_log "stop_power_off_test delete mode test file fail,  error($?)"
        return 1
    fi
    
    print_log "stop power off test"
    
    return 0
}

do_power_off()
{
    power_off 0 $1
    if [ $? != 0 ]; then
        print_log "do_power_off fail,  error($?)"
        return 1
    fi
    
    return 0
}

print_help()
{
    echo "***********************************"
    echo "***********************************"
    echo "options : start xxx --start OT test"
    echo "        eg: start home"
    echo "options : stop --stop OT test"
    echo "options : sleep xxx --sleep"
    echo "        eg: sleep home"
    echo "options : poweroff xxx --power down"
    echo "        eg: poweroff home"
    echo "***********************************"
    echo "***********************************"
}

####################################################################################
####################################################################################
####################################################################################

status=0

if [ -n "$1" ];then 
    if [ "$1" == "start" ];then
        start_power_off_test $2
        status=$?
    elif [ "$1" == "stop" ];then
        stop_power_off_test
        status=$?
    elif [ "$1" == "sleep" ];then
        if [ -n "$2" ];then 
            print_log " $2 power off sleep begin"
            sleep $DEFAULT_DEALY_POWER_OFF_TIME
            print_log " $2 power off sleep end"
        else
            print_log "power off sleep invalid param"
            status=1
        fi
    elif  [ "$1" == "poweroff" ];then
        do_power_off $2
        status=$?
    else
        print_log "invalid param, $1"
        status=1
    fi
else
    print_log "invalid param, no param"
    status=1
fi

if [ $status != 0 ];then
    print_log "error($status)..., please see help info"
    print_help
fi

exit $status
