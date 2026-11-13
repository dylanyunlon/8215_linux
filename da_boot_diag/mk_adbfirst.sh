#!/bin/bash
# 用法: mk_adbfirst.sh SRC_DIR OUT_DIR
# 作用: 保留完整启动流程, 但让 adb 在 rcS 之前最先启动, 并让 rcS 跳过后面的 S30adb(避免 gadget 被配置两次)
set -e
SRC=$1; OUT=$2
die(){ echo "!! $*"; exit 1; }
[ -d "$SRC" ] && [ -n "$OUT" ] || die "用法: $0 SRC_DIR OUT_DIR"
rm -rf "$OUT"; cp -a "$SRC" "$OUT"
I="$OUT/system.img.ext4"
W=$(mktemp -d)
debugfs -R 'cat /etc/inittab' "$I" 2>/dev/null > "$W/inittab.orig"
grep -q '^::sysinit:/etc/init.d/rcS$' "$W/inittab.orig" || die "inittab 里没有 rcS 行"
sed 's|^::sysinit:/etc/init.d/rcS$|::sysinit:/etc/init.d/S30adb start\n::sysinit:/etc/init.d/rcS|' "$W/inittab.orig" > "$W/inittab.new"
debugfs -R 'cat /etc/init.d/rcS' "$I" 2>/dev/null > "$W/rcS.orig"
grep -q '\[ ! -f "\$i" \] && continue' "$W/rcS.orig" || die "rcS 里没找到锚点行"
awk '{print} /\[ ! -f "\$i" \] && continue/ && !d {print "     [ \"$i\" = /etc/init.d/S30adb ] && continue"; d=1}' "$W/rcS.orig" > "$W/rcS.new"
sh -n "$W/rcS.new" || die "新 rcS 语法错误"
printf 'cd /etc\nrm inittab\nwrite %s inittab\nset_inode_field inittab uid 0\nset_inode_field inittab gid 0\nset_inode_field inittab mode 0100644\ncd /etc/init.d\nrm rcS\nwrite %s rcS\nset_inode_field rcS uid 0\nset_inode_field rcS gid 0\nset_inode_field rcS mode 0100755\n' "$W/inittab.new" "$W/rcS.new" > "$W/dbg"
debugfs -w -f "$W/dbg" "$I" >/dev/null 2>&1
debugfs -R 'cat /etc/inittab' "$I" 2>/dev/null | cmp -s - "$W/inittab.new" || die "inittab 写入后内容不一致"
debugfs -R 'cat /etc/init.d/rcS' "$I" 2>/dev/null | cmp -s - "$W/rcS.new" || die "rcS 写入后内容不一致"
for f in /etc/inittab /etc/init.d/rcS; do debugfs -R "stat $f" "$I" 2>/dev/null | grep -q 'User: *0 *Group: *0' || die "$f 属主不是 root"; done
echo "-- inittab 改动:"; diff "$W/inittab.orig" "$W/inittab.new" | grep '^[<>]' || true
echo "-- rcS 改动:";     diff "$W/rcS.orig" "$W/rcS.new" | grep '^[<>]' || true
e2fsck -fn "$I" 2>&1 | tail -1
echo "== 校验值"; (cd "$OUT" && md5sum Image.bin ac83xx.dtb.bin system.img.ext4)
echo "烧录目录: $OUT"
