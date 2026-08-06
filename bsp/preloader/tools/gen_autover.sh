#!/bin/bash


# tmp for generator auto_version.h
pl_dir=${TOPDIR}/../bsp/preloader
temp_log=${TOPDIR}/../out/log.tmp
(cd ${pl_dir} && git log -1 > ${temp_log})

cmt_line=
ator_line=
ct_line=

HEADFILE=$1/auto_version.h

while read line; do
    if [ ! -z "`echo $line | grep "^commit"`" ]; then
        cmt_line=$line
    elif [ ! -z "`echo $line | grep "^Author:"`" ]; then
        ator_line=$line
    elif [ ! -z "`echo $line | grep "^Date:"`" ]; then
        ct_line=$line
    fi
done < ${temp_log}

commitid=`echo $cmt_line | awk -F ' ' '{print $2}'`
author=`echo $ator_line | awk -F ':' '{print $2}'`
change=`echo $ct_line | awk -F ':' '{print $2}'`
#rm -f ${temp_log}

cat<<EOF>$HEADFILE
/*Note, this header file was generated automaticlly, so don't revise it by yourself.
#
#  Generate commit id by auto and print at the first preloader log
*/

#ifndef __AUTO_VERSION__
#define __AUTO_VERSION__

#define PRELOADER_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))

#define AUTO_VERSION PRELOADER_VERSION(1,4,12)   //

#define AUTO_BUILD_DATE "2010/02/25/01:00:29"

#define COMMITID          "$commitid"
#define LAST_CHANGE_TIME  "$change"
#endif // __AUTO_VERSION__

EOF

