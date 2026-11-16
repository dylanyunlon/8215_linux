#!/bin/sh
# 用法: sh /tmp/monitor.sh [进程名，默认自动找 hcn_application_new / hcn_application]
# 每 2 秒一行: 时间 系统CPU% 进程CPU% 前三个最忙线程 进程RSS 系统free/avail/cached/shmem/slab /tmp占用
# 百分比以"单个核"为 100% 计算线程和进程，系统CPU是全部核的平均。
# 线程后面括号 (uX+kY) = 用户态 X% + 内核态 Y%: u 高=我们的代码/LVGL 绘制；k 高=显存拷贝/U盘读/驱动调用。
# oth=[...] = 应用以外最忙的三个进程(含内核线程)，用来找出"CPU 去哪了"。
# 输出只含 ASCII，避免 Windows 终端乱码。
NAME=$1
NCPU=$(grep -c '^processor' /proc/cpuinfo); [ "$NCPU" -ge 1 ] 2>/dev/null || NCPU=1
TMPF=/tmp/mon.$$
getpid() {
    if [ -n "$NAME" ]; then P=$(pidof "$NAME"); else
        P=$(pidof hcn_application_new); [ -z "$P" ] && P=$(pidof hcn_application)
    fi
    P=${P%% *}
}
memk() { grep "^$1:" /proc/meminfo | sed 's/[^0-9]//g'; }
read CPU U N S I W IRQ SIRQ REST < /proc/stat
PREV_TOT=$((U+N+S+I+W+IRQ+SIRQ)); PREV_IDLE=$((I+W)); PREV_APP=0; LASTP=
while true; do
    sleep 2
    getpid
    read CPU U N S I W IRQ SIRQ REST < /proc/stat
    TOT=$((U+N+S+I+W+IRQ+SIRQ)); IDLE=$((I+W))
    DT=$((TOT-PREV_TOT)); [ "$DT" -le 0 ] && DT=1
    SYS=$(( (DT-(IDLE-PREV_IDLE))*100/DT ))
    PREV_TOT=$TOT; PREV_IDLE=$IDLE
    PERCORE=$((DT/NCPU)); [ "$PERCORE" -le 0 ] && PERCORE=1   # 单核口径: 总节拍/核数
    APP=-; RSS=-; TOP=-
    if [ -n "$P" ] && [ -r /proc/$P/stat ]; then
        if [ "$P" != "$LASTP" ]; then PREV_APP=0; LASTP=$P; fi
        set -- $(sed 's/^.*) //' /proc/$P/stat); CUR=$(( ${12} + ${13} ))
        # 单核口径: 进程耗时 / (总节拍/核数)
        PERCORE=$((DT/NCPU)); [ "$PERCORE" -le 0 ] && PERCORE=1
        if [ "$PREV_APP" -gt 0 ]; then APP=$(( (CUR-PREV_APP)*100/PERCORE )); else APP=0; fi
        PREV_APP=$CUR
        RSS=$(grep VmRSS /proc/$P/status | sed 's/[^0-9]//g')
        : > $TMPF
        for D in /proc/$P/task/*; do
            T=${D##*/}; [ -r $D/stat ] || continue
            set -- $(sed 's/^.*) //' $D/stat); TU=${12}; TS=${13}; TC=$((TU+TS))
            eval "TP=\${TPREV_$T:-$TC}; UP0=\${TPU_$T:-$TU}; SP0=\${TPS_$T:-$TS}"
            eval "TPREV_$T=$TC; TPU_$T=$TU; TPS_$T=$TS"
            TD=$(( (TC-TP)*100/PERCORE )); UD=$(( (TU-UP0)*100/PERCORE )); SD=$(( (TS-SP0)*100/PERCORE ))
            [ "$TD" -gt 0 ] && echo "$TD $UD $SD $(cat $D/comm 2>/dev/null)-$T" >> $TMPF
        done
        TOP=$(sort -rn $TMPF | head -3 | sed 's/^\([0-9]*\) \([0-9]*\) \([0-9]*\) \(.*\)$/\4:\1%(u\2+k\3)/' | tr '\n' ' ')
        [ -z "$TOP" ] && TOP=idle
    fi
    # 系统里其它进程(含内核线程)里最忙的三个——只用 read 内建读 /proc，不 fork，不给板子添负担
    : > $TMPF.o
    for Q in /proc/[0-9]*; do
        QP=${Q##*/}; [ "$QP" = "$P" ] && continue
        read -r QL < $Q/stat 2>/dev/null || continue
        QN=${QL#*(}; QN=${QN%%)*}; QR=${QL##*) }
        set -- $QR; QT=$(( ${12} + ${13} ))
        eval "QV=\${OPREV_$QP:-$QT}"; eval "OPREV_$QP=$QT"
        QD=$(( (QT-QV)*100/${PERCORE:-100} ))
        [ "$QD" -gt 0 ] && echo "$QD $QN-$QP" >> $TMPF.o
    done
    OTH=$(sort -rn $TMPF.o | head -3 | sed 's/^\([0-9]*\) \(.*\)$/\2:\1%/' | tr '\n' ' ')
    [ -z "$OTH" ] && OTH=none
    TMPUSE=$(df -k /tmp 2>/dev/null | tail -1 | sed 's/  */ /g' | cut -d' ' -f3)
    read UP _ < /proc/uptime
    echo "[$UP] $(date +%T) sys=${SYS}% app=${APP}% [$TOP] oth=[$OTH] pid=${P:--} rssKB=$RSS freeKB=$(memk MemFree) availKB=$(memk MemAvailable) cachedKB=$(memk Cached) shmemKB=$(memk Shmem) slabKB=$(memk Slab) tmpKB=${TMPUSE:--}"
done
