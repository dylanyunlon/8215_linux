#!/bin/sh
# 后台运行 hcn_application，脱离 adb 会话：PC 睡眠/断开 adb 都不会影响它（输出写板子上的文件，不走管道到 PC）。
# 启动:   adb shell "nohup sh /tmp/run_app_bg.sh >/dev/null 2>&1 &"        (没有 nohup 就换 setsid)
# 指定程序: adb shell "nohup sh /tmp/run_app_bg.sh /usr/bin/hcn_application >/dev/null 2>&1 &"
# 取回日志: adb pull /tmp/app_night.log ; adb pull /tmp/app_night.log.1
# 日志按行数轮转(每 3000 行换一个文件，不 fork)，最多占约 600KB 内存盘；每行前带开机秒数，可与 monitor.sh 的日志对齐。
BIN=${1:-/usr/bin/hcn_application}
LOG=${APP_LOG:-/tmp/app_night.log}
[ -x "$BIN" ] || { echo "找不到或不可执行: $BIN" > "$LOG"; exit 1; }
killall hcn_application hcn_application_new 2>/dev/null; sleep 1
cd /
rm -f "$LOG" "$LOG.1" /tmp/app.rc
echo "==== start $(date) bin=$BIN md5=$(md5sum "$BIN" | cut -d' ' -f1) pid_of_this_script=$$" >> "$LOG"
N=0
( LD_LIBRARY_PATH=/usr/lib/hcn_application "$BIN" 2>&1; echo "$?" > /tmp/app.rc ) | \
while IFS= read -r L; do
    read UP _ < /proc/uptime
    echo "[$UP] $L" >> "$LOG"
    N=$((N+1))
    if [ "$N" -ge 3000 ]; then mv -f "$LOG" "$LOG.1"; N=0; fi
done
RC=$(cat /tmp/app.rc 2>/dev/null)
read UP _ < /proc/uptime
{
  echo "[$UP] ==== APP EXITED  code=${RC:-?}  (137=SIGKILL, 139=SIGSEGV, 141=SIGPIPE, 129=SIGHUP, 0=正常退出)  $(date)"
  echo "---- meminfo"; grep -E 'MemTotal|MemFree|MemAvailable|^Cached' /proc/meminfo
  echo "---- dmesg tail"; dmesg | tail -80
  echo "---- /tmp/aee"; ls -l /tmp/aee 2>&1
} >> "$LOG"
