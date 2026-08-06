#!/bin/bash


# tmp for generator auto_version.h

git log -1 > log.tmp

cmt_line=
ator_line=
ct_line=

echo "For $1 project"

mkdir -p ${ARM2OBJDIR}/inc/
HEADFILE=${ARM2OBJDIR}/inc/auto_version.h

while read line; do
    if [ ! -z "`echo $line | grep "commit"`" ]; then
        cmt_line=$line
    elif [ ! -z "`echo $line | grep "Author:"`" ]; then
        ator_line=$line
    elif [ ! -z "`echo $line | grep "Date:"`" ]; then
        ct_line=$line
    fi
done < log.tmp

commitid=`echo $cmt_line | awk -F ' ' '{print $2}'`
author=`echo $ator_line | awk -F ':' '{print $2}'`
change=`echo $ct_line | awk -F ':' '{print $2}'`
rm -f log.tmp

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

mkdir -p ${ARM2OBJDIR}/inc/generated
HEADFILE=${ARM2OBJDIR}/inc/generated/atc_project.h

if [ $1 = "android-ac823x" ]; then
    platform=ac823x
    os=android
    version=m
    board=ac823x_evb
    proj=ac23x_evb
elif [ $1 = "android-ac823x-adas" ]; then
    platform=ac823x
    os=android
    version=m
    board=ac823x_evb
    proj=ac823x_adas
elif [ $1 = "android-ac83xx" ]; then
    platform=ac83xx
    os=android
    version=m
    board=ac83xx_evb
elif [ $1 = "linux-ac83xx" ]; then
    platform=ac83xx
    os=linux
    version=dizzy   
    board=ac83xx_evb 
else
    echo "project must be confirm"
    exit 1
fi

cat<<EOF>$HEADFILE
/*Note, this header file was generated automaticlly, so don't revise it by yourself.
#
#  Generate atc project.h log
*/

#ifndef __ATC_PROJECT_H__
#define __ATC_PROJECT_H__



#define CONFIG_ATC_OS_$os
#define CONFIG_ATC_OS_VER_$version
#define CONFIG_ATC_PLATFORM_$platform
#define CONFIG_ATC_BOARD_$board
#define CONFIG_ATC_PRJ_$proj

#endif // __ATC_PROJECT_H__

EOF







