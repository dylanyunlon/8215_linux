#!/bin/sh -
# $RCSfile: check_cs.sh $
# $Revision: #4 $
# $Date: 2015/08/19 $
# $Author: zeng.zhang $
#
# Description:
#         script to check kernel code style 
#=========================================================================
CUR_DIR=`pwd`
CFILES=*.c
chmod 0777 ${SCRIPTS_PATH}/checkpatch.pl

if [ ".$2" != "." ]
then
CLOG=$2
else
CLOG=cs.txt
fi

if [ ".$1" != "." ]
then
CFILES=$1
fi

echo "check : ${CUR_DIR}/${CFILES} output to ${CLOG}  "

${SCRIPTS_PATH}/checkpatch.pl --show-types --no-tree --max-line-length=120 --ignore MEMORY_BARRIER,NEW_TYPEDEFS,VOLATILE,BRACES -f  ${CUR_DIR}/${CFILES} >$CLOG

