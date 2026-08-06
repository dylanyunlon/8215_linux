##
## @version: 0.2 
## @author: shouzhi chen
## @description:  The purpose of this script is try to check out from p4 server, 
##                and start to build image automatically after going through 
##                a simple setup 
##

### Configuration for P4 Client
export P4PORT=10.17.1.173:3018
export ROOT=//CNB

#### configure following items according to your working environment
#### TODO ####
export SYNC_FOLDER=MT3360/DEV_BR/android/android4.2.2_pm
export WORKSPACE_ROOT=/proj/mtk68037/workspace/TestP4
export P4CLIENT=WS_shouzhi.chen_testp4
export P4USER=shouzhi.chen
export P4PASSWD=*********
#TODO end

export P4CHARSET=utf8
### End 

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
function usage() {
    color_print -blue "Usage: restart_clean_build.sh Changelist#"
    color_print -blue "       # If there is no changelist, sync code to latest by default"
    color_print -blue "Note that before running this scripts, please configure your build environment items which "
    color_print -blue "marked by #TODO"
}

# Check needed arguments
function check_para() {
	(( ${#P4CLIENT} > 0 )) || color_print -red "P4CLIENT is not set"
	(( ${#P4USER} > 0 )) || color_print -red "P4USER is not set"
	(( ${#P4PASSWD} > 0 )) || color_print -red "P4PASSWD is not set"
	(( ${#SYNC_FOLDER} > 0 )) || color_print -red "SYNC_FOLDER is not set"
	(( ${#WORKSPACE_ROOT} > 0 )) || color_print -red "WORKSPACE_ROOT is not set"
}

# delete src files
function delete_src() {

    local ANDROIDQ
    if [ ! -z `which androidq` ];then
	echo "androidq ok"
  	ANDROIDQ=androidq
    else
 	echo "no androidq"
    	ANDROIDQ=
    fi

    if [ -e $WORKSPACE_ROOT ] 
    then
        $ANDROIDQ rm -rf $WORKSPACE_ROOT/$SYNC_FOLDER 2>&1
	# clean log file
        rm -rf $WORKSPACE_ROOT/delete.log
        rm -rf $WORKSPACE_ROOT/sync.log
        rm -rf $WORKSPACE_ROOT/build.log
	color_print -green "Enter your workspace and source get deleted!"
    else
        color_print -red "The Root of your workspace is not existed!"
    fi
    return 0 
}

# force sync specified code path and changelist
function sync_src() {
    local CHANGELIST=$1
    if [ "$CHANGELIST" != "" ] 
    then
        color_print -green "Start to sync code base @$CHANGELIST"
        p4 sync -f $ROOT/$SYNC_FOLDER/...@$CHANGELIST
    else
        color_print -green "Start to sync code base to lastest"
	p4 sync -f $ROOT/$SYNC_FOLDER/...#head
    fi	    
    return 0 
}

# start build 
function start_build() {
	if [ -e $WORKSPACE_ROOT/$SYNC_FOLDER/selfbuild ] 
	then
		cd $WORKSPACE_ROOT/$SYNC_FOLDER && ./selfbuild 
	else
		color_print -red "Build script file is not existed!"
	fi
    	return 0 
}

function main() {
	usage
	local ret=`check_para`
	echo $ret
	if [ ! x"$ret" = x ] 
	then
		## parameters are not all set	
		color_print -red "This script is going to exit due to parameter not right"
	        exit 1
	fi
        color_print -green "Start to delete old code base"
	delete_src >$WORKSPACE_ROOT/delete.log
        color_print -green "Code is deleted"
	if [ "$1" == "" ]
	then
            color_print -green "Start to sync code base to lastest"
	else
            color_print -green "Start to sync code base CHANGELIST $1"
	fi
	sync_src $1 >$WORKSPACE_ROOT/sync.log
	color_print -green "Sync code base done"
	color_print -green "Start to build"
	start_build >$WORKSPACE_ROOT/build.log
	color_print -green "Build image done"
}

main $@
