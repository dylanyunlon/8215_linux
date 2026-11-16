#!/bin/bash
# cmp_126_vs_remote.sh — 对比 126 上的 DA 树与远程仓库(main_lvgl)，列出远程缺少/不一致的文件
#
# 126 没有公网，拿不到远程，所以分两步：
#   步骤 1 (在 126):     ./cmp_126_vs_remote.sh manifest [TREE] > ~/dylan/manifest_126.tsv
#   步骤 2 (有仓库的机器): ./cmp_126_vs_remote.sh compare ~/dylan/manifest_126.tsv /path/to/8215_linux [OUTDIR]
#   步骤 3 (回 126 打包):  ./cmp_126_vs_remote.sh pack TREE OUTDIR/push.list [out.tgz]
# 126 上若也有一份仓库副本，可一步完成:
#                        ./cmp_126_vs_remote.sh both TREE /path/to/8215_linux [OUTDIR]
#
# 也可以把 manifest_126.tsv 直接发给我，我在我这边对着远程仓库比。
#
# 对比结果分类:
#   MISSING  远程没有这个文件        -> 需要推
#   DIFF     远程有，但内容不同      -> 需要推(或确认谁是新的)
#   SAME     完全一致
#   SKIP-BIN 闭源/二进制(.a .so .o .bin .img、prebuild/atc_ftl) -> 只记 MD5，不建议推
#   SKIP-BIG 大于 MAXSZ(默认 5MB)    -> 不进打包清单
set -u
MAXSZ=${MAXSZ:-5242880}

# 要核对的范围（相对 DA 树根目录）
SCOPE=(
  source/packages/application/musicplayer
  source/packages/application/lvgl_home
  source/packages/application/home
  source/vendor/hcn/application
  buildroot/configs
  prebuild/atc_ftl
)
# 按名字查找的零散文件（内核改动、defconfig、各包的 .mk）
KFILES=(bootanidrv.c displayinit.c metazone_drv.c 'ac83xx*defconfig')
MKPAT=('musicplayer*.mk' 'hcn_application*.mk' 'lvgl_home*.mk' 'alsa-plugins*.mk')

die(){ echo "!! $*" >&2; exit 1; }

gen_manifest() {
  local tree=$1
  [ -d "$tree" ] || die "树目录不存在: $tree"
  cd "$tree" || exit 1
  {
    for d in "${SCOPE[@]}"; do
      [ -d "$d" ] && find "$d" \( \( -name .git -o -name build -o -name node_modules \) -type d -o -name '*.orig' -o -name '*.o' \) -prune -o -type f -print0
    done
    for f in "${KFILES[@]}"; do
      find source/kernel buildroot -name "$f" -type f -print0 2>/dev/null
    done
    for p in "${MKPAT[@]}"; do
      find buildroot build configs -name "$p" -type f -print0 2>/dev/null
    done
  } | sort -zu | while IFS= read -r -d '' f; do
    printf '%s\t%s\t%s\n' "$f" "$(stat -c %s "$f")" "$(md5sum <"$f" | cut -d' ' -f1)"
  done
}

is_bin() {
  case "$1" in
    prebuild/atc_ftl/*|*.a|*.so|*.so.*|*.o|*.bin|*.img|*.ko) return 0;;
  esac
  return 1
}

compare() {
  local man=$1 repo=$2 out=${3:-./cmp_out}
  [ -f "$man" ] || die "manifest 不存在: $man"
  [ -d "$repo/.git" ] || echo "(提示: $repo 不是 git 仓库根目录，请确认路径)" >&2
  mkdir -p "$out"
  : >"$out/missing.txt"; : >"$out/diff.txt"; : >"$out/same.txt"; : >"$out/skipped.txt"; : >"$out/push.list"
  local p sz m status c rm cand
  while IFS=$'\t' read -r p sz m; do
    [ -n "$p" ] || continue
    cand=("$p")
    # 远程把 source/vendor/hcn/application/ 下的音乐文件放在 hcn_music_lvgl/ 里
    case "$p" in source/vendor/hcn/application/*) cand+=("hcn_music_lvgl/${p#source/vendor/hcn/application/}");; esac
    status=MISSING
    for c in "${cand[@]}"; do
      if [ -f "$repo/$c" ]; then
        rm=$(md5sum <"$repo/$c" | cut -d' ' -f1)
        if [ "$rm" = "$m" ]; then status=SAME; break; else status=DIFF; fi
      fi
    done
    if [ "$status" != SAME ]; then
      if is_bin "$p"; then printf 'SKIP-BIN\t%s\t%s\t%s\n' "$p" "$sz" "$m" >>"$out/skipped.txt"; continue; fi
      if [ "$sz" -gt "$MAXSZ" ]; then printf 'SKIP-BIG\t%s\t%s\t%s\n' "$p" "$sz" "$m" >>"$out/skipped.txt"; continue; fi
    fi
    case "$status" in
      SAME)    echo "$p" >>"$out/same.txt";;
      MISSING) echo "$p" >>"$out/missing.txt"; echo "$p" >>"$out/push.list";;
      DIFF)    echo "$p" >>"$out/diff.txt";    echo "$p" >>"$out/push.list";;
    esac
  done <"$man"

  {
    echo "== 对比结果 ($(date '+%F %T')) =="
    echo "manifest: $man   repo: $repo"
    printf 'SAME %s | MISSING %s | DIFF %s | SKIPPED(二进制/过大) %s | 待推 %s\n' \
      "$(wc -l <"$out/same.txt")" "$(wc -l <"$out/missing.txt")" "$(wc -l <"$out/diff.txt")" \
      "$(wc -l <"$out/skipped.txt")" "$(wc -l <"$out/push.list")"
    echo; echo "-- 按目录统计待推文件 --"
    awk -F/ '{ k=$1"/"$2"/"$3"/"$4; print k }' "$out/push.list" | sort | uniq -c | sort -rn | head -30
    echo; echo "-- DIFF（两边都有，内容不同）前 40 个 --"; head -40 "$out/diff.txt"
    echo; echo "-- MISSING（远程没有）前 60 个 --"; head -60 "$out/missing.txt"
    echo; echo "-- SKIPPED（只记 MD5，不建议推）--"; cut -f1,2,4 "$out/skipped.txt" | head -30
  } | tee "$out/report.txt"
  echo; echo "完整清单在: $out/{missing,diff,same,skipped}.txt  待推清单: $out/push.list"
}

pack() {
  local tree=$1 list=$2 tgz=${3:-$HOME/dylan/push_to_remote.tgz}
  [ -d "$tree" ] && [ -f "$list" ] || die "用法: pack TREE push.list [out.tgz]"
  # 简单的敏感信息检查
  local bad=0 f
  while IFS= read -r f; do
    [ -f "$tree/$f" ] || { echo "!! 清单里的文件在树里不存在: $f"; continue; }
    if grep -qIE 'ghp_[A-Za-z0-9]{20,}|BEGIN [A-Z ]*PRIVATE KEY|AKIA[0-9A-Z]{16}' "$tree/$f" 2>/dev/null; then
      echo "!! 疑似含密钥/令牌，已从包里剔除: $f"; bad=1; echo "$f" >>"${list}.excluded"
    fi
  done <"$list"
  if [ -f "${list}.excluded" ]; then grep -vxFf "${list}.excluded" "$list" >"${list}.final"; else cp "$list" "${list}.final"; fi
  tar czf "$tgz" -C "$tree" -T "${list}.final" || die "打包失败"
  echo "已打包 $(wc -l <"${list}.final") 个文件 -> $tgz ($(du -h "$tgz" | cut -f1))"
  echo "放进仓库时解压到 da_tree_126/ 下，路径与 126 树内保持一致："
  echo "  mkdir -p da_tree_126 && tar xzf $(basename "$tgz") -C da_tree_126"
  [ $bad = 1 ] && echo "(有文件被剔除，见 ${list}.excluded)"
  return 0
}

case "${1:-}" in
  manifest) gen_manifest "${2:-/work2/tanyunlong/dylan/AC8215_DA}";;
  compare)  [ $# -ge 3 ] || die "用法: compare MANIFEST REPO_DIR [OUTDIR]"; compare "$2" "$3" "${4:-./cmp_out}";;
  pack)     pack "${2:-}" "${3:-}" "${4:-}";;
  both)     [ $# -ge 3 ] || die "用法: both TREE REPO_DIR [OUTDIR]"
            tmp=$(mktemp); gen_manifest "$2" >"$tmp"; compare "$tmp" "$3" "${4:-./cmp_out}"; rm -f "$tmp"
            echo; echo "下一步: $0 pack $2 ${4:-./cmp_out}/push.list";;
  *) sed -n 2,22p "$0"; exit 1;;
esac
