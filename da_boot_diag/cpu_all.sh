#!/bin/bash
# 用法: bash cpu_all.sh [采样秒数，默认 3]
# 采样两次 /proc/stat，算出每个逻辑 CPU 在这段时间内的忙碌百分比（iowait 算空闲），
# 输出: 整机平均、各档位 CPU 数、最忙的 8 个、全部 CPU 的小格子图。
SEC=${1:-3}
A=$(mktemp); B=$(mktemp)
grep '^cpu[0-9]' /proc/stat > $A; sleep "$SEC"; grep '^cpu[0-9]' /proc/stat > $B
paste -d' ' $A $B | awk -v sec="$SEC" '
{ # 前 11 列是第一次: name user nice system idle iowait irq softirq steal ... ; 后 11 列是第二次
  n=$1; sub("cpu","",n)
  t1=$2+$3+$4+$5+$6+$7+$8+$9; i1=$5+$6
  t2=$13+$14+$15+$16+$17+$18+$19+$20; i2=$16+$17
  dt=t2-t1; b=(dt>0)? 100*((t2-t1)-(i2-i1))/dt : 0
  cpu[n]=b; sum+=b; cnt++
  if (b<10) lo++; else if (b<50) mid++; else if (b<90) hi++; else full++
}
END {
  printf "共 %d 个逻辑 CPU，采样 %d 秒\n", cnt, sec
  printf "整机平均占用: %.1f%%   (相当于 %.1f 个核心在忙，满载是 %d 个)\n", sum/cnt, sum/100, cnt
  printf "分档: <10%% 的 %d 个 | 10~50%% 的 %d 个 | 50~90%% 的 %d 个 | >90%% 的 %d 个\n\n", lo, mid, hi, full
  # 最忙的 8 个
  for (k in cpu) printf "%.1f %s\n", cpu[k], k | "sort -rn | head -8 > /tmp/.cpu_top8"
  close("sort -rn | head -8 > /tmp/.cpu_top8")
  printf "最忙的 8 个:"
  while ((getline l < "/tmp/.cpu_top8") > 0) { split(l, a, " "); printf "  CPU%s=%s%%", a[2], a[1] }
  printf "\n\n全部 CPU（数字是忙碌百分比，每行 18 个）:\n"
  for (i=0; i<cnt; i++) { printf "%3d ", cpu[i]; if ((i+1)%18==0) printf "\n" }
  if (cnt%18) printf "\n"
}'
rm -f $A $B /tmp/.cpu_top8
