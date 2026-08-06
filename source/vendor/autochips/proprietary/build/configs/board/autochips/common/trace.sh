#!/bin/sh

TDIR=/sys/kernel/debug/tracing

echo 3 > /proc/sys/vm/drop_caches
echo 16384 > $TDIR/buffer_size_kb
cat $TDIR/buffer_size_kb

echo nop > $TDIR/current_tracer
#echo record-cmd > $TDIR/trace_options
#echo print-tgid > $TDIR/trace_options


echo 'sched_switch sched_wakeup sched_wakeup_new sched_migrate_task' >> $TDIR/set_event
echo 'irq_handler_entry irq_handler_exit' >> $TDIR/set_event
echo 'softirq_raise softirq_entry softirq_exit' >> $TDIR/set_event
echo 'cpu_frequency' >> $TDIR/set_event

#Clear
echo 0 > $TDIR/tracing_on
echo > $TDIR/trace

#Start trace
read -p "###### Press any key to start #####"
echo 1 > $TDIR/tracing_on

echo "Tracing ..."

#Stop trace
read -p "###### Press any key to stop #####"
echo 0 > $TDIR/tracing_on

#Capture trace
#Directly cat may stuck in sendfile()
cat -n $TDIR/trace | sed -e 's/.\{7\}//' > trace.log

