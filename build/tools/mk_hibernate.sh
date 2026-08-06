#
# Scripts only for creating hibernation images
# @Precondition:  make sure busybox is under /data folder
#

#
# color_print - Print string with color
# @$1: specify color
# @$2: specify string which is printed with specified color
#
# -e  		enable interpretation of backslash escapes
# \e or \033 	output escapes
# format: 	\e[background_color;forground_color;HIGHLIGHTm
# default: 	\e[0m
####################################
# black		\033[30m
# red		\033[31m
# green		\033[32m
# yellow 	\033[33m
# blue		\033[34m
# purple 	\033[35m
# cyan 		\033[36m
# white		\033[37m
# reset		\033[0m
####################################
function color_print() {
	while (( $# != 0 ))
	do
		case $1 in
			-black)
				echo -ne "\033[30m";
			;;
			-red)
				echo -ne "\033[31m";
			;;
			-green)
				echo -ne "\033[32m";
			;;
			-yellow)
				echo -ne "\033[33m";
			;;
			-blue)
				echo -ne "\033[34m";
			;;
			-purple)
				echo -ne "\033[35m";
			;;
			-cyan)
				echo -ne "\033[36m";
			;;
			-white)
				echo -ne "\033[37m";
			;;
			-h|-help|--help)
				echo "Usage: color_print -color string";
				echo "Example: color_print -red red -green green";
			;;
			*)
				echo -e "$1\033[0m"
			;;
		esac
		shift
	done
}


#
# usage - Print usage of this scripts
# @disk: reboot is set by default. options are reboot|shutdown|test|testproc
# @state: disk is set by default. options are disk|mem|standby
function usage() {
    color_print -blue "Usage: make_hibernate.sh [reboot]|shutdown|test|testproc [disk]|mem| [mtdblock10]"
    color_print -blue "Example: make_hibernate.sh"
    color_print -blue "Example: make_hibernate.sh shutdown disk mtdblock10"
}

function create_swap() {
#check if file exist
	[ -e /dev/block/$1 ] || color_print  -red "/dev/block/$1 does NOT exist."
	/data/busybox mkswap "/dev/block/$1"
	/data/busybox swapon "/dev/block/$1"
}

function release_pagecaches() {
	local DROP_CACHE=/proc/sys/vm/drop_caches
	echo $1 >$DROP_CACHE
	color_print -blue "caches are released before suspend"
}

function set_power() {
#check if file exist
	[ -e /sys/power/disk ] || color_print -red "System does NOT support hibernation."
	echo $1>/sys/power/disk
	echo $2>/sys/power/state
}

function main() {
    local DISK=shutdown
    local STATE=disk
    local SWAP=mtdblock10

    if [ $# -ne 0 ] && [ $# -ne 3 ]; then
        usage
	exit
    fi

    if [ $# -eq 3 ]; then
        DISK=$1
        STATE=$2
        SWAP=$3
    fi
    create_swap $SWAP
    release_pagecaches 3
    set_power $DISK $STATE
}

main $@

