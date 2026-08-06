#!/bin/bash

if [ -z $1 ];then
	echo "Need have a folder/file name"
	exit 1
fi

MY_RKEY=`date +%Y%m%d_%H%M%S_%N_$RANDOM`

for i in $*; do
	if [ -e $i ]; then
		MY_DEL=`echo $i.$MY_RKEY|sed -e 's/\//_/g'`
		mv $i $MY_DEL
		rm -rf $MY_DEL &
	fi
	DIRNAME=`dirname $i`
	(rmdir --ignore-fail-on-non-empty -p $DIRNAME 2>/dev/null 1>/dev/null; exit 0)
done
