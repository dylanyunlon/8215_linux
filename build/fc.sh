#!/bin/sh
# This script is a helper script to help convert non-kernel coding style
# source code to kernel coding style.
#
# !!!NOTE!!!
# This script is only intend to run only once on each file. The script 
# might output undesire output. It is not advise to use this script as a
# mean to skip proper editor setting and coding habit.
#
#
CUR_DIR=`pwd`

echo "${DA_TOP}/build/fmtcm -f ${CUR_DIR}/$1"

${DA_TOP}/build/astyle --options=${DA_TOP}/build/astyle1.conf ${CUR_DIR}/$1
${DA_TOP}/build/fmtcm -f "${CUR_DIR}/$1"
