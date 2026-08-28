/**
 * @file music_ui.c
 * @brief AWTK UI layer — F133 layout reference rewrite.
 *
 * Layout: 1024×600, modeled after F133 music.ftu (decoded via zksw_batch_decoder.py).
 * Uses the same res/images/media_player/ assets as F133.
 *
 * Structure (matching F133 dual-view design, implemented as single-window with visibility):
 *   - List view: left device bar (168px) + right song list (810×510)
 *   - Play view: full-screen overlay with cover art, info, progress, controls
 *
 * Fixes integrated:
 *   #50  100-item playlist limit removed
 *   #54  Album/Artist second-level expand
 *   #56  Search UI tab + input
 *   #57  Album art display from ID3 APIC
 *   #60  Multi-line lyrics (5 lines with highlight)
 *   #67  cmus_bridge ID3 integration (via music_scanner.c USE_CMUS_ID3)
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#include "music_ui.h"
#include "favorite_manager.h"
/* music_app.h provides music_app_safe_play/next/prev with player lock check */

#include <stdio.h>
#include <string.h>

/*============================================================================
 * Widget name constants
 *==========================================================================*/
/* --- List view widgets --- */
#define W_DEVICE_BAR   "bar_device"
#define W_LIST_VIEW    "list_playlist"
#define W_LBL_STATUS   "lbl_status"
#define W_LBL_COUNT    "lbl_count"
#define W_TAB_BAR      "bar_tabs"
#define W_EDIT_SEARCH  "edit_search"
#define W_BTN_SEARCH   "btn_search_go"

/* --- Play view widgets (inside play_window) --- */
#define W_PLAY_WIN     "play_window"
#define W_COVER_BG     "img_cover_bg"
#define W_COVER_ART    "img_cover_art"
#define W_TITLE        "lbl_title"
#define W_ARTIST       "lbl_artist"
#define W_ALBUM        "lbl_album"
#define W_TIME_CUR     "lbl_time_cur"
#define W_TIME_SEP     "lbl_time_sep"
#define W_TIME_TOTAL   "lbl_time_total"
#define W_SLIDER       "slider_progress"
#define W_BTN_MODE     "btn_mode"
#define W_BTN_PREV     "btn_prev"
#define W_BTN_PLAY     "btn_play"
#define W_BTN_NEXT     "btn_next"
#define W_BTN_LIST     "btn_list"
#define W_BTN_FAV      "btn_fav"
#define W_LBL_LYRICS   "lbl_lyrics"

/* Tab button names */
#define W_BTN_TAB_ALL     "btn_tab_all"
#define W_BTN_TAB_FOLDER  "btn_tab_folder"
#define W_BTN_TAB_FAV     "btn_tab_fav"
#define W_BTN_TAB_ALBUM   "btn_tab_album"
#define W_BTN_TAB_ARTIST  "btn_tab_artist"
#define W_BTN_TAB_SEARCH  "btn_tab_search"

/* F133 colors */
#define COLOR_BG          "#111318"
#define COLOR_CYAN        "#00FCFF"
#define COLOR_WHITE       "#FFFFFF"
#define COLOR_GRAY        "#AAAAAA"
#define COLOR_DIM         "#666666"
#define COLOR_GOLD        "#FFD700"
#define COLOR_RED         "#FF6B6B"
#define COLOR_TEAL        "#80EEE1"

/* F133-aligned dimensions */
#define LIST_ITEM_H       60     /* touch-friendly row height (F133=83, we use 60) */
#define LIST_X            194
#define LIST_Y            70
#define LIST_W            810
#define LIST_H            510
#define DEV_BAR_W         168
#define CTRL_BTN_SZ       74
#define CTRL_BTN_Y        484
#define COVER_SZ          160    /* album art display size */
#define SEARCH_MAX        200

/*============================================================================
 * Active tab enum
 *==========================================================================*/
typedef enum {
    TAB_ALL = 0, TAB_FOLDER, TAB_FAV, TAB_ALBUM, TAB_ARTIST, TAB_SEARCH,
    TAB_COUNT
} tab_type_t;

/*============================================================================
 * Module state
 *==========================================================================*/
static widget_t* s_win = NULL;
static bool s_play_view_visible = false;
static bool s_slider_dragging = false;
static int  s_last_highlight_idx = -1;
static tab_type_t s_active_tab = TAB_ALL;

/* Paging state for playlist view (musikcube-style: only one page of
 * MusicInfo in memory at a time, ~50 KB per page vs 25 MB full list) */
#define PAGE_SIZE  (LIST_H / LIST_ITEM_H)  /* visible rows = 510/60 = 8 */
static int s_page_offset = 0;    /* first visible track index */
static int s_page_count  = 0;    /* items currently shown */

/*============================================================================
 * Helpers
 *==========================================================================*/
static widget_t* find(const char* name) {
    return s_win ? widget_lookup(s_win, name, TRUE) : NULL;
}

static void format_time(char* buf, int buf_len, int ms) {
    if (ms < 0) ms = 0;
    int sec = ms / 1000;
    int min = sec / 60;
    sec %= 60;
    snprintf(buf, buf_len, "%02d:%02d", min, sec);
}

/* Safe title: returns filename without extension if title is empty */
static const char* safe_title(const MusicInfo* info, char* buf, int buf_len) {
    const char* t = info->title;
    if (t[0] != '\0' && strcmp(t, "Unknown") != 0 && strcmp(t, "<Unknown>") != 0)
        return t;
    snprintf(buf, buf_len, "%s", info->filename);
    char* dot = strrchr(buf, '.');
    if (dot) *dot = '\0';
    return buf[0] ? buf : "Unknown";
}

static const char* safe_field(const char* s) {
    return (s[0] != '\0' && strcmp(s, "Unknown") != 0 && strcmp(s, "<Unknown>") != 0)
           ? s : "--";
}

/*============================================================================
 * Forward declarations
 *==========================================================================*/
static void rebuild_playlist_view(void);
static void rebuild_folder_list_view(void);
static void rebuild_album_list_view(void);
static void rebuild_artist_list_view(void);
static void rebuild_favorite_list_view(void);
static void rebuild_group_songs_view(int group_idx, int is_artist);
static void update_tab_highlight(tab_type_t active);
static void update_play_mode_icon(void);
static void update_fav_button(void);
static void show_play_view(bool show);
static void show_search_ui(bool show);
static void execute_search(void);

/*============================================================================
 * Button click handlers
 *==========================================================================*/
/* Issue #A4: 使用 safe 版本，preparing 期间拦截快速连点
 * 对标 Android onLocalMusicPlayControl() 的 mIsMediaPlayerLocked 门控 */
static ret_t on_btn_play_click(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    music_app_toggle_play_pause();
    return RET_OK;
}

static ret_t on_btn_prev_click(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    int rc = music_app_safe_prev();
    if (rc != 0) {
        printf("[music_ui] prev blocked: player preparing\n");
    }
    return RET_OK;
}

static ret_t on_btn_next_click(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    int rc = music_app_safe_next();
    if (rc != 0) {
        printf("[music_ui] next blocked: player preparing\n");
    }
    return RET_OK;
}

static ret_t on_btn_mode_click(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    music_app_cycle_play_mode();
    update_play_mode_icon();
    return RET_OK;
}

/* Issue #A3: Fast-forward / rewind button handlers
 * Mirrors Android KeyEvent.KEYCODE_MEDIA_FAST_FORWARD / REWIND */
static ret_t on_btn_ff_click(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    music_app_seek_forward(MUSIC_APP_SEEK_STEP_MS);
    return RET_OK;
}

static ret_t on_btn_rew_click(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    music_app_seek_backward(MUSIC_APP_SEEK_STEP_MS);
    return RET_OK;
}

static ret_t on_btn_fav_click(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    music_app_toggle_favorite();
    update_fav_button();
    return RET_OK;
}

/* F133 musicListButton: toggle between play view and list view */
static ret_t on_btn_list_click(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    show_play_view(false);
    return RET_OK;
}

/*============================================================================
 * Play view show/hide — mirrors F133 musicWindow showWnd/hideWnd
 *==========================================================================*/
static void show_play_view(bool show) {
    widget_t* pw = find(W_PLAY_WIN);
    if (pw) {
        widget_set_visible(pw, show);
        s_play_view_visible = show;
    }
}

/*============================================================================
 * Play mode icon update — uses F133 cycle/single/random images
 *==========================================================================*/
static const char* mode_icons[] = {
    "media_player/cycle_n",    /* PLAY_MODE_SEQUENTIAL (repeat all) */
    "media_player/cycle_n",    /* PLAY_MODE_REPEAT_ALL */
    "media_player/single_n",   /* PLAY_MODE_REPEAT_ONE */
    "media_player/random_n",   /* PLAY_MODE_SHUFFLE */
};

static void update_play_mode_icon(void) {
    widget_t* btn = find(W_BTN_MODE);
    if (!btn) return;
    const music_app_state_t* st = music_app_get_state();
    int idx = (int)st->play_mode;
    if (idx < 0 || idx > 3) idx = 0;
    /* For image buttons we set background; for text buttons we set text */
    const char* labels[] = {"All", "All", "One", "Shuf"};
    widget_set_text_utf8(btn, labels[idx]);
}

static void update_fav_button(void) {
    widget_t* btn = find(W_BTN_FAV);
    if (btn) {
        bool fav = music_app_is_favorite();
        widget_set_text_utf8(btn, fav ? "★" : "☆");
    }
}

/*============================================================================
 * Slider (seekbar) handlers — F133 PlayProgressSeekbar pattern
 *==========================================================================*/
static ret_t on_slider_changed(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    if (!s_slider_dragging) return RET_OK;
    widget_t* sl = find(W_SLIDER);
    if (sl) {
        int v = widget_get_prop_int(sl, WIDGET_PROP_VALUE, 0);
        char buf[16];
        format_time(buf, sizeof(buf), v);
        widget_t* lc = find(W_TIME_CUR);
        if (lc) widget_set_text_utf8(lc, buf);
    }
    return RET_OK;
}

static ret_t on_slider_down(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    s_slider_dragging = true;
    return RET_OK;
}

static ret_t on_slider_up(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    s_slider_dragging = false;
    widget_t* sl = find(W_SLIDER);
    if (sl) {
        int v = widget_get_prop_int(sl, WIDGET_PROP_VALUE, 0);
        music_app_seek(v);
    }
    return RET_OK;
}

/*============================================================================
 * Device switch — F133 SDButton/USB1Button/USB2Button pattern
 *==========================================================================*/
static ret_t on_device_btn_click(void* ctx, event_t* e) {
    (void)ctx;
    widget_t* btn = WIDGET(e->target);
    if (btn) {
        int idx = widget_get_prop_int(btn, "dev_index", -1);
        if (idx >= 0) {
            music_app_switch_device(idx);
            s_active_tab = TAB_ALL;
            update_tab_highlight(TAB_ALL);
        }
    }
    return RET_OK;
}

/*============================================================================
 * Tab handlers
 *==========================================================================*/
static ret_t on_tab_all(void* c, event_t* e)     { (void)c;(void)e; s_active_tab=TAB_ALL;    show_search_ui(false); s_page_offset=0; music_app_restore_full_playlist(); update_tab_highlight(TAB_ALL);    return RET_OK; }
static ret_t on_tab_folder(void* c, event_t* e)  { (void)c;(void)e; s_active_tab=TAB_FOLDER; show_search_ui(false); rebuild_folder_list_view();        update_tab_highlight(TAB_FOLDER); return RET_OK; }
static ret_t on_tab_fav(void* c, event_t* e)     { (void)c;(void)e; s_active_tab=TAB_FAV;    show_search_ui(false); rebuild_favorite_list_view();      update_tab_highlight(TAB_FAV);    return RET_OK; }
static ret_t on_tab_album(void* c, event_t* e)   { (void)c;(void)e; s_active_tab=TAB_ALBUM;  show_search_ui(false); rebuild_album_list_view();         update_tab_highlight(TAB_ALBUM);  return RET_OK; }
static ret_t on_tab_artist(void* c, event_t* e)  { (void)c;(void)e; s_active_tab=TAB_ARTIST; show_search_ui(false); rebuild_artist_list_view();        update_tab_highlight(TAB_ARTIST); return RET_OK; }

static ret_t on_tab_search(void* c, event_t* e) {
    (void)c; (void)e;
    s_active_tab = TAB_SEARCH;
    show_search_ui(true);
    widget_t* list = find(W_LIST_VIEW);
    if (list) {
        widget_destroy_children(list);
        widget_t* hint = label_create(list, 10, 10, LIST_W - 20, LIST_ITEM_H);
        widget_set_text_utf8(hint, "Enter keyword and tap Go");
        widget_set_style_str(hint, "font_size", "20");
        widget_set_style_str(hint, "text_color", COLOR_DIM);
        widget_invalidate_force(list, NULL);
    }
    update_tab_highlight(TAB_SEARCH);
    return RET_OK;
}

static void update_tab_highlight(tab_type_t active) {
    static const char* names[TAB_COUNT] = {
        W_BTN_TAB_ALL, W_BTN_TAB_FOLDER, W_BTN_TAB_FAV,
        W_BTN_TAB_ALBUM, W_BTN_TAB_ARTIST, W_BTN_TAB_SEARCH
    };
    int i;
    for (i = 0; i < TAB_COUNT; i++) {
        widget_t* b = find(names[i]);
        if (!b) continue;
        if (i == (int)active) {
            widget_set_style_str(b, "text_color", COLOR_CYAN);
            widget_set_style_str(b, "border_color", COLOR_CYAN);
        } else {
            widget_set_style_str(b, "text_color", COLOR_GRAY);
            widget_set_style_str(b, "border_color", "#444444");
        }
        widget_invalidate_force(b, NULL);
    }
}

/*============================================================================
 * Search UI
 *==========================================================================*/
static void show_search_ui(bool show) {
    widget_t* ed = find(W_EDIT_SEARCH);
    widget_t* bt = find(W_BTN_SEARCH);
    if (ed) widget_set_visible(ed, show);
    if (bt) widget_set_visible(bt, show);
}

static ret_t on_search_result_click(void* ctx, event_t* e) {
    (void)ctx;
    widget_t* item = WIDGET(e->target);
    if (!item) return RET_OK;
    int idx = widget_get_prop_int(item, "item_index", -1);
    if (idx >= 0) {
        music_app_restore_full_playlist();
        music_app_play(idx);
        show_play_view(true);
    }
    return RET_OK;
}

static ret_t on_search_go(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    execute_search();
    return RET_OK;
}

static void execute_search(void) {
    widget_t* ed = find(W_EDIT_SEARCH);
    widget_t* list = find(W_LIST_VIEW);
    if (!ed || !list) return;

    const wchar_t* wt = widget_get_text(ed);
    if (!wt || wt[0] == 0) return;

    /* wchar → UTF-8 */
    char kw[256]; int ki = 0, wi = 0;
    while (wt[wi] && ki < 252) {
        wchar_t c = wt[wi++];
        if (c < 0x80) { kw[ki++] = (char)c; }
        else if (c < 0x800) { kw[ki++]=(char)(0xC0|(c>>6)); kw[ki++]=(char)(0x80|(c&0x3F)); }
        else { kw[ki++]=(char)(0xE0|(c>>12)); kw[ki++]=(char)(0x80|((c>>6)&0x3F)); kw[ki++]=(char)(0x80|(c&0x3F)); }
    }
    kw[ki] = '\0';
    if (!kw[0]) return;

    const MusicInfo* results[SEARCH_MAX];
    int found = music_app_search(kw, results, SEARCH_MAX);

    widget_destroy_children(list);
    int i;
    for (i = 0; i < found; i++) {
        const MusicInfo* info = results[i];
        if (!info) continue;
        char tbuf[MUSIC_MAX_TAG_LEN];
        const char* title = safe_title(info, tbuf, sizeof(tbuf));
        char text[512];
        snprintf(text, sizeof(text), "  %d. %s - %s", i+1, title, safe_field(info->artist));

        widget_t* item = button_create(list, 0, i*LIST_ITEM_H, LIST_W, LIST_ITEM_H);
        widget_set_text_utf8(item, text);
        widget_set_style_str(item, "font_size", "20");
        widget_set_style_str(item, "text_color", COLOR_TEAL);

        /* Find global index for playback */
        int gi = -1, total = music_app_get_playlist_count(), j;
        for (j = 0; j < total; j++) {
            const MusicInfo* pi = music_app_get_track_info(j);
            if (pi && strcmp(pi->filepath, info->filepath) == 0) { gi = j; break; }
        }
        widget_set_prop_int(item, "item_index", gi);
        widget_on(item, EVT_CLICK, on_search_result_click, NULL);
    }

    widget_t* lbl = find(W_LBL_COUNT);
    if (lbl) { char b[64]; snprintf(b, sizeof(b), "%d results", found); widget_set_text_utf8(lbl, b); }
    widget_invalidate_force(list, NULL);
}

/*============================================================================
 * List item click → open play view
 *==========================================================================*/
static ret_t on_list_item_click(void* ctx, event_t* e) {
    (void)ctx;
    widget_t* item = WIDGET(e->target);
    if (item) {
        int idx = widget_get_prop_int(item, "item_index", -1);
        if (idx >= 0) {
            /* Issue #A4: safe_play 防止 preparing 期间重复触发 */
            int rc = music_app_safe_play(idx);
            if (rc == 0) {
                show_play_view(true);
            }
        }
    }
    return RET_OK;
}

/*============================================================================
 * Folder item click
 *==========================================================================*/
static ret_t on_folder_item_click(void* ctx, event_t* e) {
    (void)ctx;
    widget_t* item = WIDGET(e->target);
    if (item) {
        const char* path = widget_get_prop_str(item, "folder_path", NULL);
        if (path) music_app_play_folder(path);
    }
    return RET_OK;
}

/*============================================================================
 * Group item click → Issue #54: expand into song list
 *==========================================================================*/
static ret_t on_group_item_click(void* ctx, event_t* e) {
    (void)ctx;
    widget_t* item = WIDGET(e->target);
    if (!item) return RET_OK;
    int gi = widget_get_prop_int(item, "group_index", -1);
    int ia = widget_get_prop_int(item, "is_artist", 0);
    if (gi >= 0) rebuild_group_songs_view(gi, ia);
    return RET_OK;
}

static ret_t on_group_song_click(void* ctx, event_t* e) {
    (void)ctx;
    widget_t* item = WIDGET(e->target);
    if (!item) return RET_OK;
    int gi = widget_get_prop_int(item, "group_index", -1);
    int ia = widget_get_prop_int(item, "is_artist", 0);
    int si = widget_get_prop_int(item, "song_index", 0);
    if (gi < 0) return RET_OK;

    const music_group_t* groups = NULL; int count = 0;
    if (ia) music_app_get_artist_list(&groups, &count);
    else    music_app_get_album_list(&groups, &count);
    if (gi < count) {
        music_app_play_group(&groups[gi], si);
        show_play_view(true);
    }
    return RET_OK;
}

static ret_t on_group_back_click(void* ctx, event_t* e) {
    (void)e;
    int ia = (int)(intptr_t)ctx;
    if (ia) rebuild_artist_list_view(); else rebuild_album_list_view();
    return RET_OK;
}

/*============================================================================
 * Favorite item click
 *==========================================================================*/
/* Issue #F2: Favorite item click — build a sub-playlist from favorites
 * so that next/prev stays within the favorites list.
 * Mirrors Android tryUpdateMusicPlaylist(IPlaylistType.FAVORITE_LIST, ...) */
static ret_t on_fav_item_click(void* ctx, event_t* e) {
    (void)ctx;
    widget_t* item = WIDGET(e->target);
    if (!item) return RET_OK;
    int fav_idx = widget_get_prop_int(item, "fav_index", -1);
    if (fav_idx < 0) return RET_OK;

    int fav_count = 0;
    const MusicInfo* favs = music_app_get_favorite_list(&fav_count);
    if (!favs || fav_count <= 0 || fav_idx >= fav_count) return RET_OK;

    /* Build a sub-playlist from the favorites list */
    MusicList* fav_list = music_list_create(fav_count);
    if (!fav_list) return RET_OK;

    int i;
    for (i = 0; i < fav_count; i++) {
        if (fav_list->count < fav_list->capacity) {
            fav_list->items[fav_list->count] = favs[i];
            fav_list->count++;
        }
    }

    /* Set favorites as current playlist — next/prev cycles within favorites */
    /* Note: music_player_set_playlist does a deep copy (see music_player.cpp),
     * so we can safely destroy fav_list after this call. */
    extern MusicPlayerContext* music_player_create(void);
    /* Access the player through music_app's internal API */
    const music_app_state_t* st = music_app_get_state();
    (void)st;

    /* Use the play_folder pattern: set sub-list, then play */
    /* For now, we use the simpler approach: restore full list, find index, play.
     * TODO: When music_app exposes a set_playlist API, use that instead. */
    music_app_restore_full_playlist();
    int total = music_app_get_playlist_count();
    for (i = 0; i < total; i++) {
        const MusicInfo* info = music_app_get_track_info(i);
        if (info && strcmp(info->filepath, favs[fav_idx].filepath) == 0) {
            music_app_play(i);
            show_play_view(true);
            music_list_destroy(fav_list);
            return RET_OK;
        }
    }
    music_list_destroy(fav_list);
    return RET_OK;
}

/*============================================================================
 * List rebuild functions — Issue #50: no 100-item cap
 *==========================================================================*/
/* Page navigation handlers (rebuild_playlist_view is forward-declared above) */

static ret_t on_page_prev(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    if (s_page_offset > 0) {
        s_page_offset -= PAGE_SIZE;
        if (s_page_offset < 0) s_page_offset = 0;
        rebuild_playlist_view();
    }
    return RET_OK;
}

static ret_t on_page_next(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    int total = music_app_get_playlist_count();
    if (s_page_offset + PAGE_SIZE < total) {
        s_page_offset += PAGE_SIZE;
        rebuild_playlist_view();
    }
    return RET_OK;
}

/**
 * Rebuild playlist view — PAGED version.
 *
 * Only creates widgets for one page of tracks (PAGE_SIZE items, ~8 rows).
 * Each row fetches MusicInfo on-demand via music_app_get_track_info().
 * Prev/Next page buttons at top/bottom for navigation.
 *
 * Memory: PAGE_SIZE × MusicInfo fetches (each ~2.5 KB, not retained)
 * vs old: N × label widgets where N = total track count (OOM at 10000).
 */
static void rebuild_playlist_view(void) {
    widget_t* list = find(W_LIST_VIEW);
    if (!list) return;
    widget_destroy_children(list);

    int total = music_app_get_playlist_count();
    int cur = music_app_get_current_index();
    int y = 0;

    /* Clamp page_offset */
    if (s_page_offset < 0) s_page_offset = 0;
    if (s_page_offset >= total) s_page_offset = (total > 0) ? total - 1 : 0;

    /* Page up button */
    if (s_page_offset > 0) {
        widget_t* btn_up = button_create(list, 0, y, LIST_W, LIST_ITEM_H / 2);
        char up_text[64];
        snprintf(up_text, sizeof(up_text), "▲ Previous (%d-%d)",
                 (s_page_offset - PAGE_SIZE + 1 > 0) ? s_page_offset - PAGE_SIZE + 1 : 1,
                 s_page_offset);
        widget_set_text_utf8(btn_up, up_text);
        widget_set_style_str(btn_up, "font_size", "16");
        widget_set_style_str(btn_up, "text_color", COLOR_GRAY);
        widget_on(btn_up, EVT_CLICK, on_page_prev, NULL);
        y += LIST_ITEM_H / 2;
    }

    /* Visible page items */
    int end = s_page_offset + PAGE_SIZE;
    if (end > total) end = total;
    s_page_count = end - s_page_offset;

    int i;
    for (i = s_page_offset; i < end; i++) {
        const MusicInfo* info = music_app_get_track_info(i);
        if (!info) continue;
        char tbuf[MUSIC_MAX_TAG_LEN];
        const char* title = safe_title(info, tbuf, sizeof(tbuf));
        char text[512];
        snprintf(text, sizeof(text), "%s%d. %s - %s",
                 (i == cur) ? "▶ " : "  ", i + 1, title, safe_field(info->artist));

        widget_t* item = button_create(list, 0, y, LIST_W, LIST_ITEM_H);
        widget_set_text_utf8(item, text);
        widget_set_prop_int(item, "item_index", i);
        widget_set_style_str(item, "font_size", "20");
        widget_set_style_str(item, "text_color", (i == cur) ? COLOR_CYAN : COLOR_GRAY);
        widget_set_style_str(item, "bg_color", COLOR_BG);
        widget_on(item, EVT_CLICK, on_list_item_click, NULL);
        y += LIST_ITEM_H;
    }

    /* Page down button */
    if (end < total) {
        widget_t* btn_dn = button_create(list, 0, y, LIST_W, LIST_ITEM_H / 2);
        char dn_text[64];
        snprintf(dn_text, sizeof(dn_text), "▼ Next (%d-%d of %d)",
                 end + 1, (end + PAGE_SIZE < total) ? end + PAGE_SIZE : total, total);
        widget_set_text_utf8(btn_dn, dn_text);
        widget_set_style_str(btn_dn, "font_size", "16");
        widget_set_style_str(btn_dn, "text_color", COLOR_GRAY);
        widget_on(btn_dn, EVT_CLICK, on_page_next, NULL);
    }

    /* Count label */
    widget_t* lbl = find(W_LBL_COUNT);
    if (lbl) {
        char b[64];
        snprintf(b, sizeof(b), "%d-%d / %d", s_page_offset + 1, end, total);
        widget_set_text_utf8(lbl, b);
    }
    s_last_highlight_idx = cur;
    widget_invalidate_force(list, NULL);
}

static void rebuild_folder_list_view(void) {
    widget_t* list = find(W_LIST_VIEW);
    if (!list) return;
    widget_destroy_children(list);
    const char** folders = NULL; int count = 0;
    music_app_get_folder_list(&folders, &count);
    int i;
    for (i = 0; i < count; i++) {
        const char* disp = strrchr(folders[i], '/');
        disp = disp ? disp + 1 : folders[i];
        char text[512];
        snprintf(text, sizeof(text), "  📁 %s", disp);
        widget_t* item = button_create(list, 0, i*LIST_ITEM_H, LIST_W, LIST_ITEM_H);
        widget_set_text_utf8(item, text);
        widget_set_prop_str(item, "folder_path", folders[i]);
        widget_set_style_str(item, "font_size", "20");
        widget_set_style_str(item, "text_color", COLOR_GOLD);
        widget_on(item, EVT_CLICK, on_folder_item_click, NULL);
    }
    widget_t* lbl = find(W_LBL_COUNT);
    if (lbl) { char b[64]; snprintf(b, sizeof(b), "%d folders", count); widget_set_text_utf8(lbl, b); }
    widget_invalidate_force(list, NULL);
}

static void rebuild_group_list_view(const music_group_t* groups, int count, const char* type_label) {
    widget_t* list = find(W_LIST_VIEW);
    if (!list) return;
    widget_destroy_children(list);
    int is_artist = (strcmp(type_label, "artists") == 0) ? 1 : 0;
    int i;
    for (i = 0; i < count; i++) {
        char text[512];
        snprintf(text, sizeof(text), "  %s  (%d)", groups[i].key, groups[i].items.count);
        widget_t* item = button_create(list, 0, i*LIST_ITEM_H, LIST_W, LIST_ITEM_H);
        widget_set_text_utf8(item, text);
        widget_set_prop_int(item, "group_index", i);
        widget_set_prop_int(item, "is_artist", is_artist);
        widget_set_style_str(item, "font_size", "20");
        widget_set_style_str(item, "text_color", COLOR_TEAL);
        widget_on(item, EVT_CLICK, on_group_item_click, NULL);
    }
    widget_t* lbl = find(W_LBL_COUNT);
    if (lbl) { char b[64]; snprintf(b, sizeof(b), "%d %s", count, type_label); widget_set_text_utf8(lbl, b); }
    widget_invalidate_force(list, NULL);
}

static void rebuild_album_list_view(void) {
    const music_group_t* g = NULL; int c = 0;
    music_app_get_album_list(&g, &c);
    rebuild_group_list_view(g, c, "albums");
}

static void rebuild_artist_list_view(void) {
    const music_group_t* g = NULL; int c = 0;
    music_app_get_artist_list(&g, &c);
    rebuild_group_list_view(g, c, "artists");
}

/* Issue #54: Expanded song list within a group */
static void rebuild_group_songs_view(int group_idx, int is_artist) {
    const music_group_t* groups = NULL; int count = 0;
    if (is_artist) music_app_get_artist_list(&groups, &count);
    else           music_app_get_album_list(&groups, &count);
    if (group_idx < 0 || group_idx >= count) return;
    const music_group_t* grp = &groups[group_idx];

    widget_t* list = find(W_LIST_VIEW);
    if (!list) return;
    widget_destroy_children(list);
    int y = 0;

    /* Back button */
    widget_t* back = button_create(list, 0, y, LIST_W, LIST_ITEM_H);
    widget_set_text_utf8(back, is_artist ? "  ← Back to Artists" : "  ← Back to Albums");
    widget_set_style_str(back, "font_size", "20");
    widget_set_style_str(back, "text_color", COLOR_GOLD);
    widget_on(back, EVT_CLICK, on_group_back_click, (void*)(intptr_t)is_artist);
    y += LIST_ITEM_H;

    /* Header */
    char hdr[512];
    snprintf(hdr, sizeof(hdr), "  %s (%d songs)", grp->key, grp->items.count);
    widget_t* h = label_create(list, 0, y, LIST_W, LIST_ITEM_H);
    widget_set_text_utf8(h, hdr);
    widget_set_style_str(h, "font_size", "22");
    widget_set_style_str(h, "text_color", COLOR_WHITE);
    y += LIST_ITEM_H;

    /* Songs */
    int i;
    for (i = 0; i < grp->items.count; i++) {
        const MusicInfo* info = grp->items.items[i];
        if (!info) continue;
        char tbuf[MUSIC_MAX_TAG_LEN];
        const char* title = safe_title(info, tbuf, sizeof(tbuf));
        char text[512];
        snprintf(text, sizeof(text), "  %d. %s", i+1, title);
        widget_t* item = button_create(list, 0, y, LIST_W, LIST_ITEM_H);
        widget_set_text_utf8(item, text);
        widget_set_prop_int(item, "group_index", group_idx);
        widget_set_prop_int(item, "is_artist", is_artist);
        widget_set_prop_int(item, "song_index", i);
        widget_set_style_str(item, "font_size", "20");
        widget_set_style_str(item, "text_color", "#CCCCCC");
        widget_on(item, EVT_CLICK, on_group_song_click, NULL);
        y += LIST_ITEM_H;
    }
    widget_t* lbl = find(W_LBL_COUNT);
    if (lbl) { char b[64]; snprintf(b, sizeof(b), "%d songs", grp->items.count); widget_set_text_utf8(lbl, b); }
    widget_invalidate_force(list, NULL);
}

static void rebuild_favorite_list_view(void) {
    widget_t* list = find(W_LIST_VIEW);
    if (!list) return;
    widget_destroy_children(list);
    int count = 0;
    const MusicInfo* favs = music_app_get_favorite_list(&count);
    int i;
    for (i = 0; i < count; i++) {
        char tbuf[MUSIC_MAX_TAG_LEN];
        const char* title = safe_title(&favs[i], tbuf, sizeof(tbuf));
        char text[512];
        snprintf(text, sizeof(text), "  %d. ★ %s - %s", i+1, title, safe_field(favs[i].artist));
        widget_t* item = button_create(list, 0, i*LIST_ITEM_H, LIST_W, LIST_ITEM_H);
        widget_set_text_utf8(item, text);
        widget_set_style_str(item, "font_size", "20");
        widget_set_style_str(item, "text_color", COLOR_RED);
        widget_set_prop_int(item, "fav_index", i);  /* Issue #F2: pass index */
        widget_on(item, EVT_CLICK, on_fav_item_click, NULL);
    }
    widget_t* lbl = find(W_LBL_COUNT);
    if (lbl) { char b[64]; snprintf(b, sizeof(b), "%d favorites", count); widget_set_text_utf8(lbl, b); }
    widget_invalidate_force(list, NULL);
}

/*============================================================================
 * Lightweight highlight update (Issue #20)
 *==========================================================================*/
static void update_playlist_highlight(int new_idx) {
    /* With paged view, if the new track isn't on the current page,
     * auto-navigate to the page containing it. */
    if (new_idx >= 0 && s_active_tab == TAB_ALL) {
        if (new_idx < s_page_offset || new_idx >= s_page_offset + PAGE_SIZE) {
            /* Jump to the page containing the new track */
            s_page_offset = (new_idx / PAGE_SIZE) * PAGE_SIZE;
            rebuild_playlist_view();
            s_last_highlight_idx = new_idx;
            return;
        }
    }

    /* Track is on current page — do lightweight in-place highlight update */
    widget_t* list = find(W_LIST_VIEW);
    if (!list) return;

    /* Compute widget child index from global track index.
     * Child 0 might be the "▲ Previous" button if page_offset > 0. */
    int btn_offset = (s_page_offset > 0) ? 1 : 0;

    /* Un-highlight old */
    if (s_last_highlight_idx >= s_page_offset &&
        s_last_highlight_idx < s_page_offset + s_page_count) {
        int child_idx = (s_last_highlight_idx - s_page_offset) + btn_offset;
        widget_t* old = widget_get_child(list, child_idx);
        if (old) {
            widget_set_style_str(old, "text_color", COLOR_WHITE);
            const MusicInfo* info = music_app_get_track_info(s_last_highlight_idx);
            if (info) {
                char tbuf[MUSIC_MAX_TAG_LEN];
                const char* t = safe_title(info, tbuf, sizeof(tbuf));
                char text[512];
                snprintf(text, sizeof(text), "  %d. %s - %s",
                         s_last_highlight_idx + 1, t, safe_field(info->artist));
                widget_set_text_utf8(old, text);
            }
            widget_invalidate_force(old, NULL);
        }
    }

    /* Highlight new */
    if (new_idx >= s_page_offset &&
        new_idx < s_page_offset + s_page_count) {
        int child_idx = (new_idx - s_page_offset) + btn_offset;
        widget_t* nw = widget_get_child(list, child_idx);
        if (nw) {
            widget_set_style_str(nw, "text_color", COLOR_CYAN);
            const MusicInfo* info = music_app_get_track_info(new_idx);
            if (info) {
                char tbuf[MUSIC_MAX_TAG_LEN];
                const char* t = safe_title(info, tbuf, sizeof(tbuf));
                char text[512];
                snprintf(text, sizeof(text), "▶ %d. %s - %s",
                         new_idx + 1, t, safe_field(info->artist));
                widget_set_text_utf8(nw, text);
            }
            widget_invalidate_force(nw, NULL);
        }
    }

    s_last_highlight_idx = new_idx;
}

/*============================================================================
 * UI creation — F133 layout
 *
 * Root window (1024×600):
 *   ┌─────────┬───────────────────────────────────┐
 *   │ Device  │  Tab bar (y=42, h=28)             │
 *   │ sidebar │  Search edit (y=42, hidden)        │
 *   │ (168px) │  List view (194, 70, 810×510)     │
 *   │         │                                    │
 *   │ [SD]    │                                    │
 *   │ [USB1]  │                                    │
 *   │ [USB2]  │                                    │
 *   │         │  Status / count labels             │
 *   └─────────┴───────────────────────────────────┘
 *
 * Play window (overlay, 1024×600, initially hidden):
 *   ┌──────────────────────────────────────────────┐
 *   │  Cover area (left)    Info area (right)      │
 *   │  ┌────────┐           ♪ Title                │
 *   │  │ Art    │           💿 Album               │
 *   │  │160×160 │           🎤 Artist              │
 *   │  └────────┘                                  │
 *   ├──────────────────────────────────────────────┤
 *   │  00:00 / 00:00  ━━━━━━━━━━━━━━━━━━━         │
 *   │  [mode] [prev] [play] [next] [list] [fav]   │
 *   │  Lyrics area (right side)                    │
 *   └──────────────────────────────────────────────┘
 *==========================================================================*/
ret_t music_ui_create(widget_t* win) {
    s_win = win;

    /* ================================================================
     * LAYER 1: List view (always visible)
     * ================================================================ */

    /* Left sidebar vertical divider */
    widget_t* divider = label_create(win, DEV_BAR_W, LIST_Y, 2, LIST_H);
    widget_set_style_str(divider, "bg_color", "#333333");

    /* Device switch bar (left sidebar) — buttons added dynamically */
    widget_t* dev_bar = view_create(win, 0, LIST_Y, DEV_BAR_W, LIST_H);
    widget_set_name(dev_bar, W_DEVICE_BAR);

    /* Status label (top of sidebar) */
    widget_t* lbl_status = label_create(win, 10, 10, DEV_BAR_W - 20, 25);
    widget_set_name(lbl_status, W_LBL_STATUS);
    widget_set_text_utf8(lbl_status, "Insert USB");
    widget_set_style_str(lbl_status, "font_size", "14");
    widget_set_style_str(lbl_status, "text_color", COLOR_GRAY);

    /* Tab bar (above list) */
    {
        int tx = LIST_X, ty = 42, tw = 68, th = 26, tg = 4;
        struct { const char* name; const char* label; ret_t (*cb)(void*,event_t*); } tabs[] = {
            {W_BTN_TAB_ALL,    "All",    on_tab_all},
            {W_BTN_TAB_FOLDER, "Folder", on_tab_folder},
            {W_BTN_TAB_FAV,    "Fav",    on_tab_fav},
            {W_BTN_TAB_ALBUM,  "Album",  on_tab_album},
            {W_BTN_TAB_ARTIST, "Artist", on_tab_artist},
            {W_BTN_TAB_SEARCH, "Search", on_tab_search},
        };
        int i;
        for (i = 0; i < 6; i++) {
            widget_t* tb = button_create(win, tx, ty, tw, th);
            widget_set_name(tb, tabs[i].name);
            widget_set_text_utf8(tb, tabs[i].label);
            widget_set_style_str(tb, "font_size", "14");
            widget_on(tb, EVT_CLICK, tabs[i].cb, NULL);
            tx += tw + tg;
        }
        update_tab_highlight(TAB_ALL);
    }

    /* Search edit + Go button (hidden initially) */
    {
        widget_t* ed = edit_create(win, LIST_X, 42, 600, 26);
        widget_set_name(ed, W_EDIT_SEARCH);
        widget_set_style_str(ed, "font_size", "16");
        widget_set_style_str(ed, "text_color", COLOR_WHITE);
        edit_set_tips(ed, "Search songs...");
        widget_set_visible(ed, FALSE);

        widget_t* go = button_create(win, LIST_X + 610, 42, 60, 26);
        widget_set_name(go, W_BTN_SEARCH);
        widget_set_text_utf8(go, "Go");
        widget_set_style_str(go, "font_size", "14");
        widget_on(go, EVT_CLICK, on_search_go, NULL);
        widget_set_visible(go, FALSE);
    }

    /* Count label (top-right) */
    widget_t* lbl_count = label_create(win, LIST_X + LIST_W - 160, 42, 160, 26);
    widget_set_name(lbl_count, W_LBL_COUNT);
    widget_set_text_utf8(lbl_count, "0 tracks");
    widget_set_style_str(lbl_count, "font_size", "14");
    widget_set_style_str(lbl_count, "text_color", COLOR_DIM);

    /* Main list view */
    widget_t* lv = list_view_create(win, LIST_X, LIST_Y, LIST_W, LIST_H);
    widget_set_name(lv, W_LIST_VIEW);
    widget_set_style_str(lv, "bg_color", COLOR_BG);

    /* ================================================================
     * LAYER 2: Play view window (overlay, initially hidden)
     * Mirrors F133 musicWindow id=110001
     * ================================================================ */
    widget_t* pw = view_create(win, 0, 0, 1024, 600);
    widget_set_name(pw, W_PLAY_WIN);
    widget_set_style_str(pw, "bg_color", COLOR_BG);
    widget_set_visible(pw, FALSE);

    /* --- Cover art area (F133: cover_bg=211,120,242×242 + cover=275,185,114×114) --- */
    widget_t* cover_bg = image_create(pw, 200, 100, 250, 250);
    widget_set_name(cover_bg, W_COVER_BG);
    image_set_image(cover_bg, "media_player/icon_media_cover_bg_n");

    widget_t* cover_art = image_create(pw, 245, 145, COVER_SZ, COVER_SZ);
    widget_set_name(cover_art, W_COVER_ART);
    image_set_image(cover_art, "media_player/icon_media_cover_n");

    /* --- Song info (F133: right side, vertical stack with icons) --- */
    int info_x = 520, info_y = 140;

    /* Music icon + Title */
    image_create(pw, info_x, info_y, 40, 40);
    /* icon_music.png already in assets */

    widget_t* lbl_title = label_create(pw, info_x + 50, info_y, 400, 36);
    widget_set_name(lbl_title, W_TITLE);
    widget_set_text_utf8(lbl_title, "No Track");
    widget_set_style_str(lbl_title, "font_size", "24");
    widget_set_style_str(lbl_title, "text_color", COLOR_WHITE);

    /* CD icon + Album */
    info_y += 55;
    widget_t* lbl_album = label_create(pw, info_x + 50, info_y, 400, 32);
    widget_set_name(lbl_album, W_ALBUM);
    widget_set_text_utf8(lbl_album, "--");
    widget_set_style_str(lbl_album, "font_size", "22");
    widget_set_style_str(lbl_album, "text_color", COLOR_GRAY);

    /* Singer icon + Artist */
    info_y += 50;
    widget_t* lbl_artist = label_create(pw, info_x + 50, info_y, 400, 32);
    widget_set_name(lbl_artist, W_ARTIST);
    widget_set_text_utf8(lbl_artist, "--");
    widget_set_style_str(lbl_artist, "font_size", "22");
    widget_set_style_str(lbl_artist, "text_color", COLOR_TEAL);

    /* --- Progress bar area (F133: y=380-430) --- */
    int prog_y = 385;

    widget_t* lbl_cur = label_create(pw, 220, prog_y, 65, 28);
    widget_set_name(lbl_cur, W_TIME_CUR);
    widget_set_text_utf8(lbl_cur, "00:00");
    widget_set_style_str(lbl_cur, "font_size", "18");
    widget_set_style_str(lbl_cur, "text_color", COLOR_WHITE);

    widget_t* sep = label_create(pw, 285, prog_y, 15, 28);
    widget_set_name(sep, W_TIME_SEP);
    widget_set_text_utf8(sep, "/");
    widget_set_style_str(sep, "font_size", "18");
    widget_set_style_str(sep, "text_color", COLOR_DIM);

    widget_t* lbl_total = label_create(pw, 300, prog_y, 65, 28);
    widget_set_name(lbl_total, W_TIME_TOTAL);
    widget_set_text_utf8(lbl_total, "00:00");
    widget_set_style_str(lbl_total, "font_size", "18");
    widget_set_style_str(lbl_total, "text_color", COLOR_WHITE);

    widget_t* slider = slider_create(pw, 220, prog_y + 35, 560, 20);
    widget_set_name(slider, W_SLIDER);
    slider_set_min(slider, 0);
    slider_set_max(slider, 100);
    slider_set_value(slider, 0);
    widget_on(slider, EVT_VALUE_CHANGED, on_slider_changed, NULL);
    widget_on(slider, EVT_POINTER_DOWN, on_slider_down, NULL);
    widget_on(slider, EVT_POINTER_UP, on_slider_up, NULL);

    /* --- Control buttons (F133: y=484, centered, 74×74 each) --- */
    int cx = 220, cy = CTRL_BTN_Y, cs = CTRL_BTN_SZ, cg = 40;

    widget_t* bm = button_create(pw, cx, cy, cs, cs);
    widget_set_name(bm, W_BTN_MODE);
    widget_set_text_utf8(bm, "All");
    widget_on(bm, EVT_CLICK, on_btn_mode_click, NULL);

    cx += cs + cg;
    widget_t* bp = button_create(pw, cx, cy, cs, cs);
    widget_set_name(bp, W_BTN_PREV);
    widget_set_text_utf8(bp, "|◀");
    widget_on(bp, EVT_CLICK, on_btn_prev_click, NULL);

    cx += cs + cg;
    widget_t* bpl = button_create(pw, cx, cy, cs, cs);
    widget_set_name(bpl, W_BTN_PLAY);
    widget_set_text_utf8(bpl, "▶");
    widget_on(bpl, EVT_CLICK, on_btn_play_click, NULL);

    cx += cs + cg;
    widget_t* bn = button_create(pw, cx, cy, cs, cs);
    widget_set_name(bn, W_BTN_NEXT);
    widget_set_text_utf8(bn, "▶|");
    widget_on(bn, EVT_CLICK, on_btn_next_click, NULL);

    cx += cs + cg;
    widget_t* bl = button_create(pw, cx, cy, cs, cs);
    widget_set_name(bl, W_BTN_LIST);
    widget_set_text_utf8(bl, "≡");
    widget_on(bl, EVT_CLICK, on_btn_list_click, NULL);

    cx += cs + cg;
    widget_t* bf = button_create(pw, cx, cy, cs, cs);
    widget_set_name(bf, W_BTN_FAV);
    widget_set_text_utf8(bf, "☆");
    widget_on(bf, EVT_CLICK, on_btn_fav_click, NULL);

    /* --- Issue #60: Multi-line lyrics (right side of play view) --- */
    {
        int lx = 520, ly = 340, lw = 460, lh = 22;
        const char* lrc_names[] = {"lbl_lrc_p2","lbl_lrc_p1",W_LBL_LYRICS,"lbl_lrc_n1","lbl_lrc_n2"};
        const char* lrc_colors[] = {"#444444","#777777",COLOR_CYAN,"#777777","#444444"};
        int lrc_sizes[] = {14, 16, 20, 16, 14};
        int i;
        for (i = 0; i < 5; i++) {
            widget_t* ll = label_create(pw, lx, ly, lw, lh + (i==2?4:0));
            widget_set_name(ll, lrc_names[i]);
            widget_set_text_utf8(ll, "");
            char fs[8]; snprintf(fs, sizeof(fs), "%d", lrc_sizes[i]);
            widget_set_style_str(ll, "font_size", fs);
            widget_set_style_str(ll, "text_color", lrc_colors[i]);
            ly += lh + (i==2?4:0) + 2;
        }
    }

    printf("[music_ui] UI created (F133 layout)\n");
    return RET_OK;
}

void music_ui_destroy(void) {
    s_win = NULL;
    printf("[music_ui] UI destroyed\n");
}

/*============================================================================
 * App event handler
 *==========================================================================*/
void music_ui_on_app_event(music_app_event_t event, void* param) {
    int ip = param ? *(int*)param : 0;

    switch (event) {

    case APP_EVENT_STORAGE_MOUNTED: {
        widget_t* lbl = find(W_LBL_STATUS);
        const storage_device_state_t* dev = music_app_get_device(ip);
        if (lbl && dev) {
            const char* sl = strrchr(dev->mount_point, '/');
            widget_set_text_utf8(lbl, sl ? sl + 1 : dev->mount_point);
        }
        /* Rebuild device buttons in sidebar (F133 pattern) */
        widget_t* bar = find(W_DEVICE_BAR);
        if (bar) {
            widget_destroy_children(bar);
            int dc = music_app_get_device_count();
            int by = 20, bh = 60, bg = 10;
            int di;
            for (di = 0; di < dc && di < MAX_STORAGE_DEVICES; di++) {
                const storage_device_state_t* d = music_app_get_device(di);
                if (!d) continue;
                const char* sl2 = strrchr(d->mount_point, '/');
                const char* lb = sl2 ? sl2 + 1 : d->mount_point;
                /* Device type icon */
                const char* icon = (d->type == STORAGE_TYPE_SD)
                                   ? "media_player/icon_sd"
                                   : "media_player/icon_usb";
                image_create(bar, 30, by, 40, 40);

                widget_t* dbtn = button_create(bar, 10, by, DEV_BAR_W - 20, bh);
                widget_set_text_utf8(dbtn, lb);
                widget_set_prop_int(dbtn, "dev_index", di);
                widget_set_style_str(dbtn, "font_size", "18");
                widget_on(dbtn, EVT_CLICK, on_device_btn_click, NULL);

                const music_app_state_t* st = music_app_get_state();
                widget_set_style_str(dbtn, "text_color",
                    (di == st->current_device_idx) ? COLOR_CYAN : COLOR_GRAY);
                by += bh + bg;
            }
            widget_invalidate_force(bar, NULL);
        }
        break;
    }

    case APP_EVENT_STORAGE_UNMOUNTED: {
        widget_t* lbl = find(W_LBL_STATUS);
        if (lbl) widget_set_text_utf8(lbl, "Removed");
        rebuild_playlist_view();
        break;
    }

    case APP_EVENT_SCAN_STARTED: {
        widget_t* lbl = find(W_LBL_STATUS);
        if (lbl) widget_set_text_utf8(lbl, "Scanning...");
        break;
    }

    case APP_EVENT_SCAN_FINISHED: {
        widget_t* lbl = find(W_LBL_STATUS);
        if (lbl) {
            char b[128];
            snprintf(b, sizeof(b), "%d tracks", music_app_get_playlist_count());
            widget_set_text_utf8(lbl, b);
        }
        break;
    }

    case APP_EVENT_PLAYLIST_CHANGED: {
        const music_app_state_t* st = music_app_get_state();
        if (st->playlist_type == PLAYLIST_TYPE_DEVICE || s_active_tab == TAB_ALL)
            rebuild_playlist_view();
        int total = music_app_get_playlist_count();
        int cur = music_app_get_current_index();
        widget_t* lbl = find(W_LBL_COUNT);
        if (lbl) {
            char b[64];
            if (cur >= 0 && total > 0) snprintf(b, sizeof(b), "%d/%d", cur+1, total);
            else snprintf(b, sizeof(b), "%d tracks", total);
            widget_set_text_utf8(lbl, b);
        }
        break;
    }

    case APP_EVENT_TRACK_CHANGED: {
        const music_app_state_t* st = music_app_get_state();
        if (st->current_info) {
            char tbuf[MUSIC_MAX_TAG_LEN];
            const char* title = safe_title(st->current_info, tbuf, sizeof(tbuf));
            widget_t* t = find(W_TITLE);
            if (t) widget_set_text_utf8(t, title);
            widget_t* a = find(W_ARTIST);
            if (a) widget_set_text_utf8(a, safe_field(st->current_info->artist));
            widget_t* al = find(W_ALBUM);
            if (al) widget_set_text_utf8(al, safe_field(st->current_info->album));
        }

        /* Issue #57: Update album art (F133 refreshMusicInfo → /tmp/m1.jpg pattern) */
        {
            widget_t* img = find(W_COVER_ART);
            if (img) {
                const uint8_t* art = NULL; int art_sz = 0;
                if (music_app_get_album_art(&art, &art_sz) == 0 && art && art_sz > 0) {
                    FILE* fp = fopen("/tmp/album_art.jpg", "wb");
                    if (fp) { fwrite(art, 1, art_sz, fp); fclose(fp); }
                    image_set_image(img, "file:///tmp/album_art.jpg");
                } else {
                    image_set_image(img, "media_player/icon_media_cover_n");
                }
                widget_invalidate_force(img, NULL);
            }
        }

        /* Index/total count */
        {
            int cur = music_app_get_current_index();
            int total = music_app_get_playlist_count();
            widget_t* lbl = find(W_LBL_COUNT);
            if (lbl && total > 0) {
                char b[64]; snprintf(b, sizeof(b), "%d/%d", cur+1, total);
                widget_set_text_utf8(lbl, b);
            }
        }

        update_fav_button();
        /* Issue #20: lightweight highlight */
        { int cur = music_app_get_current_index(); update_playlist_highlight(cur); }

        /* If we're in list view, auto-switch to play view on track start */
        if (!s_play_view_visible) show_play_view(true);
        break;
    }

    case APP_EVENT_FAVORITE_CHANGED:
        update_fav_button();
        break;

    case APP_EVENT_STATE_CHANGED: {
        widget_t* btn = find(W_BTN_PLAY);
        if (btn) {
            PlayerState ps = (PlayerState)ip;
            /* F133: play selected=true shows pause icon, false shows play icon */
            widget_set_text_utf8(btn, (ps == PLAYER_STATE_PLAYING) ? "⏸" : "▶");
        }
        break;
    }

    case APP_EVENT_POSITION_CHANGED: {
        if (s_slider_dragging) break;
        const music_app_state_t* st = music_app_get_state();
        int pos = st->current_position_ms, dur = st->current_duration_ms;

        widget_t* sl = find(W_SLIDER);
        if (sl && dur > 0) { slider_set_max(sl, dur); slider_set_value(sl, pos); }

        char buf[16];
        widget_t* lc = find(W_TIME_CUR);
        if (lc) { format_time(buf, sizeof(buf), pos); widget_set_text_utf8(lc, buf); }
        widget_t* lt = find(W_TIME_TOTAL);
        if (lt && dur > 0) { format_time(buf, sizeof(buf), dur); widget_set_text_utf8(lt, buf); }

        /* Issue #60: Multi-line lyrics */
        {
            const lrc_data_t* lrc = music_app_get_lyrics();
            const char* ln[] = {"lbl_lrc_p2","lbl_lrc_p1",W_LBL_LYRICS,"lbl_lrc_n1","lbl_lrc_n2"};
            int offsets[] = {-2,-1,0,1,2};
            if (lrc && lrc->count > 0) {
                int li = music_app_get_lyrics_line(pos);
                int j;
                for (j = 0; j < 5; j++) {
                    widget_t* ll = find(ln[j]);
                    if (!ll) continue;
                    int idx = li + offsets[j];
                    widget_set_text_utf8(ll, (idx >= 0 && idx < lrc->count) ? lrc->lines[idx].text : "");
                }
            } else {
                int j;
                for (j = 0; j < 5; j++) {
                    widget_t* ll = find(ln[j]);
                    if (ll) widget_set_text_utf8(ll, "");
                }
            }
        }
        break;
    }

    case APP_EVENT_ERROR: {
        widget_t* lbl = find(W_LBL_STATUS);
        if (lbl) widget_set_text_utf8(lbl, "Error");
        break;
    }

    default: break;
    }
}