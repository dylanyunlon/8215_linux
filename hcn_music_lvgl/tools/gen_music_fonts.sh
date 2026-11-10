#!/bin/sh
# Generate CJK fonts for the LVGL music UI (the board only ships a 16px CJK font).
#   needs: python3 + fonttools, node + npm (lv_font_conv), Noto Sans CJK (OFL) or Source Han Sans (OFL)
#   usage: tools/gen_music_fonts.sh [path/to/NotoSansCJK-Regular.ttc] [out_dir]
# Charset = ASCII + GB2312 level-1 (3755 most-used hanzi) + CJK punctuation + a few UI symbols.
set -e
TTC=${1:-/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc}
OUT=${2:-$(dirname "$0")/../lvgl/lvgl_app/fonts}
# "size:bpp".  The real board lv_conf (cluster/lvgl/lv_conf.h) has LV_FONT_FMT_TXT_LARGE=1 and
# LV_USE_FONT_COMPRESSED=1, so fonts may exceed 1 MB of bitmap and can be compressed.
# Both must also be 1 when compiling these files.  Set MUSIC_FONT_COMPRESS=0 for a board without them.
SIZES=${MUSIC_FONT_SIZES:-"20:4 32:4"}
COMPRESS=${MUSIC_FONT_COMPRESS:-1}
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT

python3 - "$TTC" "$TMP" <<'PY'
import sys
from fontTools.ttLib import TTCollection
ttc, tmp = sys.argv[1], sys.argv[2]
for f in TTCollection(ttc).fonts:
    fam = f['name'].getDebugName(1) or ''
    if fam.endswith(' SC') or fam.endswith('SC') :   # "Noto Sans CJK SC"
        f.save(tmp + '/cjk_sc.otf'); print('face:', fam); break
else:
    sys.exit('no SC face in ' + ttc)
# charset
chars = []
for hi in range(0xB0, 0xD8):               # GB2312 level 1 hanzi: B0A1..D7F9
    for lo in range(0xA1, 0xFF):
        try: chars.append(bytes([hi, lo]).decode('gb2312'))
        except UnicodeDecodeError: pass
extra = '、。，．：；！？“”‘’（）《》〈〉「」『』【】—…·～％℃×÷♥♡★☆•'
open(tmp + '/symbols.txt', 'w', encoding='utf-8').write(''.join(chars) + extra)
print('hanzi:', len(chars))
PY

mkdir -p "$OUT"
for SB in $SIZES; do
  S=${SB%%:*}; BPP=${SB##*:}
  npx --yes lv_font_conv --font "$TMP/cjk_sc.otf" --size "$S" --bpp "$BPP" $([ "$COMPRESS" = 1 ] || echo --no-compress) \
      -r 0x20-0x7E --symbols "$(cat "$TMP/symbols.txt")" \
      --format lvgl --lv-include lvgl.h --lv-font-name "music_font_$S" \
      -o "$OUT/music_font_$S.c"
  ls -l "$OUT/music_font_$S.c" | awk '{printf "music_font_%s.c  %.1f MB\n", "'$S'", $5/1048576}'
done
