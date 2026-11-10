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

if grep -q 'HCN_APP_MUSIC' "$APP/CMakeLists.txt"; then echo "[app] already integrated (HCN_APP_MUSIC found in CMakeLists.txt)"; else
  (cd "$APP" && patch -p1 --dry-run -s < "$PACK/integration.patch") || { echo "integration.patch does not apply cleanly: nothing was changed"; exit 1; }
  mkdir -p "$APP/adapters" "$APP/lvgl/lvgl_app/fonts"
  cp -r "$PACK/adapters/music" "$APP/adapters/"
  cp "$PACK"/lvgl/lvgl_app/screens/ui_Music.[ch] "$APP/lvgl/lvgl_app/screens/"
  cp "$PACK"/lvgl/lvgl_app/frameworks/music_focus.[ch] "$APP/lvgl/lvgl_app/frameworks/"
  cp "$PACK"/lvgl/lvgl_app/fonts/music_font_*.c "$APP/lvgl/lvgl_app/fonts/"
  (cd "$APP" && patch -p1 -s < "$PACK/integration.patch") && echo "[app] files copied, integration.patch applied"
fi

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
echo; echo "next (reconfigure is needed, -rebuild alone will NOT re-run cmake configure):"
echo "  rm -rf $TREE/out/build/ac83xx/hcn_application*  &&  ./allmake.sh -a <same args as before>"
