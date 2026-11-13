#!/bin/bash
# 手工快速编内核（不走 allmake），在 $T/out/build/xt_<NAME>/linux-local 里编，不动原树。
# 用法: xtool_build.sh NAME SRC_BUILD_DIR CROSS_PREFIX [ROOT]
#   NAME           变体名，产物目录 $T/out/build/xt_<NAME>
#   SRC_BUILD_DIR  已编好的内核构建目录（如 $T/out/build/ac83xx/linux-local），整个拷贝一份
#   CROSS_PREFIX   工具链前缀，如 $T/out/host/bin/arm-buildroot-linux-musleabihf-
#   ROOT           预编译库（prebuild/）所在树根，默认 $T；可指向"假根"来换库（见 DIAG_LOG_2026-11-12.md）
# 必须带的环境变量（脚本里已设）。缺了不会报错，而是静默编出另一个内核：
#   AC83XX_BOOT_DEVICE=nand   drivers/Kconfig 用 option env 读它；缺了 MTD/NAND/JFFS2 整支被砍
#   TOPDIR=<ROOT>/buildroot   atc-ftl 等 Makefile 用 $(TOPDIR)/../prebuild/... 取预编译库
# 编完检查 .config 是否漂移（应为 0 行）并打印 zImage 大小。
set -e
T=${T:-/work2/tanyunlong/dylan/AC8215_DA}
NAME=$1; SRCB=$2; CC=$3; ROOT=${4:-$T}
[ -n "$NAME" ] && [ -d "$SRCB" ] && [ -n "$CC" ] || { echo "用法: $0 NAME SRC_BUILD_DIR CROSS_PREFIX [ROOT]"; exit 1; }
export AC83XX_BOOT_DEVICE=nand TOPDIR=$ROOT/buildroot
P=$T/out/build/xt_$NAME; W=$P/linux-local
rm -rf "$P" && mkdir -p "$P" && cp -a "$SRCB" "$W"
cd "$W"
cp .config /tmp/config_before_$NAME
make ARCH=arm CROSS_COMPILE=$CC clean
make -j"$(nproc)" ARCH=arm CROSS_COMPILE=$CC zImage
diff <(grep -vE '^(#|$)' /tmp/config_before_$NAME | sort) <(grep -vE '^(#|$)' .config | sort) > /tmp/config_drift_$NAME.txt || true
echo "config 漂移行数: $(wc -l < /tmp/config_drift_$NAME.txt)（应为 0）"
ls -l arch/arm/boot/zImage
