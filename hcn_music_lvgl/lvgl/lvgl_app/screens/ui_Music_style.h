/**
 * @file ui_Music_style.h
 * @brief Design tokens shared by the music screen files.  Palette follows the launcher / Bluetooth screens:
 *        dark slate, one accent (the launcher's sage green).
 */
#ifndef UI_MUSIC_STYLE_H
#define UI_MUSIC_STYLE_H

#include "lvgl/lvgl.h"

LV_FONT_DECLARE(music_font_20);
LV_FONT_DECLARE(music_font_32);

#define C_BG        lv_color_hex(0x0F1418)
#define C_PANEL     lv_color_hex(0x17202A)
#define C_RAISED    lv_color_hex(0x1F2933)
#define C_LINE      lv_color_hex(0x2B3845)
#define C_TEXT      lv_color_hex(0xF2F5F7)
#define C_MUTED     lv_color_hex(0x8D9BA8)
#define C_ACCENT    lv_color_hex(0x7BB191)
#define C_ACCENT_BG lv_color_hex(0x25392E)

/* 模糊封面背景 = 一张 256x150 的图放大 4 倍（带抗锯齿）垫在全屏最底下。
 * 板上实测：软件渲染下，快速滑动列表时界面线程占单核 60%~70%，原因是每次重绘都要重做这层缩放。
 * 定案：默认关闭，背景固定为纯色 C_BG；关闭时变暗遮罩一并隐藏，换歌也不再生成模糊图。
 * 需要时可用 -DMUSIC_UI_BACKDROP=1 打开（旧的缩放方案，滑动会很吃 CPU）。 */
#ifndef MUSIC_UI_BACKDROP
#define MUSIC_UI_BACKDROP 0
#endif

#define F_BODY   (&music_font_20)
#define F_TITLE  (&music_font_32)
#define F_ICON   (&lv_font_montserrat_32)
#define F_SYM    (&lv_font_montserrat_20)

#endif /* UI_MUSIC_STYLE_H */
