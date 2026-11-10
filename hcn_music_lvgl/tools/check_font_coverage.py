#!/usr/bin/env python3
"""Every non-ASCII character in a string literal of the music UI must exist in music_font_20 / music_font_32,
   otherwise it is drawn as an empty box.  Charset = what tools/gen_music_fonts.sh generates.
   usage: tools/check_font_coverage.py            (exit 1 and a list when something is missing)"""
import glob, os, re, sys
root = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
gen = open(os.path.join(root, "tools", "gen_music_fonts.sh"), encoding="utf-8").read()
extra = re.search(r"extra\s*=\s*'([^']*)'", gen).group(1)
cs = set(extra)
for hi in range(0xB0, 0xD8):                                   # GB2312 level 1 (3755 hanzi)
    for lo in range(0xA1, 0xFF):
        try: cs.add(bytes([hi, lo]).decode("gb2312"))
        except UnicodeDecodeError: pass
files = glob.glob(os.path.join(root, "lvgl/lvgl_app/screens/ui_Music*.c")) + glob.glob(os.path.join(root, "lvgl/lvgl_app/frameworks/music_focus.c"))
lit = re.compile(r'"((?:[^"\\\n]|\\.)*)"')
missing = {}
for f in files:
    for n, line in enumerate(open(f, encoding="utf-8"), 1):
        code = re.sub(r"/\*.*?\*/", "", line)                    # drop block comments on the same line
        code = code.split("//")[0] if "//" in code and '"' not in code.split("//")[0][-1:] else code
        for m in lit.finditer(code):
            for ch in m.group(1):
                if ord(ch) > 0x7E and ch not in cs: missing.setdefault(ch, []).append("%s:%d" % (os.path.basename(f), n))
if missing:
    print("MISSING from the music fonts:")
    for ch, where in sorted(missing.items()): print("  U+%04X %s  %s" % (ord(ch), ch, ", ".join(where[:3])))
    sys.exit(1)
print("font coverage ok: %d files, all non-ASCII characters in string literals are in the font" % len(files))
