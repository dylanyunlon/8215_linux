#!/bin/bash
# DA 树内核的诊断改动: 一键打上 / 一键还原。
#
# 用法: T=/work2/tanyunlong/dylan/AC8215_DA D=~/dylan bash kernel_diag.sh apply|restore|status
#
# apply 做三件事(原件先备份到 $D, 已备份则不覆盖):
#   1) defconfig: 开 fbcon(FRAMEBUFFER_CONSOLE/VT_CONSOLE/FONTS/FONT_8x16), PANIC_TIMEOUT=0
#   2) defconfig: 与 AWTK(NAND 板可启动)对齐 —— VOUT/VCP/MALI/UMP/BACKLIGHT 改模块, FB1/SCO/COMBO 关;
#      FB0_ATC 与 ATC_METAZONE 必须保持内置(bootanidrv 是 obj-y, 直接调用 fb/osd/pmx/lcd 与 metazone 的函数)
#   3) displayinit.c: 用 patch_osd.awk 在 osd_init() 里关闭开机动画平面, 让 fbcon 可见
# 注意: ATC_COMBO 由 build/setup 按"树里有无 WiFi 源码"导出 ATC_WIFI_CHIP=AIC8800 后被 select 进来,
#       defconfig 关不掉; 本脚本不改 setup(改了会让 lvgl_home 依赖不存在的 androidauto 包)。
# apply/restore 之后都要 rm -rf $T/out/build/ac83xx/linux-local 再整编, 否则 buildroot 不会重读 defconfig。
set -e
T=${T:-/work2/tanyunlong/dylan/AC8215_DA}; D=${D:-$HOME/dylan}
HERE=$(cd "$(dirname "$0")" && pwd)
KC=$T/source/kernel/kernel-3.18/arch/arm/configs/ac83xx_defconfig
F=$T/source/kernel/kernel-3.18/drivers/video/fbdev/atc/ac83xx/displayinit.c
KC_BAK=$D/ac83xx_kernel_defconfig.orig; F_BAK=$D/displayinit.c.orig
die(){ echo "!! $*" >&2; exit 1; }
[ -f "$KC" ] && [ -f "$F" ] || die "T=$T 下找不到内核 defconfig 或 displayinit.c"
SYMS='FRAMEBUFFER_CONSOLE|FRAMEBUFFER_CONSOLE_DETECT_PRIMARY|VT_CONSOLE|FONTS|FONT_8x16|PANIC_TIMEOUT|ATC_COMBO|ATC_METAZONE|BACKLIGHT_ATC|FB0_ATC|FB1_ATC|MALI400|UMP|VCP_ATC|VIDEO_ATC_VOUT|SND_AIC_UART_SCO'

case "$1" in
apply)
  mkdir -p "$D"; [ -f "$KC_BAK" ] || cp "$KC" "$KC_BAK"; [ -f "$F_BAK" ] || cp "$F" "$F_BAK"
  grep -q 'DIAG: hide' "$KC_BAK" "$F_BAK" && die "备份里已含诊断改动, 备份不是原件, 停下"
  sed -E "/^(# )?CONFIG_($SYMS)[ =]/d" "$KC_BAK" > "$KC.tmp"
  cat >> "$KC.tmp" <<'EOF'
CONFIG_FRAMEBUFFER_CONSOLE=y
CONFIG_FRAMEBUFFER_CONSOLE_DETECT_PRIMARY=y
CONFIG_VT_CONSOLE=y
CONFIG_FONTS=y
CONFIG_FONT_8x16=y
CONFIG_PANIC_TIMEOUT=0
# CONFIG_ATC_COMBO is not set
CONFIG_ATC_METAZONE=y
CONFIG_BACKLIGHT_ATC=m
CONFIG_FB0_ATC=y
# CONFIG_FB1_ATC is not set
CONFIG_MALI400=m
CONFIG_UMP=m
CONFIG_VCP_ATC=m
CONFIG_VIDEO_ATC_VOUT=m
# CONFIG_SND_AIC_UART_SCO is not set
EOF
  mv "$KC.tmp" "$KC"
  awk -f "$HERE/patch_osd.awk" "$F_BAK" > "$F.tmp" || { rm -f "$F.tmp"; die "patch_osd.awk 失败"; }
  mv "$F.tmp" "$F"
  echo "已打上诊断改动。下一步: rm -rf $T/out/build/ac83xx/linux-local 后整编";;
restore)
  [ -f "$KC_BAK" ] && [ -f "$F_BAK" ] || die "缺备份 $KC_BAK 或 $F_BAK"
  cp "$KC_BAK" "$KC"; cp "$F_BAK" "$F"
  echo "已还原内核 defconfig 和 displayinit.c。下一步: rm -rf $T/out/build/ac83xx/linux-local 后整编";;
status)
  [ -f "$KC_BAK" ] && [ -f "$F_BAK" ] || { echo "还没有备份(没执行过 apply), 当前即原件"; exit 0; }
  cmp -s "$KC" "$KC_BAK" && echo "defconfig: 原样" || { echo "defconfig: 有改动"; diff "$KC_BAK" "$KC" | grep '^[<>]' || true; }
  cmp -s "$F" "$F_BAK" && echo "displayinit.c: 原样" || echo "displayinit.c: 有改动 ($(grep -c 'DIAG: hide' "$F") 处 DIAG)";;
*) die "用法: $0 apply|restore|status";;
esac
