#!/bin/bash
# 从一份 release_images 生成诊断包(只改副本, 原目录不动)。
#
# 用法:
#   mk_diag_pkg.sh SRC_DIR OUT_DIR [--bootargs "panic=0 console=tty0"] [--disable PAT]... [--inittab keep|adb|udev-adb-fb]
#
#   --bootargs  追加到 dtb 的 bootargs/bootargs1/bootargs3 末尾(recovery 用的 bootargs2 不动)
#   --disable   把 compatible 匹配 PAT 的 dts 节点置为 disabled(可多次); 需要同目录的 disable_node.awk
#   --inittab   keep        : 不改 inittab
#               adb         : rcS 换成只起 adb(不起 udev, 避免按 modalias 自动加载模块)
#               udev-adb-fb : rcS 换成 udev + adb + 往 /dev/fb0 写雪花
#               (三种都会注释掉 rcK)
#
# 环境变量: DTC(默认 dtc 或 $T/out/host/bin/dtc), PRELOADER_DTB_OFFSET(默认 49152 = 0xC000)
# dtb 尾部的 boot_misc 直接取自 SRC_DIR/ac83xx.dtb.bin 偏移 0xC000 之后的字节, 不依赖单独的 boot_misc.bin。
set -e
die(){ echo "!! $*" >&2; exit 1; }
HERE=$(cd "$(dirname "$0")" && pwd)
SRC=$1; OUT=$2; shift 2 || die "用法: $0 SRC_DIR OUT_DIR [选项]"
BOOTARGS=""; INITTAB=keep; DIS=()
while [ $# -gt 0 ]; do case "$1" in
  --bootargs) BOOTARGS=$2; shift 2;;
  --disable)  DIS+=("$2"); shift 2;;
  --inittab)  INITTAB=$2; shift 2;;
  *) die "未知参数 $1";; esac; done
OFF=${PRELOADER_DTB_OFFSET:-49152}
DTC=${DTC:-$( [ -n "$T" ] && [ -x "$T/out/host/bin/dtc" ] && echo "$T/out/host/bin/dtc" || echo dtc )}
command -v "$DTC" >/dev/null || [ -x "$DTC" ] || die "找不到 dtc ($DTC)"
[ -f "$SRC/ac83xx.dtb.bin" ] && [ -f "$SRC/system.img.ext4" ] || die "$SRC 不像 release_images 目录"
[ "$(cd "$SRC" && pwd)" != "$(mkdir -p "$OUT" && cd "$OUT" && pwd)" ] || die "OUT 不能等于 SRC"

echo "== 1) 全新副本: $OUT"
rm -rf "$OUT" && cp -a "$SRC" "$OUT"
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT

echo "== 2) dtb"
if [ -n "$BOOTARGS" ] || [ ${#DIS[@]} -gt 0 ]; then
  SZ=$(stat -c%s "$SRC/ac83xx.dtb.bin"); [ "$SZ" -gt "$OFF" ] || die "原 dtb($SZ) 不大于 $OFF, 没有拼 boot_misc?"
  tail -c +$((OFF+1)) "$SRC/ac83xx.dtb.bin" > "$W/boot_misc.bin"
  "$DTC" -q -I dtb -O dts "$SRC/ac83xx.dtb.bin" > "$W/a.dts"
  cp "$W/a.dts" "$W/b.dts"
  for p in "${DIS[@]}"; do
    awk -v PAT="$p" -f "$HERE/disable_node.awk" "$W/b.dts" > "$W/c.dts" || die "--disable '$p' 没匹配到节点"
    mv "$W/c.dts" "$W/b.dts"
  done
  if [ -n "$BOOTARGS" ]; then
    esc=$(printf '%s' "$BOOTARGS" | sed 's/[&|\\]/\\&/g')
    sed -i -E "/^[[:space:]]*bootargs[13]? = \"/s|no_console_suspend \"|no_console_suspend $esc \"|" "$W/b.dts"
    [ "$(grep -cE "^[[:space:]]*bootargs[13]? = \".*$esc \"" "$W/b.dts")" = 3 ] || die "bootargs 没改到 3 行"
  fi
  "$DTC" -q -I dts -O dtb -o "$W/new.dtb" "$W/b.dts"
  [ "$(stat -c%s "$W/new.dtb")" -lt "$OFF" ] || die "新 dtb 超过 $OFF"
  cp "$W/new.dtb" "$OUT/ac83xx.dtb.bin"
  dd if="$W/boot_misc.bin" of="$OUT/ac83xx.dtb.bin" bs=1 seek="$OFF" conv=notrunc 2>/dev/null
  cmp -s <(tail -c +$((OFF+1)) "$OUT/ac83xx.dtb.bin") "$W/boot_misc.bin" || die "boot_misc 拼接不一致"
  echo "-- dtb 改动(语义 diff):"
  diff <("$DTC" -q -I dtb -O dts "$SRC/ac83xx.dtb.bin") <("$DTC" -q -I dtb -O dts "$OUT/ac83xx.dtb.bin") | grep '^[<>]' | cut -c1-160 || true
else
  echo "-- dtb 不改"
fi

echo "== 3) inittab: $INITTAB"
I="$OUT/system.img.ext4"
if [ "$INITTAB" != keep ]; then
  debugfs -R 'cat /etc/inittab' "$I" 2>/dev/null > "$W/inittab.orig"
  grep -q '^::sysinit:/etc/init.d/rcS$' "$W/inittab.orig" || die "inittab 里没有 '::sysinit:/etc/init.d/rcS'"
  case "$INITTAB" in
    adb)         NEW='::sysinit:/etc/init.d/S30adb start';;
    udev-adb-fb) NEW='::sysinit:/etc/init.d/S10udev start\n::sysinit:/etc/init.d/S30adb start\n::sysinit:/bin/sh -c "(sleep 3; dd if=/dev/urandom of=/dev/fb0 bs=4096 count=4000 2>/dev/null) \&"';;
    *) die "未知 --inittab $INITTAB";;
  esac
  sed -e "s|^::sysinit:/etc/init.d/rcS\$|$NEW|" -e 's|^\(::shutdown:/etc/init.d/rcK\)$|#\1|' "$W/inittab.orig" > "$W/inittab.new"
  printf 'cd /etc\nrm inittab\nwrite %s inittab\nset_inode_field inittab uid 0\nset_inode_field inittab gid 0\nset_inode_field inittab mode 0100644\n' "$W/inittab.new" > "$W/dbg"
  debugfs -w -f "$W/dbg" "$I" >/dev/null 2>&1
  debugfs -R 'cat /etc/inittab' "$I" 2>/dev/null | cmp -s - "$W/inittab.new" || die "inittab 写入后内容不一致"
  debugfs -R 'stat /etc/inittab' "$I" 2>/dev/null | grep -q 'User: *0 *Group: *0' || die "inittab 属主不是 root"
  [ "$(debugfs -R 'ls /' "$I" 2>/dev/null | cat | tr -s ' ' '\n' | grep -c inittab)" = 0 ] || die "根目录出现异常 inittab 条目"
  e2fsck -fn "$I" 2>&1 | grep -qiE 'illegal|error|fix' && die "e2fsck 报错"
  echo "-- inittab 改动:"; diff "$W/inittab.orig" "$W/inittab.new" | grep '^[<>]' || true
fi

echo "== 4) 校验值(拷到 Windows 后用 certutil -hashfile <文件> MD5 核对)"
( cd "$OUT" && md5sum Image.bin ac83xx.dtb.bin system.img.ext4 )
echo "烧录目录: $(cd "$OUT" && pwd)"
