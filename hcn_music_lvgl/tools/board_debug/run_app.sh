#!/bin/sh
# 用法: sh /tmp/run_app.sh [可执行文件，默认 /tmp/hcn_application_new]
# 前台运行 hcn_application；每行输出前加开机后的秒数（读 /proc/uptime，不 fork，不影响时序）；
# 退出后打印退出码、内存、dmesg 末尾。
BIN=${1:-/tmp/hcn_application_new}
if [ ! -x "$BIN" ]; then
    echo "找不到或不可执行: $BIN  (/tmp 重启会清空，需要重新 adb push 并 chmod +x)"
    exit 1
fi
echo "==== md5: $(md5sum "$BIN")"
killall hcn_application 2>/dev/null
killall hcn_application_new 2>/dev/null
sleep 1
cd /
rm -f /tmp/app.rc
echo "==== start $(date) bin=$BIN"
( LD_LIBRARY_PATH=/usr/lib/hcn_application "$BIN" 2>&1; echo "$?" > /tmp/app.rc ) | \
while IFS= read -r L; do read UP _ < /proc/uptime; echo "[$UP] $L"; done
RC=$(cat /tmp/app.rc 2>/dev/null)
echo "==== exit code=${RC:-?}  (137=被 SIGKILL, 139=段错误, 134=abort)  $(date)"
echo "==== meminfo at exit"
grep -E 'MemTotal|MemFree|MemAvailable|^Cached|Shmem|Slab' /proc/meminfo
echo "==== dmesg tail"
dmesg | tail -60
