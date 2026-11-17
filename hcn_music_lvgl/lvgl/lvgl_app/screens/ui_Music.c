/**
 * @file ui_Music.c
 * @brief Local-music screen.  Layout 1024x600:
 *
 *   +--------------------------------------------------------------+
 *   | <  本地音乐                          [U盘 · 48首] [SD卡 · 12首] |  top bar 72 (device chips when >= 2 devices)
 *   +-----------------------+--------------------------------------+
 *   |  [ cover 240x240 ]    | 全部 收藏 文件夹 专辑 歌手 搜索        |
 *   |  [封面流]     [歌词]  |  | Song title                 03:45 ♥ |
 *   |  Title (32)           |    artist · album                    |
 *   |  Artist · Album (20)  |  ... virtual list / lyrics /         |
 *   |  =====o-----------    |      cover flow / search + keyboard  |
 *   |  01:23         03:45  |                                      |
 *   |   ♥   |<   (>)  >|  ~ |   (long-press |< / >| = rewind / fast-forward 5 s)
 *   +-----------------------+--------------------------------------+
 *   background: the playing cover, blurred and dimmed (shows through the left panel)
 *
 * Threading: LV_USE_OS is LV_OS_NONE, everything here runs on the GUI thread.
 * music_app events reach us through music_adapter_poll() -> idle_queue shim; album art for the cover flow is decoded
 * by music_art's worker thread (no LVGL there) and handed over the same way.
 */
#include "ui_Music.h"
#include "ui_Music_style.h"
#include "ui_Music_cf.h"
#include "music_art.h"
#include "music_search.h"
#include "favorite_manager.h"
#include "displayfocusmanager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCR_W 1024
#define SCR_H 600
#define ROW_H 72
#define ROW_POOL 8
#define COVER_PX 240
#define MAX_LYRIC_LABELS 400
#define SEEK_REPEAT_MS 350                           /* long-press on |< / >|: one MUSIC_APP_SEEK_STEP_MS every 350 ms */
#define LIST_X 8
#define LIST_Y 68
#define LIST_W 520
#define LIST_H 408
#define SEARCH_BAR_H 48
#define KB_H 206

typedef enum { TAB_ALL = 0, TAB_FAV, TAB_FOLDER, TAB_ALBUM, TAB_ARTIST, TAB_SEARCH, TAB_COUNT } tab_t;

/* what the virtual list is currently showing */
typedef enum { LM_TRACKS, LM_FAV, LM_FOLDERS, LM_ALBUMS, LM_ARTISTS, LM_GROUP, LM_SEARCH } lmode_t;

/* what the right-hand pane shows */
typedef enum { PANE_LIST, PANE_LYRICS, PANE_COVERFLOW } pane_t;

typedef enum { RK_TRACK, RK_ENTRY, RK_BACK } rowkind_t;

typedef struct {
    rowkind_t kind;
    char lead[16];                                   /* digits, or an LV_SYMBOL_* */
    char title[MUSIC_MAX_TAG_LEN + 8];
    char sub[2 * MUSIC_MAX_TAG_LEN + 8];
    char dur[16];
    bool current, fav;
} rowdata_t;

typedef struct {
    lv_obj_t *row, *bar, *lead, *title, *sub, *dur, *fav, *chev;
    int bound; /* row index currently bound, -1 = none */
} row_t;

static struct {
    lv_obj_t *scr;
    lv_obj_t *backdrop, *dim, *left_panel;
    lv_obj_t *src_label, *dev_chip[MAX_STORAGE_DEVICES], *dev_chip_lbl[MAX_STORAGE_DEVICES];
    lv_obj_t *cover, *cover_ph, *lyr_pill, *lyr_pill_lbl, *cf_pill, *cf_pill_lbl, *title, *sub, *slider, *t_cur, *t_total;
    lv_obj_t *btn_fav, *btn_play, *btn_mode, *mode_one;
    lv_obj_t *play_icon, *fav_icon, *mode_icon;
    music_art_img_t *cover_img, *bd_img;
    bool dragging;
    uint32_t last_seek;
    lv_obj_t *tabs[TAB_COUNT];
    lv_obj_t *vlist, *vspacer, *lyrics, *fade_top, *fade_bot, *empty;
    lv_obj_t *ta, *kb, *btn_clear, *search_count;
    bool kb_visible;
    const MusicInfo *search_res[MUSIC_SEARCH_MAX];
    int search_n;
    bool search_trunc;
    row_t rows[ROW_POOL];
    tab_t tab;
    lmode_t lmode;
    pane_t pane;
    int group_idx;                                   /* LM_GROUP: index into the album/artist list */
    bool group_is_artist;
    int lrc_cur;
    const lrc_data_t *lrc_shown;
    int lrc_n;
    lv_obj_t *lrc_lbl[MAX_LYRIC_LABELS];
} S;

lv_obj_t *ui_Music = NULL;

/* ------------------------------------------------------------------ helpers */
static bool is_visible(void) { return S.scr != NULL && lv_screen_active() == S.scr; }

static lv_obj_t *plain(lv_obj_t *parent, int x, int y, int w, int h)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_opa(o, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_outline_width(o, 0, 0);
    lv_obj_set_style_shadow_width(o, 0, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    lv_obj_set_style_radius(o, 0, 0);
    return o;
}

static lv_obj_t *label(lv_obj_t *parent, const lv_font_t *font, lv_color_t color, const char *txt)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_style_text_color(l, color, 0);
    lv_label_set_text(l, txt);
    return l;
}

static void fmt_time(char *buf, size_t n, int ms)
{
    int s = ms > 0 ? ms / 1000 : 0;
    snprintf(buf, n, "%02d:%02d", s / 60, s % 60);
}

static const char *safe_str(const char *s, const char *fallback) { return (s && s[0]) ? s : fallback; }

static const char *title_of(const MusicInfo *mi, char *buf, size_t n)
{
    if (mi->title[0]) return mi->title;
    snprintf(buf, n, "%s", safe_str(mi->filename, "未知歌曲"));
    return buf;
}

static void sub_of(const MusicInfo *mi, char *buf, size_t n)
{
    snprintf(buf, n, "%s · %s", safe_str(mi->artist, "未知艺术家"), safe_str(mi->album, "未知专辑"));
}

static void press_style(lv_obj_t *o, lv_color_t pressed_bg)
{
    lv_obj_add_flag(o, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(o, pressed_bg, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, LV_STATE_PRESSED);
}

/* ------------------------------------------------------------------ row model */
static bool is_current_path(const char *path)
{
    const music_app_state_t *st = music_app_get_state();
    return st->current_info && path && strcmp(st->current_info->filepath, path) == 0;
}

static bool sub_playlist_active(void)
{
    return music_app_get_state()->playlist_type != PLAYLIST_TYPE_DEVICE;
}

/* rows above the data rows: "back" in a group, "show all songs" banner while a sub-playlist is active */
static int header_rows(void)
{
    if (S.lmode == LM_GROUP) return 1;
    if (S.lmode == LM_TRACKS && sub_playlist_active()) return 1;
    return 0;                                          /* search results: no header */
}

static const music_group_t *group_at(bool artist, int idx, int *n_out)
{
    const music_group_t *g = NULL;
    int n = 0;
    if (artist) music_app_get_artist_list(&g, &n); else music_app_get_album_list(&g, &n);
    if (n_out) *n_out = n;
    return (g && idx >= 0 && idx < n) ? &g[idx] : NULL;
}

static int data_count(void)
{
    int n = 0;
    switch (S.lmode) {
    case LM_TRACKS:  return music_app_get_playlist_count();
    case LM_FAV:     music_app_get_favorite_list(&n); return n;
    case LM_FOLDERS: { const char **f = NULL; music_app_get_folder_list(&f, &n); return n; }
    case LM_ALBUMS:  group_at(false, 0, &n); return n;
    case LM_ARTISTS: group_at(true, 0, &n); return n;
    case LM_GROUP:   { const music_group_t *g = group_at(S.group_is_artist, S.group_idx, NULL); return g ? g->items.count : 0; }
    case LM_SEARCH:  return S.search_n;
    }
    return 0;
}

static int list_count(void) { return data_count() + header_rows(); }

static void fill_track(rowdata_t *rd, const MusicInfo *mi, int number)
{
    rd->kind = RK_TRACK;
    snprintf(rd->title, sizeof rd->title, "%s", mi->title[0] ? mi->title : safe_str(mi->filename, "未知歌曲"));
    snprintf(rd->sub, sizeof rd->sub, "%s · %s", safe_str(mi->artist, "未知艺术家"), safe_str(mi->album, "未知专辑"));
    if (mi->duration_ms > 0) fmt_time(rd->dur, sizeof rd->dur, mi->duration_ms);
    rd->current = is_current_path(mi->filepath);
    rd->fav = favorite_contains(mi->filepath);
    if (rd->current) snprintf(rd->lead, sizeof rd->lead, "%s", LV_SYMBOL_PLAY);
    else snprintf(rd->lead, sizeof rd->lead, "%d", number);
}

static bool row_fill(int row, rowdata_t *rd)
{
    int hdr = header_rows(), i = row - hdr;
    memset(rd, 0, sizeof *rd);
    if (row < 0) return false;

    if (row < hdr) {                                   /* header row */
        rd->kind = RK_BACK;
        snprintf(rd->lead, sizeof rd->lead, "%s", LV_SYMBOL_LEFT);
        if (S.lmode == LM_GROUP) {
            const music_group_t *g = group_at(S.group_is_artist, S.group_idx, NULL);
            snprintf(rd->title, sizeof rd->title, "%s", g ? safe_str(g->key, "未知") : "");
            snprintf(rd->sub, sizeof rd->sub, "返回%s列表", S.group_is_artist ? "歌手" : "专辑");
        } else {
            snprintf(rd->title, sizeof rd->title, "正在播放筛选列表");
            snprintf(rd->sub, sizeof rd->sub, "点按返回全部歌曲");
        }
        return true;
    }
    switch (S.lmode) {
    case LM_TRACKS: { const MusicInfo *mi = music_app_get_track_info(i); if (!mi) return false; fill_track(rd, mi, i + 1); return true; }
    case LM_FAV: {
        int n = 0; const MusicInfo *f = music_app_get_favorite_list(&n);
        if (!f || i >= n) return false;
        fill_track(rd, &f[i], i + 1); return true;
    }
    case LM_FOLDERS: {
        const char **f = NULL; int n = 0; const char *slash, *name;
        music_app_get_folder_list(&f, &n);
        if (!f || i >= n || !f[i]) return false;
        rd->kind = RK_ENTRY;
        slash = strrchr(f[i], '/'); name = (slash && slash[1]) ? slash + 1 : f[i];
        snprintf(rd->lead, sizeof rd->lead, "%s", LV_SYMBOL_DIRECTORY);
        snprintf(rd->title, sizeof rd->title, "%s", name);
        snprintf(rd->sub, sizeof rd->sub, "%s", f[i]);
        return true;
    }
    case LM_ALBUMS:
    case LM_ARTISTS: {
        const music_group_t *g = group_at(S.lmode == LM_ARTISTS, i, NULL);
        if (!g) return false;
        rd->kind = RK_ENTRY;
        snprintf(rd->lead, sizeof rd->lead, "%s", LV_SYMBOL_AUDIO);
        snprintf(rd->title, sizeof rd->title, "%s", safe_str(g->key, "未知"));
        snprintf(rd->sub, sizeof rd->sub, "%d 首", g->items.count);
        return true;
    }
    case LM_GROUP: {
        const music_group_t *g = group_at(S.group_is_artist, S.group_idx, NULL);
        if (!g || i >= g->items.count || !g->items.items[i]) return false;
        fill_track(rd, g->items.items[i], i + 1); return true;
    }
    case LM_SEARCH:
        if (i < 0 || i >= S.search_n) return false;
        fill_track(rd, S.search_res[i], i + 1); return true;
    }
    return false;
}

/* ------------------------------------------------------------------ virtual list */
static bool lead_is_symbol(const rowdata_t *rd) { return rd->lead[0] && (unsigned char)rd->lead[0] >= 0x80; }

static void row_bind(row_t *r, int row)
{
    rowdata_t rd;
    if (!row_fill(row, &rd)) { lv_obj_add_flag(r->row, LV_OBJ_FLAG_HIDDEN); r->bound = -1; return; }
    lv_obj_remove_flag(r->row, LV_OBJ_FLAG_HIDDEN);
    r->bound = row;
    lv_obj_set_y(r->row, row * ROW_H);
    lv_label_set_text(r->title, rd.title);
    lv_label_set_text(r->sub, rd.sub);
    lv_label_set_text(r->dur, rd.dur);
    lv_label_set_text(r->lead, rd.lead);
    lv_obj_set_style_text_font(r->lead, lead_is_symbol(&rd) ? F_SYM : F_BODY, 0);
    lv_obj_set_style_text_color(r->lead, (rd.current || rd.kind != RK_TRACK) ? C_ACCENT : C_MUTED, 0);
    lv_obj_set_style_text_color(r->title, rd.current ? C_ACCENT : C_TEXT, 0);
    lv_obj_set_style_bg_opa(r->row, rd.kind == RK_BACK ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_color(r->row, C_ACCENT_BG, 0);
    if (rd.current) lv_obj_remove_flag(r->bar, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(r->bar, LV_OBJ_FLAG_HIDDEN);
    if (rd.fav) lv_obj_remove_flag(r->fav, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(r->fav, LV_OBJ_FLAG_HIDDEN);
    if (rd.kind == RK_ENTRY) lv_obj_remove_flag(r->chev, LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(r->chev, LV_OBJ_FLAG_HIDDEN);
}

static void vlist_sync(bool force)
{
    int count = list_count();
    int first = lv_obj_get_scroll_y(S.vlist) / ROW_H;
    int max_first = count > ROW_POOL ? count - ROW_POOL : 0;
    if (first > max_first) first = max_first;
    if (first < 0) first = 0;
    /* A row is pinned to slot (idx % ROW_POOL): scrolling by one row rebinds exactly one row. */
    for (int idx = first; idx < first + ROW_POOL; idx++) {
        row_t *r = &S.rows[idx % ROW_POOL];
        if (idx >= count) { lv_obj_add_flag(r->row, LV_OBJ_FLAG_HIDDEN); r->bound = -1; }
        else if (force || r->bound != idx) row_bind(r, idx);
    }
}

static void on_vlist_scroll(lv_event_t *e) { LV_UNUSED(e); vlist_sync(false); }

static void vlist_reload(bool to_top)
{
    int count = list_count();
    lv_obj_set_height(S.vspacer, count * ROW_H > 1 ? count * ROW_H : 1);
    if (to_top || lv_obj_get_scroll_y(S.vlist) > count * ROW_H) lv_obj_scroll_to_y(S.vlist, 0, LV_ANIM_OFF);
    for (int i = 0; i < ROW_POOL; i++) S.rows[i].bound = -1;
    vlist_sync(true);
}

static const tab_t LMODE_TAB[] = {TAB_ALL, TAB_FAV, TAB_FOLDER, TAB_ALBUM, TAB_ARTIST, TAB_ALBUM, TAB_SEARCH};
static void set_lmode(lmode_t m, bool to_top);
static void sync_status(void);

static void on_row_clicked(lv_event_t *e)
{
    row_t *r = (row_t *)lv_event_get_user_data(e);
    int hdr = header_rows(), i;
    if (!r || r->bound < 0) return;
    i = r->bound - hdr;

    if (r->bound < hdr) {                              /* header row */
        if (S.lmode == LM_GROUP) set_lmode(S.group_is_artist ? LM_ARTISTS : LM_ALBUMS, true);
        else { music_app_restore_full_playlist(); vlist_reload(true); sync_status(); }
        return;
    }
    switch (S.lmode) {
    case LM_TRACKS: music_app_safe_play(i); break;
    case LM_FAV: {                                     /* same approach as the AWTK UI: map back into the full device list */
        int n = 0, total, k;
        const MusicInfo *f = music_app_get_favorite_list(&n);
        char path[MUSIC_MAX_PATH_LEN];
        if (!f || i < 0 || i >= n) break;
        snprintf(path, sizeof path, "%s", f[i].filepath);
        music_app_restore_full_playlist();
        total = music_app_get_playlist_count();
        for (k = 0; k < total; k++) {
            const MusicInfo *p = music_app_get_track_info(k);
            if (p && strcmp(p->filepath, path) == 0) { music_app_safe_play(k); break; }
        }
        break;
    }
    case LM_FOLDERS: {
        const char **f = NULL; int n = 0;
        music_app_get_folder_list(&f, &n);
        if (f && i >= 0 && i < n && f[i]) { music_app_play_folder(f[i]); S.tab = TAB_ALL; set_lmode(LM_TRACKS, true); }
        break;
    }
    case LM_ALBUMS:
    case LM_ARTISTS:
        S.group_is_artist = (S.lmode == LM_ARTISTS);
        S.group_idx = i;
        set_lmode(LM_GROUP, true);
        break;
    case LM_GROUP: {
        const music_group_t *g = group_at(S.group_is_artist, S.group_idx, NULL);
        if (g) music_app_play_group(g, i);
        break;
    }
    case LM_SEARCH: {                                  /* results point into the device list: index = pointer distance */
        const MusicInfo *mi = (i >= 0 && i < S.search_n) ? S.search_res[i] : NULL;
        const storage_device_state_t *dev = music_app_get_device(music_app_get_state()->current_device_idx);
        if (mi && dev && dev->music_list && mi >= dev->music_list->items && mi < dev->music_list->items + dev->music_list->count) {
            int idx = (int)(mi - dev->music_list->items);
            music_app_restore_full_playlist();
            music_app_safe_play(idx);
        }
        break;
    }
    }
}

static lv_obj_t *fixed_label(lv_obj_t *parent, const lv_font_t *font, lv_color_t color, int w)
{
    /* LV_LABEL_LONG_DOT only truncates when the height is fixed to the lines we want */
    lv_obj_t *l = label(parent, font, color, "");
    lv_obj_set_size(l, w, lv_font_get_line_height(font));
    lv_label_set_long_mode(l, LV_LABEL_LONG_DOT);
    return l;
}

static void build_rows(lv_obj_t *parent)
{
    for (int i = 0; i < ROW_POOL; i++) {
        row_t *r = &S.rows[i];
        r->bound = -1;
        r->row = plain(parent, 0, 0, 520, ROW_H);
        press_style(r->row, C_RAISED);
        lv_obj_set_style_border_side(r->row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_width(r->row, 1, 0);
        lv_obj_set_style_border_color(r->row, C_LINE, 0);
        lv_obj_add_flag(r->row, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_event_cb(r->row, on_row_clicked, LV_EVENT_CLICKED, r);

        r->bar = plain(r->row, 0, 16, 4, 40);
        lv_obj_set_style_bg_opa(r->bar, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(r->bar, C_ACCENT, 0);
        lv_obj_set_style_radius(r->bar, 2, 0);

        r->lead = label(r->row, F_BODY, C_MUTED, "");
        lv_obj_set_width(r->lead, 52);
        lv_obj_set_style_text_align(r->lead, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_pos(r->lead, 12, 22);

        r->title = fixed_label(r->row, F_BODY, C_TEXT, 330);
        lv_obj_set_pos(r->title, 76, 9);
        r->sub = fixed_label(r->row, F_BODY, C_MUTED, 330);
        lv_obj_set_pos(r->sub, 76, 38);

        r->dur = label(r->row, F_BODY, C_MUTED, "");
        lv_obj_set_width(r->dur, 64);
        lv_obj_set_style_text_align(r->dur, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_pos(r->dur, 418, 22);

        r->fav = label(r->row, F_BODY, C_ACCENT, "♥");
        lv_obj_set_pos(r->fav, 490, 22);

        r->chev = label(r->row, F_SYM, C_MUTED, LV_SYMBOL_RIGHT);
        lv_obj_set_pos(r->chev, 488, 24);
    }
}

/* ------------------------------------------------------------------ cover art + blurred backdrop */
static void backdrop_clear(void)
{
    if (S.backdrop) { lv_obj_add_flag(S.backdrop, LV_OBJ_FLAG_HIDDEN); lv_image_set_src(S.backdrop, NULL); }
    if (S.dim) lv_obj_add_flag(S.dim, LV_OBJ_FLAG_HIDDEN);
    if (S.left_panel) lv_obj_set_style_bg_opa(S.left_panel, LV_OPA_COVER, 0);
    music_art_free(S.bd_img);
    S.bd_img = NULL;
}

static void backdrop_set(void)
{
    music_art_img_t *bd;
    if (!MUSIC_UI_BACKDROP || !S.cover_img || !S.backdrop) { backdrop_clear(); return; }
    bd = music_art_make_backdrop(S.cover_img, 256, 150);     /* shown at 4x: a soft colour wash of the cover */
    if (!bd) { backdrop_clear(); return; }
    lv_image_set_src(S.backdrop, &bd->dsc);
    music_art_free(S.bd_img);                                 /* free the old pixels only after LVGL points at the new ones */
    S.bd_img = bd;
    lv_obj_remove_flag(S.backdrop, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(S.dim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(S.left_panel, LV_OPA_70, 0);      /* let the wash shine through the now-playing card */
}

static void cover_clear(void)
{
    if (S.cover) { lv_obj_add_flag(S.cover, LV_OBJ_FLAG_HIDDEN); lv_image_set_src(S.cover, NULL); }
    if (S.cover_ph) lv_obj_remove_flag(S.cover_ph, LV_OBJ_FLAG_HIDDEN);
    music_art_free(S.cover_img);
    S.cover_img = NULL;
    backdrop_clear();
}

static void cover_sync(void)
{
    const uint8_t *data = NULL;
    int size = 0;

    cover_clear();
    if (!S.cover || music_app_get_album_art(&data, &size) != 0 || !data || size <= 0) return;
    S.cover_img = music_art_decode(data, size, COVER_PX, 0);
    if (!S.cover_img) return;
    lv_image_set_src(S.cover, &S.cover_img->dsc);
    lv_obj_remove_flag(S.cover, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(S.cover_ph, LV_OBJ_FLAG_HIDDEN);
    backdrop_set();
}

/* ------------------------------------------------------------------ lyrics (single lines; the current one scrolls sideways when too long) */
static void lyric_line_style(lv_obj_t *l, bool current)
{
    const lv_font_t *f = current ? F_TITLE : F_BODY;
    lv_obj_set_style_text_font(l, f, 0);
    lv_obj_set_style_text_color(l, current ? C_ACCENT : C_MUTED, 0);
    lv_obj_set_height(l, lv_font_get_line_height(f) + 24);   /* + 2 x 12 px padding */
    lv_label_set_long_mode(l, current ? LV_LABEL_LONG_SCROLL_CIRCULAR : LV_LABEL_LONG_DOT);
}

static void lyrics_clear(void)
{
    lv_obj_clean(S.lyrics);
    S.lrc_n = 0;
    S.lrc_cur = -1;
    S.lrc_shown = NULL;
}

static void lyrics_rebuild(void)
{
    const lrc_data_t *lrc = music_app_get_lyrics();
    lyrics_clear();
    if (!lrc || lrc->count <= 0) {
        lv_obj_t *l = label(S.lyrics, F_BODY, C_MUTED, "暂无歌词");
        lv_obj_set_width(l, 500);
        lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_top(l, 150, 0);
        return;
    }
    S.lrc_shown = lrc;
    for (int i = 0; i < lrc->count && i < MAX_LYRIC_LABELS; i++) {
        lv_obj_t *l = label(S.lyrics, F_BODY, C_MUTED, lrc->lines[i].text[0] ? lrc->lines[i].text : " ");
        lv_obj_set_width(l, 500);
        lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_ver(l, 12, 0);
        lyric_line_style(l, false);
        S.lrc_lbl[S.lrc_n++] = l;
    }
    lv_obj_t *pad = plain(S.lyrics, 0, 0, 1, 170);           /* lets first/last line reach the centre */
    LV_UNUSED(pad);
}

static void lyrics_seek(int pos_ms)
{
    int line;
    if (!S.lrc_shown || S.lrc_n == 0) return;
    line = music_app_get_lyrics_line(pos_ms);
    if (line < 0 || line >= S.lrc_n || line == S.lrc_cur) return;
    if (S.lrc_cur >= 0 && S.lrc_cur < S.lrc_n) lyric_line_style(S.lrc_lbl[S.lrc_cur], false);
    S.lrc_cur = line;
    lyric_line_style(S.lrc_lbl[line], true);
    lv_obj_update_layout(S.lyrics);
    {
        int cy = lv_obj_get_y(S.lrc_lbl[line]) + lv_obj_get_height(S.lrc_lbl[line]) / 2;
        int target = cy - lv_obj_get_height(S.lyrics) / 2;
        lv_obj_scroll_to_y(S.lyrics, target > 0 ? target : 0, LV_ANIM_ON);
    }
}

/* ------------------------------------------------------------------ sync from music_app */
static void sync_play_state(void)
{
    const music_app_state_t *st = music_app_get_state();
    lv_label_set_text(S.play_icon, st->player_state == PLAYER_STATE_PLAYING ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);
}

static void sync_mode(void)
{
    static const char *icons[] = {LV_SYMBOL_LIST, LV_SYMBOL_LOOP, LV_SYMBOL_LOOP, LV_SYMBOL_SHUFFLE};
    PlayMode m = music_app_get_state()->play_mode;
    lv_label_set_text(S.mode_icon, icons[(int)m & 3]);
    lv_obj_set_style_text_color(S.mode_icon, m == PLAY_MODE_SEQUENTIAL ? C_MUTED : C_ACCENT, 0);
    if (m == PLAY_MODE_REPEAT_ONE) lv_obj_remove_flag(S.mode_one, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(S.mode_one, LV_OBJ_FLAG_HIDDEN);
}

static void sync_fav(void)
{
    bool fav = music_app_get_state()->current_info && music_app_is_favorite();
    lv_label_set_text(S.fav_icon, fav ? "♥" : "♡");
    lv_obj_set_style_text_color(S.fav_icon, fav ? C_ACCENT : C_MUTED, 0);
}

static void sync_position(void)
{
    const music_app_state_t *st = music_app_get_state();
    char b[16];
    if (S.dragging) return;
    if (st->current_duration_ms > 0) lv_slider_set_range(S.slider, 0, st->current_duration_ms);
    lv_slider_set_value(S.slider, st->current_position_ms, LV_ANIM_OFF);
    fmt_time(b, sizeof b, st->current_position_ms);
    lv_label_set_text(S.t_cur, b);
    fmt_time(b, sizeof b, st->current_duration_ms);
    lv_label_set_text(S.t_total, b);
    lyrics_seek(st->current_position_ms);
}

static void sync_pane(void);

static void sync_meta(void)
{
    const music_app_state_t *st = music_app_get_state();
    char tb[MUSIC_MAX_TAG_LEN + 8], sb[2 * MUSIC_MAX_TAG_LEN + 8];
    bool had_pane = (S.pane != PANE_LIST);
    if (st->current_info) {
        lv_label_set_text(S.title, title_of(st->current_info, tb, sizeof tb));
        sub_of(st->current_info, sb, sizeof sb);
        lv_label_set_text(S.sub, sb);
    } else {
        lv_label_set_text(S.title, "未在播放");
        lv_label_set_text(S.sub, "选择右侧歌曲开始播放");
    }
    /* the lyrics / cover-flow switches only make sense while there is a current track */
    if (st->current_info) { lv_obj_remove_flag(S.lyr_pill, LV_OBJ_FLAG_HIDDEN); lv_obj_remove_flag(S.cf_pill, LV_OBJ_FLAG_HIDDEN); }
    else { lv_obj_add_flag(S.lyr_pill, LV_OBJ_FLAG_HIDDEN); lv_obj_add_flag(S.cf_pill, LV_OBJ_FLAG_HIDDEN); S.pane = PANE_LIST; }
    cover_sync();
    lyrics_rebuild();
    sync_fav();
    sync_position();
    if (had_pane && S.pane == PANE_LIST) sync_pane();
}

/* device chips (top bar): shown when two or more storage devices are mounted */
static void on_dev_chip(lv_event_t *e)
{
    music_app_switch_device((int)(intptr_t)lv_event_get_user_data(e));
}

static void sync_devices(void)
{
    const music_app_state_t *st = music_app_get_state();
    int total = music_app_get_device_count(), mounted = 0, idx[MAX_STORAGE_DEVICES], i, w, x;
    for (i = 0; i < total && i < MAX_STORAGE_DEVICES; i++) {
        const storage_device_state_t *d = music_app_get_device(i);
        if (d && d->mounted) idx[mounted++] = i;
    }
    if (mounted < 2) {
        for (i = 0; i < MAX_STORAGE_DEVICES; i++) lv_obj_add_flag(S.dev_chip[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(S.src_label, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_add_flag(S.src_label, LV_OBJ_FLAG_HIDDEN);
    w = mounted <= 2 ? 200 : 150;
    x = SCR_W - 24 - mounted * (w + 8) + 8;
    for (i = 0; i < MAX_STORAGE_DEVICES; i++) {
        lv_obj_t *c = S.dev_chip[i];
        if (i >= mounted) { lv_obj_add_flag(c, LV_OBJ_FLAG_HIDDEN); continue; }
        {
            const storage_device_state_t *d = music_app_get_device(idx[i]);
            bool cur = idx[i] == st->current_device_idx;
            char b[64];
            int cnt = d->music_list ? d->music_list->count : 0;
            if (d->scanning) snprintf(b, sizeof b, "%s · 扫描中", d->type == STORAGE_TYPE_SD ? "SD卡" : "U盘");
            else snprintf(b, sizeof b, "%s · %d 首", d->type == STORAGE_TYPE_SD ? "SD卡" : "U盘", cnt);
            lv_label_set_text(S.dev_chip_lbl[i], b);
            lv_obj_set_width(S.dev_chip_lbl[i], w - 24);
            lv_obj_set_style_text_color(S.dev_chip_lbl[i], cur ? C_ACCENT : C_MUTED, 0);
            lv_obj_set_style_bg_opa(c, cur ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_color(c, cur ? C_ACCENT : C_LINE, 0);
            lv_obj_set_pos(c, x + i * (w + 8), 14);
            lv_obj_set_size(c, w, 44);
            lv_obj_set_user_data(c, (void *)(intptr_t)idx[i]);
            lv_obj_remove_flag(c, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

/* status chip + empty states */
static void sync_status(void)
{
    const music_app_state_t *st = music_app_get_state();
    const storage_device_state_t *dev = music_app_get_device(st->current_device_idx);
    int count = data_count();
    char b[96];
    const char *msg = NULL;

    sync_devices();
    if (!dev || !dev->mounted) {
        lv_label_set_text(S.src_label, "未检测到存储设备");
        msg = "未检测到 U 盘或 SD 卡\n请插入存有音乐的存储设备";
    } else {
        const char *kind = dev->type == STORAGE_TYPE_SD ? "SD卡" : "U盘";
        const char *scope = "";
        if (st->playlist_type == PLAYLIST_TYPE_FOLDER) scope = " · 文件夹";
        else if (st->playlist_type == PLAYLIST_TYPE_ALBUM) scope = " · 专辑";
        else if (st->playlist_type == PLAYLIST_TYPE_ARTIST) scope = " · 歌手";
        else if (st->playlist_type == PLAYLIST_TYPE_FAVORITE) scope = " · 收藏";
        if (dev->scanning) snprintf(b, sizeof b, "%s · 正在扫描 %d 首", kind, music_app_get_playlist_count());
        else snprintf(b, sizeof b, "%s · %d 首%s", kind, music_app_get_playlist_count(), scope);
        lv_label_set_text(S.src_label, b);
        if (S.lmode == LM_SEARCH) {
            const char *q = lv_textarea_get_text(S.ta);
            if (!q || !q[0]) msg = "输入歌名、歌手或拼音首字母\n例如 zjl 可找到「周杰伦」";
            else if (count == 0) msg = "没有找到匹配的歌曲";
        } else if (count == 0) {
            switch (S.lmode) {
            case LM_FAV:     msg = "还没有收藏的歌曲\n点按 ♡ 收藏当前歌曲"; break;
            case LM_FOLDERS: msg = dev->scanning ? "正在扫描音乐…" : "没有文件夹"; break;
            case LM_ALBUMS:
            case LM_ARTISTS: msg = dev->scanning ? "正在扫描音乐…" : "暂无分类信息"; break;
            default:         msg = dev->scanning ? "正在扫描音乐…" : "没有找到音乐文件"; break;
            }
        }
    }
    if (S.pane != PANE_LIST) msg = NULL;
    if (msg) {
        lv_label_set_text(S.empty, msg);
        if (S.lmode == LM_SEARCH) lv_obj_align(S.empty, LV_ALIGN_TOP_MID, 0, LIST_Y + SEARCH_BAR_H + (S.kb_visible ? 36 : 120));
        else lv_obj_align(S.empty, LV_ALIGN_CENTER, 0, 20);
        lv_obj_remove_flag(S.empty, LV_OBJ_FLAG_HIDDEN);
    } else lv_obj_add_flag(S.empty, LV_OBJ_FLAG_HIDDEN);

    if (S.lmode == LM_SEARCH && S.search_n > 0) {
        snprintf(b, sizeof b, S.search_trunc ? "%d+ 首" : "%d 首", S.search_n);
        lv_label_set_text(S.search_count, b);
    } else lv_label_set_text(S.search_count, "");
}

static tab_t current_tab(void)
{
    if (S.lmode == LM_GROUP) return S.group_is_artist ? TAB_ARTIST : TAB_ALBUM;
    return LMODE_TAB[S.lmode];
}

static void sync_pane(void)
{
    bool list = (S.pane == PANE_LIST), search = list && S.lmode == LM_SEARCH, want_cf = (S.pane == PANE_COVERFLOW);
    int vy = LIST_Y, vh = LIST_H;

    for (int i = 0; i < TAB_COUNT; i++) {
        bool sel = list && (i == (int)current_tab());
        lv_obj_set_style_bg_opa(S.tabs[i], sel ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_text_color(lv_obj_get_child(S.tabs[i], 0), sel ? C_ACCENT : C_MUTED, 0);
    }
    lv_obj_set_style_bg_color(S.lyr_pill, S.pane == PANE_LYRICS ? C_ACCENT : lv_color_black(), 0);
    lv_obj_set_style_bg_opa(S.lyr_pill, S.pane == PANE_LYRICS ? LV_OPA_COVER : LV_OPA_60, 0);
    lv_obj_set_style_text_color(S.lyr_pill_lbl, S.pane == PANE_LYRICS ? C_BG : C_TEXT, 0);
    lv_obj_set_style_bg_color(S.cf_pill, want_cf ? C_ACCENT : lv_color_black(), 0);
    lv_obj_set_style_bg_opa(S.cf_pill, want_cf ? LV_OPA_COVER : LV_OPA_60, 0);
    lv_obj_set_style_text_color(S.cf_pill_lbl, want_cf ? C_BG : C_TEXT, 0);

    if (S.pane == PANE_LYRICS) {
        lv_obj_remove_flag(S.lyrics, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(S.fade_top, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(S.fade_bot, LV_OBJ_FLAG_HIDDEN);
        S.lrc_cur = -1;
        lyrics_seek(music_app_get_state()->current_position_ms);
    } else {
        lv_obj_add_flag(S.lyrics, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(S.fade_top, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(S.fade_bot, LV_OBJ_FLAG_HIDDEN);
    }
    if (want_cf != music_cf_is_visible()) music_cf_set_visible(want_cf);

    if (search) {
        vy = LIST_Y + SEARCH_BAR_H + 8;
        vh = S.kb_visible ? (LIST_Y + LIST_H - KB_H) - vy - 4 : LIST_Y + LIST_H - vy;
    }
    if (search) { lv_obj_remove_flag(S.ta, LV_OBJ_FLAG_HIDDEN); lv_obj_remove_flag(S.btn_clear, LV_OBJ_FLAG_HIDDEN); lv_obj_remove_flag(S.search_count, LV_OBJ_FLAG_HIDDEN); }
    else { lv_obj_add_flag(S.ta, LV_OBJ_FLAG_HIDDEN); lv_obj_add_flag(S.btn_clear, LV_OBJ_FLAG_HIDDEN); lv_obj_add_flag(S.search_count, LV_OBJ_FLAG_HIDDEN); }
    if (search && S.kb_visible) { lv_obj_remove_flag(S.kb, LV_OBJ_FLAG_HIDDEN); lv_obj_add_state(S.ta, LV_STATE_FOCUSED); }
    else { lv_obj_add_flag(S.kb, LV_OBJ_FLAG_HIDDEN); lv_obj_remove_state(S.ta, LV_STATE_FOCUSED); }

    if (list) {
        lv_obj_remove_flag(S.vlist, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(S.vlist, LIST_X, vy);
        lv_obj_set_height(S.vlist, vh);
        vlist_sync(true);
    } else lv_obj_add_flag(S.vlist, LV_OBJ_FLAG_HIDDEN);
    sync_status();
}

static void set_lmode(lmode_t m, bool to_top)
{
    S.lmode = m;
    S.pane = PANE_LIST;
    if (m != LM_SEARCH) S.kb_visible = false;
    vlist_reload(to_top);
    sync_pane();
}

static void search_update(void)
{
    const char *q = S.ta ? lv_textarea_get_text(S.ta) : "";
    S.search_n = music_search_run(q, S.search_res, MUSIC_SEARCH_MAX, &S.search_trunc);
    if (S.lmode == LM_SEARCH) { vlist_reload(true); sync_status(); }
}

static void sync_all(void)
{
    sync_play_state();
    sync_mode();
    sync_meta();
    vlist_reload(false);
    sync_pane();
    music_cf_refresh();
}

/* ------------------------------------------------------------------ input */
static void on_back(lv_event_t *e) { LV_UNUSED(e); displayfocusmanager_go_home(); }
static void on_prev(lv_event_t *e) { LV_UNUSED(e); music_app_safe_prev(); }
static void on_next(lv_event_t *e) { LV_UNUSED(e); music_app_safe_next(); }
static void seek_step(bool forward)
{
    if (lv_tick_elaps(S.last_seek) < SEEK_REPEAT_MS) return;    /* LVGL repeats every ~100 ms: throttle so the decoder is not thrashed */
    S.last_seek = lv_tick_get();
    if (forward) music_app_seek_forward(MUSIC_APP_SEEK_STEP_MS); else music_app_seek_backward(MUSIC_APP_SEEK_STEP_MS);
}
static void on_prev_hold(lv_event_t *e) { LV_UNUSED(e); seek_step(false); }
static void on_next_hold(lv_event_t *e) { LV_UNUSED(e); seek_step(true); }
static void on_mode(lv_event_t *e) { LV_UNUSED(e); music_app_cycle_play_mode(); sync_mode(); }
static void on_fav(lv_event_t *e)
{
    LV_UNUSED(e);
    if (music_app_get_state()->current_info) music_app_toggle_favorite();
}
static void on_play(lv_event_t *e)
{
    LV_UNUSED(e);
    if (music_app_get_state()->current_info == NULL && music_app_get_playlist_count() > 0) music_app_safe_play(0);
    else music_app_toggle_play_pause();
}
static void on_tab(lv_event_t *e)
{
    static const lmode_t tab_mode[TAB_COUNT] = {LM_TRACKS, LM_FAV, LM_FOLDERS, LM_ALBUMS, LM_ARTISTS, LM_SEARCH};
    S.tab = (tab_t)(intptr_t)lv_event_get_user_data(e);
    if (S.tab == TAB_SEARCH) { S.kb_visible = true; search_update(); }
    set_lmode(tab_mode[S.tab], true);
}
static void on_lyrics_toggle(lv_event_t *e)
{
    LV_UNUSED(e);
    S.pane = (S.pane == PANE_LYRICS) ? PANE_LIST : PANE_LYRICS;
    sync_pane();
}
static void on_cf_toggle(lv_event_t *e)
{
    LV_UNUSED(e);
    S.pane = (S.pane == PANE_COVERFLOW) ? PANE_LIST : PANE_COVERFLOW;
    sync_pane();
}
static void on_slider(lv_event_t *e)
{
    lv_event_code_t c = lv_event_get_code(e);
    char b[16];
    if (c == LV_EVENT_PRESSED) S.dragging = true;
    else if (c == LV_EVENT_VALUE_CHANGED && S.dragging) {
        fmt_time(b, sizeof b, lv_slider_get_value(S.slider));
        lv_label_set_text(S.t_cur, b);
    } else if (c == LV_EVENT_RELEASED || c == LV_EVENT_PRESS_LOST) {
        if (S.dragging) { S.dragging = false; music_app_seek(lv_slider_get_value(S.slider)); }
    }
}
static void show_kb(bool show)
{
    S.kb_visible = show;
    sync_pane();
}
static void on_ta(lv_event_t *e)
{
    lv_event_code_t c = lv_event_get_code(e);
    if (c == LV_EVENT_VALUE_CHANGED) search_update();
    else if (c == LV_EVENT_CLICKED && !S.kb_visible) show_kb(true);
}
static void on_kb(lv_event_t *e)
{
    lv_event_code_t c = lv_event_get_code(e);
    if (c == LV_EVENT_READY || c == LV_EVENT_CANCEL) show_kb(false);        /* OK / keyboard key: results get the full height */
}
static void on_clear(lv_event_t *e)
{
    LV_UNUSED(e);
    lv_textarea_set_text(S.ta, "");
    search_update();
}

/* ------------------------------------------------------------------ build */
static lv_obj_t *icon_button(lv_obj_t *parent, int x, int y, int d, lv_obj_t **icon_out, const lv_font_t *font,
                             const char *txt, lv_color_t color, lv_event_code_t code, lv_event_cb_t cb)
{
    lv_obj_t *b = plain(parent, x, y, d, d);
    lv_obj_t *l = label(b, font, color, txt);
    press_style(b, C_RAISED);
    lv_obj_set_style_radius(b, d / 2, 0);
    lv_obj_center(l);
    lv_obj_add_event_cb(b, cb, code, NULL);
    if (icon_out) *icon_out = l;
    return b;
}

static lv_obj_t *pill(lv_obj_t *parent, int x, int y, int w, const char *txt, lv_obj_t **lbl_out, lv_event_cb_t cb)
{
    lv_obj_t *p = plain(parent, x, y, w, 38);
    lv_obj_set_style_radius(p, 19, 0);
    lv_obj_set_style_bg_opa(p, LV_OPA_60, 0);
    lv_obj_set_style_bg_color(p, lv_color_black(), 0);
    lv_obj_add_flag(p, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(p, 8);
    *lbl_out = label(p, F_BODY, C_TEXT, txt);
    lv_obj_center(*lbl_out);
    lv_obj_add_event_cb(p, cb, LV_EVENT_CLICKED, NULL);
    return p;
}

static void build_now_playing(lv_obj_t *scr)
{
    lv_obj_t *p = plain(scr, 24, 88, 424, 488);
    S.left_panel = p;
    lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(p, C_PANEL, 0);
    lv_obj_set_style_radius(p, 20, 0);

    lv_obj_t *frame = plain(p, (424 - COVER_PX) / 2, 20, COVER_PX, COVER_PX);
    lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(frame, C_RAISED, 0);
    lv_obj_set_style_radius(frame, 16, 0);
    lv_obj_set_style_clip_corner(frame, true, 0);
    S.cover_ph = label(frame, &lv_font_montserrat_32, C_LINE, LV_SYMBOL_AUDIO);
    lv_obj_center(S.cover_ph);
    S.cover = lv_image_create(frame);
    lv_obj_set_pos(S.cover, 0, 0);
    lv_obj_add_flag(S.cover, LV_OBJ_FLAG_HIDDEN);

    /* two switches along the bottom edge of the cover: the right-hand pane flips between list / cover flow / lyrics */
    S.cf_pill = pill(frame, 8, COVER_PX - 46, 96, "封面流", &S.cf_pill_lbl, on_cf_toggle);
    S.lyr_pill = pill(frame, COVER_PX - 84, COVER_PX - 46, 76, "歌词", &S.lyr_pill_lbl, on_lyrics_toggle);

    S.title = label(p, F_TITLE, C_TEXT, "");
    lv_obj_set_width(S.title, 376);
    lv_label_set_long_mode(S.title, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_pos(S.title, 24, 272);

    S.sub = fixed_label(p, F_BODY, C_MUTED, 376);
    lv_obj_set_pos(S.sub, 24, 320);

    S.slider = lv_slider_create(p);
    lv_obj_set_pos(S.slider, 28, 358);
    lv_obj_set_size(S.slider, 368, 8);
    lv_obj_set_ext_click_area(S.slider, 22);
    lv_slider_set_range(S.slider, 0, 1000);
    lv_obj_set_style_bg_color(S.slider, C_LINE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(S.slider, C_ACCENT, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(S.slider, C_TEXT, LV_PART_KNOB);
    lv_obj_set_style_pad_all(S.slider, 6, LV_PART_KNOB);
    lv_obj_add_event_cb(S.slider, on_slider, LV_EVENT_ALL, NULL);

    S.t_cur = label(p, F_BODY, C_MUTED, "00:00");
    lv_obj_set_pos(S.t_cur, 24, 376);
    S.t_total = label(p, F_BODY, C_MUTED, "00:00");
    lv_obj_set_width(S.t_total, 120);
    lv_obj_set_style_text_align(S.t_total, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(S.t_total, 424 - 24 - 120, 376);

    S.btn_fav = icon_button(p, 28, 414, 64, &S.fav_icon, F_TITLE, "♡", C_MUTED, LV_EVENT_CLICKED, on_fav);
    {   /* tap = previous / next, hold = rewind / fast-forward 5 s steps (SHORT_CLICKED: a long press must not also skip the track) */
        lv_obj_t *prev = icon_button(p, 100, 418, 56, NULL, F_ICON, LV_SYMBOL_PREV, C_TEXT, LV_EVENT_SHORT_CLICKED, on_prev);
        lv_obj_add_event_cb(prev, on_prev_hold, LV_EVENT_LONG_PRESSED_REPEAT, NULL);
        S.btn_play = icon_button(p, 176, 410, 72, &S.play_icon, F_ICON, LV_SYMBOL_PLAY, C_BG, LV_EVENT_CLICKED, on_play);
        lv_obj_t *next = icon_button(p, 268, 418, 56, NULL, F_ICON, LV_SYMBOL_NEXT, C_TEXT, LV_EVENT_SHORT_CLICKED, on_next);
        lv_obj_add_event_cb(next, on_next_hold, LV_EVENT_LONG_PRESSED_REPEAT, NULL);
    }
    lv_obj_set_style_bg_opa(S.btn_play, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(S.btn_play, C_ACCENT, 0);
    lv_obj_set_style_bg_color(S.btn_play, lv_color_hex(0x93C4A8), LV_STATE_PRESSED);
    S.btn_mode = icon_button(p, 332, 414, 64, &S.mode_icon, F_ICON, LV_SYMBOL_LIST, C_MUTED, LV_EVENT_CLICKED, on_mode);
    S.mode_one = label(S.btn_mode, &lv_font_montserrat_14, C_ACCENT, "1");
    lv_obj_align(S.mode_one, LV_ALIGN_CENTER, 0, 1);
    lv_obj_add_flag(S.mode_one, LV_OBJ_FLAG_HIDDEN);
}

static lv_obj_t *fade(lv_obj_t *parent, int y, bool top)
{
    lv_obj_t *f = plain(parent, LIST_X, y, LIST_W, 52);
    lv_obj_remove_flag(f, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(f, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(f, C_PANEL, 0);
    lv_obj_set_style_bg_grad_color(f, C_PANEL, 0);
    lv_obj_set_style_bg_grad_dir(f, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_main_opa(f, top ? LV_OPA_COVER : LV_OPA_TRANSP, 0);    /* opaque at the pane edge, clear towards the middle */
    lv_obj_set_style_bg_grad_opa(f, top ? LV_OPA_TRANSP : LV_OPA_COVER, 0);
    lv_obj_add_flag(f, LV_OBJ_FLAG_HIDDEN);
    return f;
}

static void build_right(lv_obj_t *scr)
{
    static const char *names[TAB_COUNT] = {"全部", "收藏", "文件夹", "专辑", "歌手", "搜索"};
    static const int widths[TAB_COUNT] = {72, 72, 96, 72, 72, 72};
    int x = 12;
    lv_obj_t *p = plain(scr, 464, 88, 536, 488);
    lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(p, C_PANEL, 0);
    lv_obj_set_style_radius(p, 20, 0);

    for (int i = 0; i < TAB_COUNT; i++) {
        lv_obj_t *t = plain(p, x, 10, widths[i], 48);
        lv_obj_t *l = label(t, F_BODY, C_MUTED, names[i]);
        press_style(t, C_RAISED);
        lv_obj_set_style_radius(t, 24, 0);
        lv_obj_set_style_bg_color(t, C_ACCENT_BG, 0);
        lv_obj_center(l);
        lv_obj_add_event_cb(t, on_tab, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        S.tabs[i] = t;
        x += widths[i] + 8;
    }

    /* virtual list: spacer gives the scroll range, a fixed pool of rows follows the viewport */
    S.vlist = plain(p, LIST_X, LIST_Y, LIST_W, LIST_H);
    lv_obj_add_flag(S.vlist, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(S.vlist, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(S.vlist, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_bg_color(S.vlist, C_LINE, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(S.vlist, LV_OPA_COVER, LV_PART_SCROLLBAR);
    lv_obj_set_style_width(S.vlist, 4, LV_PART_SCROLLBAR);
    lv_obj_set_style_clip_corner(S.vlist, true, 0);
    S.vspacer = plain(S.vlist, 0, 0, 1, 1);
    build_rows(S.vlist);
    lv_obj_add_event_cb(S.vlist, on_vlist_scroll, LV_EVENT_SCROLL, NULL);

    S.lyrics = plain(p, LIST_X, LIST_Y, LIST_W, LIST_H);
    lv_obj_add_flag(S.lyrics, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_scroll_dir(S.lyrics, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(S.lyrics, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(S.lyrics, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(S.lyrics, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    S.fade_top = fade(p, LIST_Y, true);                      /* lyrics fade out at the top and bottom edge */
    S.fade_bot = fade(p, LIST_Y + LIST_H - 52, false);

    music_cf_create(p, LIST_X, LIST_Y, LIST_W, LIST_H);

    /* search: input bar + Latin keyboard (the board has no pinyin IME; music_pinyin matches initials, e.g. zjl) */
    S.ta = lv_textarea_create(p);
    lv_obj_set_pos(S.ta, LIST_X, LIST_Y);
    lv_obj_set_size(S.ta, 330, SEARCH_BAR_H);
    lv_textarea_set_one_line(S.ta, true);
    lv_textarea_set_max_length(S.ta, 40);
    lv_textarea_set_placeholder_text(S.ta, "歌名 / 歌手 / 拼音首字母");
    lv_obj_set_style_text_font(S.ta, F_BODY, 0);
    lv_obj_set_style_text_font(S.ta, F_BODY, LV_PART_TEXTAREA_PLACEHOLDER);
    lv_obj_set_style_text_color(S.ta, C_TEXT, 0);
    lv_obj_set_style_text_color(S.ta, C_MUTED, LV_PART_TEXTAREA_PLACEHOLDER);
    lv_obj_set_style_bg_color(S.ta, C_RAISED, 0);
    lv_obj_set_style_bg_opa(S.ta, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(S.ta, 0, 0);
    lv_obj_set_style_outline_width(S.ta, 0, 0);
    lv_obj_set_style_shadow_width(S.ta, 0, 0);
    lv_obj_set_style_radius(S.ta, 14, 0);
    lv_obj_set_style_pad_left(S.ta, 16, 0);
    lv_obj_set_style_pad_ver(S.ta, 9, 0);
    lv_obj_set_style_border_width(S.ta, 2, LV_PART_CURSOR);
    lv_obj_set_style_border_side(S.ta, LV_BORDER_SIDE_LEFT, LV_PART_CURSOR);
    lv_obj_set_style_border_color(S.ta, C_ACCENT, LV_PART_CURSOR);
    lv_obj_add_flag(S.ta, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(S.ta, on_ta, LV_EVENT_ALL, NULL);

    S.search_count = label(p, F_BODY, C_MUTED, "");
    lv_obj_set_width(S.search_count, 84);
    lv_obj_set_style_text_align(S.search_count, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(S.search_count, LIST_X + 338, LIST_Y + 11);
    lv_obj_add_flag(S.search_count, LV_OBJ_FLAG_HIDDEN);

    S.btn_clear = plain(p, LIST_X + 430, LIST_Y, 90, SEARCH_BAR_H);
    lv_obj_t *cl = label(S.btn_clear, F_BODY, C_TEXT, "清除");
    lv_obj_center(cl);
    press_style(S.btn_clear, C_RAISED);
    lv_obj_set_style_radius(S.btn_clear, 14, 0);
    lv_obj_set_style_border_width(S.btn_clear, 1, 0);
    lv_obj_set_style_border_color(S.btn_clear, C_LINE, 0);
    lv_obj_add_flag(S.btn_clear, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(S.btn_clear, on_clear, LV_EVENT_CLICKED, NULL);

    S.kb = lv_keyboard_create(p);
    lv_obj_set_size(S.kb, LIST_W, KB_H);
    /* lv_keyboard_create() aligns itself to the bottom and set_pos() would only add to that: align explicitly */
    lv_obj_align(S.kb, LV_ALIGN_TOP_LEFT, LIST_X, LIST_Y + LIST_H - KB_H);
    lv_keyboard_set_mode(S.kb, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_keyboard_set_textarea(S.kb, S.ta);
    lv_obj_set_style_bg_opa(S.kb, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(S.kb, 0, 0);
    lv_obj_set_style_pad_all(S.kb, 0, 0);
    lv_obj_set_style_pad_gap(S.kb, 6, 0);
    lv_obj_set_style_text_font(S.kb, &lv_font_montserrat_20, LV_PART_ITEMS);   /* Latin + symbol glyphs; the default font here is CJK-only */
    lv_obj_set_style_text_color(S.kb, C_TEXT, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(S.kb, C_RAISED, LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(S.kb, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_radius(S.kb, 10, LV_PART_ITEMS);
    lv_obj_set_style_border_width(S.kb, 0, LV_PART_ITEMS);
    lv_obj_set_style_shadow_width(S.kb, 0, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(S.kb, C_ACCENT_BG, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(S.kb, C_ACCENT, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(S.kb, C_LINE, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_add_flag(S.kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(S.kb, on_kb, LV_EVENT_ALL, NULL);

    S.empty = label(p, F_BODY, C_MUTED, "");
    lv_obj_set_width(S.empty, 480);
    lv_obj_set_style_text_align(S.empty, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(S.empty, 8, 0);
    lv_obj_align(S.empty, LV_ALIGN_CENTER, 0, 20);
    lv_obj_add_flag(S.empty, LV_OBJ_FLAG_HIDDEN);
}

static void on_art_ready(const char *path, void *user)
{
    LV_UNUSED(user);
    music_cf_on_art_ready(path);
}

void ui_Music_screen_init(lv_obj_t **screen)
{
    lv_obj_t *back, *bl;
    if (S.scr) { if (screen) *screen = S.scr; return; }
    memset(&S, 0, sizeof S);
    S.lrc_cur = -1;
    S.tab = TAB_ALL;
    S.lmode = LM_TRACKS;
    S.pane = PANE_LIST;

    S.scr = lv_obj_create(NULL);
    ui_Music = S.scr;
    lv_obj_remove_flag(S.scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(S.scr, C_BG, 0);
    lv_obj_set_style_bg_opa(S.scr, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(S.scr, 0, 0);

    /* blurred cover backdrop (4x scaled colour wash) + dimming layer, behind everything */
    S.backdrop = lv_image_create(S.scr);
    lv_obj_remove_flag(S.backdrop, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_pivot(S.backdrop, 0, 0);
    lv_image_set_scale(S.backdrop, 1024);
    lv_image_set_antialias(S.backdrop, true);
    lv_obj_set_pos(S.backdrop, 0, 0);
    lv_obj_add_flag(S.backdrop, LV_OBJ_FLAG_HIDDEN);
    S.dim = plain(S.scr, 0, 0, SCR_W, SCR_H);
    lv_obj_remove_flag(S.dim, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(S.dim, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(S.dim, LV_OPA_50, 0);
    lv_obj_add_flag(S.dim, LV_OBJ_FLAG_HIDDEN);

    back = plain(S.scr, 12, 8, 72, 56);
    bl = label(back, &lv_font_montserrat_28, C_TEXT, LV_SYMBOL_LEFT);
    lv_obj_center(bl);
    press_style(back, C_RAISED);
    lv_obj_set_style_radius(back, 28, 0);
    lv_obj_add_event_cb(back, on_back, LV_EVENT_CLICKED, NULL);

    lv_obj_set_pos(label(S.scr, F_TITLE, C_TEXT, "本地音乐"), 96, 18);
    S.src_label = label(S.scr, F_BODY, C_MUTED, "");
    lv_obj_set_width(S.src_label, 420);
    lv_obj_set_style_text_align(S.src_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_pos(S.src_label, SCR_W - 24 - 420, 24);
    for (int i = 0; i < MAX_STORAGE_DEVICES; i++) {          /* device chips: switch between USB / SD */
        lv_obj_t *c = plain(S.scr, 0, 14, 200, 44);
        press_style(c, C_RAISED);
        lv_obj_set_style_radius(c, 22, 0);
        lv_obj_set_style_bg_color(c, C_ACCENT_BG, 0);
        lv_obj_set_style_border_width(c, 1, 0);
        lv_obj_set_style_border_color(c, C_LINE, 0);
        S.dev_chip_lbl[i] = label(c, F_BODY, C_MUTED, "");
        lv_label_set_long_mode(S.dev_chip_lbl[i], LV_LABEL_LONG_DOT);
        lv_obj_set_height(S.dev_chip_lbl[i], lv_font_get_line_height(F_BODY));
        lv_obj_center(S.dev_chip_lbl[i]);
        lv_obj_add_flag(c, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_event_cb(c, on_dev_chip, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        S.dev_chip[i] = c;
    }

    build_now_playing(S.scr);
    build_right(S.scr);
    music_art_init();
    music_art_set_ready_cb(on_art_ready, NULL);
    sync_all();
    if (screen) *screen = S.scr;
}

void ui_Music_screen_destroy(void)
{
    if (S.scr) lv_obj_delete(S.scr);
    music_cf_destroy();                                       /* after the objects are gone: it frees the placeholder they used */
    music_art_set_ready_cb(NULL, NULL);
    music_art_deinit();
    music_art_free(S.cover_img);
    music_art_free(S.bd_img);
    memset(&S, 0, sizeof S);
    ui_Music = NULL;
}

void ui_Music_refresh(void)
{
    if (S.scr) sync_all();
}

/* ------------------------------------------------------------------ app events */
void ui_Music_on_app_event(music_app_event_t event, void *param)
{
    (void)param;
    if (!S.scr) return;               /* screen not built yet: ui_Music_refresh() will catch up */
    /* hidden: only keep the play icon honest; on_enter -> ui_Music_refresh() resyncs the rest */
    if (!is_visible() && event != APP_EVENT_STATE_CHANGED && event != APP_EVENT_TRACK_CHANGED) return;

    switch (event) {
    case APP_EVENT_STORAGE_MOUNTED:
    case APP_EVENT_STORAGE_UNMOUNTED:
    case APP_EVENT_SCAN_STARTED:
    case APP_EVENT_SCAN_FINISHED:
    case APP_EVENT_PLAYLIST_CHANGED:
        if (S.lmode == LM_GROUP && !group_at(S.group_is_artist, S.group_idx, NULL)) S.lmode = S.group_is_artist ? LM_ARTISTS : LM_ALBUMS;
        search_update();                                      /* result pointers may dangle after a rescan / switch / unmount */
        vlist_reload(false);
        sync_pane();
        if (event == APP_EVENT_STORAGE_UNMOUNTED) sync_meta();
        music_cf_refresh();
        break;
    case APP_EVENT_SCAN_PROGRESS:     /* count only: never rebuild rows while scanning (issue #64 in the AWTK UI) */
        sync_status();
        break;
    case APP_EVENT_TRACK_CHANGED:
        if (is_visible()) { sync_meta(); vlist_sync(true); music_cf_refresh(); }
        sync_play_state();
        break;
    case APP_EVENT_STATE_CHANGED:
        sync_play_state();
        break;
    case APP_EVENT_POSITION_CHANGED:
        sync_position();
        break;
    case APP_EVENT_FAVORITE_CHANGED:
        sync_fav();
        if (S.lmode == LM_FAV) { vlist_reload(false); sync_status(); } else vlist_sync(true);
        break;
    case APP_EVENT_ERROR:
        lv_label_set_text(S.sub, "播放出错，请重试或选择其它歌曲");
        break;
    default:
        break;
    }
}
