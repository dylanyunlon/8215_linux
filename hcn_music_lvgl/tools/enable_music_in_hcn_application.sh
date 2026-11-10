#!/bin/bash
# STEP 2: put the LVGL music UI into hcn_application and wire the Buildroot package.
#   usage:  enable_music_in_hcn_application.sh <tree> <pack_dir>
#           <pack_dir> = where hcn_music_lvgl.tar.gz was unpacked (has adapters/, lvgl/, integration.patch)
#   Run STEP 1 (add_musicplayer_pkg.sh) and rebuild the tree once before this.
#   Dry-runs the patch first; touches nothing if it does not apply.  Originals: hcn_application.mk.orig.
set -e
TREE=${1:?usage: $0 <tree> <pack_dir>}; PACK=${2:?missing pack dir}
APP=$TREE/source/vendor/hcn/application
MK=$TREE/source/vendor/autochips/proprietary/build/configs/package/hcn_application/hcn_application.mk
for f in "$APP/CMakeLists.txt" "$MK" "$PACK/integration.patch" "$PACK/adapters/music/CMakeLists.txt" "$PACK/lvgl/lvgl_app/screens/ui_Music.c"; do
  [ -e "$f" ] || { echo "missing: $f"; exit 1; }; done
[ -f "$TREE/out/staging/usr/lib/libmusicplayer.so" ] || { echo "libmusicplayer.so is not in out/staging yet: run STEP 1 and rebuild first"; exit 1; }

# 1) patch: dry-run first, touch nothing if it does not apply (skipped when already applied)
if grep -q 'HCN_APP_MUSIC' "$APP/CMakeLists.txt"; then NEED_PATCH=0; echo "[app] integration.patch already applied"; else
  (cd "$APP" && patch -p1 --dry-run -s < "$PACK/integration.patch") || { echo "integration.patch does not apply cleanly: nothing was changed"; exit 1; }
  NEED_PATCH=1
fi
# 2) files: always (re)sync everything under adapters/music and lvgl/lvgl_app from the pack
#    (the pack only holds our own files, so nothing of HCN's is overwritten; safe to re-run)
mkdir -p "$APP/adapters/music" "$APP/lvgl/lvgl_app"
cp -r "$PACK/adapters/music/." "$APP/adapters/music/"
cp -r "$PACK/lvgl/lvgl_app/." "$APP/lvgl/lvgl_app/"
echo "[app] files synced from the pack"
if [ "$NEED_PATCH" = 1 ]; then (cd "$APP" && patch -p1 -s < "$PACK/integration.patch") && echo "[app] integration.patch applied"; fi
# 3) every source the CMake music block lists must exist, otherwise add_library() fails at configure time
miss=0
for f in $(awk '/^if\(HCN_APP_MUSIC\)/{f=1;next} /^endif\(\)/{f=0} f' "$APP/lvgl/lvgl_app/CMakeLists.txt" | grep -oE '[A-Za-z_]+/[A-Za-z0-9_./]+\.c'); do
  [ -f "$APP/lvgl/lvgl_app/$f" ] || { echo "MISSING source listed in CMake: $f"; miss=1; }; done
for h in ui_Music_style.h ui_Music.h ui_Music_cf.h music_art.h music_search.h music_pinyin.h; do
  [ -f "$APP/lvgl/lvgl_app/screens/$h" ] || { echo "MISSING header: screens/$h"; miss=1; }; done
[ -f "$APP/lvgl/lvgl_app/frameworks/music_focus.h" ] || { echo "MISSING header: frameworks/music_focus.h"; miss=1; }
[ "$miss" = 0 ] || { echo "incomplete integration, fix the pack and re-run"; exit 1; }
echo "[app] all CMake-listed sources and headers present"

python3 - "$MK" <<'PY'
import os, re, sys
p = sys.argv[1]; s = open(p).read()
if 'HCN_APP_MUSIC_SRC' in s and re.search(r'^HCN_APPLICATION_DEPENDENCIES.*\bmusicplayer\b', s, re.M):
    print('[mk] already patched'); sys.exit(0)
if not os.path.exists(p + '.orig'): open(p + '.orig', 'w').write(s)
m = re.search(r'^(HCN_APPLICATION_DEPENDENCIES\s*[+]?=\s*[^\n]*?)[ \t]*$', s, re.M)
if not m: sys.exit('[mk] HCN_APPLICATION_DEPENDENCIES line not found')
if not re.search(r'\bmusicplayer\b', m.group(1)): s = s[:m.end(1)] + ' musicplayer' + s[m.end(1):]
m = re.search(r'^HCN_APPLICATION_CONF_OPTS\s*[+]?=\s*\\\n((?:[^\n]*\\\n)*)([^\n]*)$', s, re.M)
if not m: sys.exit('[mk] HCN_APPLICATION_CONF_OPTS block not found')
if 'HCN_APP_MUSIC_SRC' not in s:
    last = m.group(2).rstrip()
    new = last + ' \\\n\t-DHCN_APP_MUSIC_SRC=$(TOPDIR)/../source/packages/application/musicplayer'
    s = s[:m.start(2)] + new + s[m.end(2):]
open(p, 'w').write(s); print('[mk] patched: musicplayer dependency + -DHCN_APP_MUSIC_SRC')
PY
for f in "$TREE/out/target/ac83xx/etc/init.d/S01lvgl_home" "$TREE/out/target/ac83xx/usr/bin/lvgl_home"; do
  if [ -e "$f" ]; then
    echo "NOTE: stale $f"
    echo "      Buildroot does not remove the files of a package that was disabled later (BR2_PACKAGE_LVGL_HOME)."
    echo "      Move them out of out/target BEFORE the final build, otherwise the image starts two UI processes on one screen."
  fi
done
echo; echo "next (reconfigure is needed, -rebuild alone will NOT re-run cmake configure):"
echo "  rm -rf $TREE/out/build/ac83xx/hcn_application*  &&  ./allmake.sh -a <same args as before>"
