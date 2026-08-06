#!/bin/sh
# This script is a helper script to help convert non-kernel coding style
# source code to kernel coding style.
#
# !!!NOTE!!!
# This script is only intend to run only once on each file. The script 
# might output undesire output. It is not advise to use this script as a
# mean to skip proper editor setting and coding habit.
#


usage()
{
	echo 'cv.sh [-p kernel/scripts][file1] [file2] ...'
	echo "${SCRIPTS_PATH}"
	exit 0
}

if [ ".$1" == "." ]; then
	usage
fi

if [ ! -e $SCRIPTS_PATH/checkpatch.pl ]; then
	SCRIPTS_PATH=kernel/scripts
fi

if [ ! -e $SCRIPTS_PATH/checkpatch.pl ]; then
	SCRIPTS_PATH=kernel-3.18/scripts
fi

if [ ! -e $SCRIPTS_PATH/checkpatch.pl ]; then
	SCRIPTS_PATH=kernel-3.10/scripts
fi

if [ $1 == "-p" ]; then
	SCRIPTS_PATH=$2
	shift 2
fi

if [ ! -e $SCRIPTS_PATH/checkpatch.pl ]; then
	echo Kernel scripts checkpatch.pl not found. Stopped.
	exit 1
fi

if [ ! -e $SCRIPTS_PATH/cleanfile ]; then
	echo Kernel scripts cleanfile not found. Stopped.
	exit 1
fi

for i in $*; do
	if [ ! -e $i ]; then
		echo $i Not found, skip.
		continue
	fi

	filename=$(basename $i)
	extension="${filename##*.}"
	if [ $extension == "c" -o $extension == "h" ]; then
		# Indent all c source.
		indent -l100 -linux -nsob -il0 $i
		rm $i~

		# Process the empty line before EXPORT_();
		gawk '/^}/{T=1;}                \
		        { if ($0~/^$/) {          \
		             if (T==1) T=2; else { if (T==2) print; T=0; print}} \
		        else {                  \
		             if ($0 ~ /^EXPORT_/ || $0 ~ /^[a-z]*_init/ || $0 ~ /^module_exit/) { print ; T = 0;}    \
		             else {if (T==2) {printf("\n"); T=0}  print;}}}' $i > $i.n
		mv $i.n $i

		# Check level correct. The last } must be on first column
		gawk ' /^[ \t]+}/{if (CONT==0) T=1;}/^}/ \
		        {T=0} {CONT=0;}/\\$/{CONT=1;} \
		        END {if (T==1) printf("Please check %s for error!\n", FILENAME)}' $i
	fi

	# auto fix with checkpatch
	$SCRIPTS_PATH/checkpatch.pl --no-tree --max-line-length=120 --fix -f $i > /dev/null
	if [ -e $i.EXPERIMENTAL-checkpatch-fixes ]; then
		mv $i.EXPERIMENTAL-checkpatch-fixes $i
	fi

	# auto fix space issue, apply cleanfile
	$SCRIPTS_PATH/cleanfile -width 120 $i
done
