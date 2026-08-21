/**
 * @file music_ui.c
 * @brief AWTK UI layer implementation for the music player.
 *
 * All widgets are created programmatically (no XML UI description needed,
 * though XML can be used if preferred — just load via window_open()).
 *
 * Layout targets 1024×600 (same as cluster large screen).
 *
 * Reference: Android MusicInfoLayout.java / MusicListLayout.java
 *            Android MusicInfoFragment.java (play controls)
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#include "music_ui.h"
#include "favorite_manager.h"

#include <stdio.h>
#include <string.h>

/*============================================================================
 * Widget name constants (for widget_lookup)
 *==========================================================================*/
#define W_TITLE       "lbl_title"
#define W_ARTIST      "lbl_artist"
#define W_ALBUM       "lbl_album"
#define W_TIME_CUR    "lbl_time_cur"
#define W_TIME_TOTAL  "lbl_time_total"
#define W_SLIDER      "slider_progress"
#define W_BTN_PLAY    "btn_play"
#define W_BTN_PREV    "btn_prev"
#define W_BTN_NEXT    "btn_next"
#define W_BTN_MODE    "btn_mode"
#define W_LBL_MODE    "lbl_mode"
#define W_LBL_STATUS  "lbl_status"
#define W_LIST_VIEW   "list_playlist"
#define W_LBL_COUNT   "lbl_count"
#define W_BTN_FAV     "btn_fav"

/* Issue #41: Storage device switch buttons */
#define W_DEVICE_BAR  "bar_device"
#define W_BTN_DEV_FMT "btn_dev_%d"   /* btn_dev_0, btn_dev_1, ... */

/* Issue #42: Playlist type Tab buttons */
#define W_TAB_BAR     "bar_tabs"
#define W_BTN_TAB_ALL "btn_tab_all"
#define W_BTN_TAB_FOLDER "btn_tab_folder"
#define W_BTN_TAB_FAV "btn_tab_fav"
#define W_BTN_TAB_ALBUM "btn_tab_album"
#define W_BTN_TAB_ARTIST "btn_tab_artist"

/* Issue #44: Lyrics display */
#define W_LBL_LYRICS  "lbl_lyrics"

/* Issue #42: Current active tab */
typedef enum {
    TAB_ALL     = 0,
    TAB_FOLDER  = 1,
    TAB_FAV     = 2,
    TAB_ALBUM   = 3,
    TAB_ARTIST  = 4,
} tab_type_t;

/*============================================================================
 * Module state
 *==========================================================================*/
static widget_t* s_win = NULL;
static bool s_slider_dragging = false;
static int s_last_highlight_idx = -1; /* Issue #20: track last highlighted row */
static tab_type_t s_active_tab = TAB_ALL; /* Issue #42: active playlist tab */

/*============================================================================
 * Helper: find child widget by name
 *==========================================================================*/
static widget_t* find(const char* name) {
    return s_win ? widget_lookup(s_win, name, TRUE) : NULL;
}

/*============================================================================
 * Time formatting helper
 *==========================================================================*/
static void format_time(char* buf, int buf_len, int ms) {
    if (ms < 0) ms = 0;
    int sec = ms / 1000;
    int min = sec / 60;
    sec %= 60;
    snprintf(buf, buf_len, "%02d:%02d", min, sec);
}

/*============================================================================
 * Widget event handlers → music_app calls
 *==========================================================================*/
static ret_t on_btn_play_click(void* ctx, event_t* e) {
    (void)ctx;
    (void)e;
    music_app_toggle_play_pause();
    return RET_OK;
}

static ret_t on_btn_prev_click(void* ctx, event_t* e) {
    (void)ctx;
    (void)e;
    music_app_prev();
    return RET_OK;
}

static ret_t on_btn_next_click(void* ctx, event_t* e) {
    (void)ctx;
    (void)e;
    music_app_next();
    return RET_OK;
}

static ret_t on_btn_mode_click(void* ctx, event_t* e) {
    (void)ctx;
    (void)e;
    music_app_cycle_play_mode();

    /* Update mode label immediately */
    const music_app_state_t* st = music_app_get_state();
    const char* mode_str = "Sequential";
    switch (st->play_mode) {
        case PLAY_MODE_SEQUENTIAL: mode_str = "Sequential";  break;
        case PLAY_MODE_REPEAT_ALL: mode_str = "Repeat All";  break;
        case PLAY_MODE_REPEAT_ONE: mode_str = "Repeat One";  break;
        case PLAY_MODE_SHUFFLE:    mode_str = "Shuffle";      break;
    }
    widget_t* lbl = find(W_LBL_MODE);
    if (lbl) widget_set_text_utf8(lbl, mode_str);

    return RET_OK;
}

/* Issue #1: Favorite toggle button handler */
static void update_fav_button_text(void) {
    widget_t* btn = find(W_BTN_FAV);
    if (btn) {
        bool is_fav = music_app_is_favorite();
        widget_set_text_utf8(btn, is_fav ? "Unfav" : "Fav");
    }
}

static ret_t on_btn_fav_click(void* ctx, event_t* e) {
    (void)ctx;
    (void)e;
    music_app_toggle_favorite();
    update_fav_button_text();
    return RET_OK;
}

/*============================================================================
 * Issue #42: Tab click handlers — switch between playlist views
 * Mirrors Android MusicViewPaperFragment Tab switching
 *==========================================================================*/
static void rebuild_folder_list_view(void);
static void rebuild_album_list_view(void);
static void rebuild_artist_list_view(void);
static void rebuild_favorite_list_view(void);
static void update_tab_highlight(tab_type_t active);

static ret_t on_tab_all_click(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    s_active_tab = TAB_ALL;
    music_app_restore_full_playlist();
    update_tab_highlight(TAB_ALL);
    return RET_OK;
}

static ret_t on_tab_folder_click(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    s_active_tab = TAB_FOLDER;
    rebuild_folder_list_view();
    update_tab_highlight(TAB_FOLDER);
    return RET_OK;
}

static ret_t on_tab_fav_click(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    s_active_tab = TAB_FAV;
    rebuild_favorite_list_view();
    update_tab_highlight(TAB_FAV);
    return RET_OK;
}

static ret_t on_tab_album_click(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    s_active_tab = TAB_ALBUM;
    rebuild_album_list_view();
    update_tab_highlight(TAB_ALBUM);
    return RET_OK;
}

static ret_t on_tab_artist_click(void* ctx, event_t* e) {
    (void)ctx; (void)e;
    s_active_tab = TAB_ARTIST;
    rebuild_artist_list_view();
    update_tab_highlight(TAB_ARTIST);
    return RET_OK;
}

/* Issue #42: Update tab button visual state (highlight active tab) */
static void update_tab_highlight(tab_type_t active) {
    const char* tab_names[] = {
        W_BTN_TAB_ALL, W_BTN_TAB_FOLDER, W_BTN_TAB_FAV,
        W_BTN_TAB_ALBUM, W_BTN_TAB_ARTIST
    };
    int i;
    for (i = 0; i < 5; i++) {
        widget_t* btn = find(tab_names[i]);
        if (btn) {
            if (i == (int)active) {
                widget_set_style_str(btn, "text_color", "#00E0FF");
                widget_set_style_str(btn, "border_color", "#00E0FF");
            } else {
                widget_set_style_str(btn, "text_color", "#AAAAAA");
                widget_set_style_str(btn, "border_color", "#444444");
            }
            widget_invalidate_force(btn, NULL);
        }
    }
}

/*============================================================================
 * Issue #41: Device switch handler
 * Mirrors Android MusicUI RadioButton rb_usb_bg / rb_sd_bg switching
 *==========================================================================*/
static ret_t on_device_btn_click(void* ctx, event_t* e) {
    (void)ctx;
    widget_t* btn = WIDGET(e->target);
    if (btn) {
        int dev_idx = widget_get_prop_int(btn, "dev_index", -1);
        if (dev_idx >= 0) {
            music_app_switch_device(dev_idx);
            s_active_tab = TAB_ALL;
            update_tab_highlight(TAB_ALL);
        }
    }
    return RET_OK;
}

/*============================================================================
 * Issue #42: Rebuild list view for folder/album/artist/favorite tabs
 *==========================================================================*/

/* Folder list click: play all songs in that folder */
static ret_t on_folder_item_click(void* ctx, event_t* e) {
    (void)ctx;
    widget_t* item = WIDGET(e->target);
    if (item) {
        const char* path = widget_get_prop_str(item, "folder_path", NULL);
        if (path) {
            music_app_play_folder(path);
        }
    }
    return RET_OK;
}

static void rebuild_folder_list_view(void) {
    widget_t* list = find(W_LIST_VIEW);
    if (!list) return;

    widget_destroy_children(list);

    const char** folders = NULL;
    int count = 0;
    music_app_get_folder_list(&folders, &count);

    int item_h = 35;
    int max_visible = count < 100 ? count : 100;
    int i;
    for (i = 0; i < max_visible; i++) {
        /* Show just the last directory name for brevity */
        const char* full_path = folders[i];
        const char* display = strrchr(full_path, '/');
        display = display ? display + 1 : full_path;

        char text[512];
        snprintf(text, sizeof(text), "   %s/", display);

        widget_t* item = label_create(list, 0, i * item_h, 984, item_h);
        widget_set_text_utf8(item, text);
        widget_set_prop_str(item, "folder_path", full_path);
        widget_set_style_str(item, "font_size", "16");
        widget_set_style_str(item, "text_color", "#FFD700");
        widget_on(item, EVT_CLICK, on_folder_item_click, NULL);
    }

    widget_t* lbl = find(W_LBL_COUNT);
    if (lbl) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%d folders", count);
        widget_set_text_utf8(lbl, buf);
    }

    widget_invalidate_force(list, NULL);
}

/* Album/artist group item click: play group starting at first track */
static ret_t on_group_item_click(void* ctx, event_t* e) {
    const music_group_t* group = (const music_group_t*)ctx;
    widget_t* item = WIDGET(e->target);
    if (item && group) {
        int index = widget_get_prop_int(item, "item_index", 0);
        music_app_play_group(group, index);
    }
    return RET_OK;
}

static void rebuild_group_list_view(const music_group_t* groups, int count,
                                    const char* type_label) {
    widget_t* list = find(W_LIST_VIEW);
    if (!list) return;

    widget_destroy_children(list);

    int item_h = 35;
    int max_visible = count < 100 ? count : 100;
    int i;
    for (i = 0; i < max_visible; i++) {
        char text[512];
        snprintf(text, sizeof(text), "   %s (%d)", groups[i].key, groups[i].count);

        widget_t* item = label_create(list, 0, i * item_h, 984, item_h);
        widget_set_text_utf8(item, text);
        widget_set_prop_int(item, "item_index", 0);
        widget_set_style_str(item, "font_size", "16");
        widget_set_style_str(item, "text_color", "#80eee1");
        /* Pass the group pointer as ctx for the click handler */
        widget_on(item, EVT_CLICK, on_group_item_click, (void*)&groups[i]);
    }

    widget_t* lbl = find(W_LBL_COUNT);
    if (lbl) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%d %s", count, type_label);
        widget_set_text_utf8(lbl, buf);
    }

    widget_invalidate_force(list, NULL);
}

static void rebuild_album_list_view(void) {
    const music_group_t* groups = NULL;
    int count = 0;
    music_app_get_album_list(&groups, &count);
    rebuild_group_list_view(groups, count, "albums");
}

static void rebuild_artist_list_view(void) {
    const music_group_t* groups = NULL;
    int count = 0;
    music_app_get_artist_list(&groups, &count);
    rebuild_group_list_view(groups, count, "artists");
}

static void rebuild_favorite_list_view(void) {
    widget_t* list = find(W_LIST_VIEW);
    if (!list) return;

    widget_destroy_children(list);

    int count = 0;
    const MusicInfo* favs = music_app_get_favorite_list(&count);

    int item_h = 35;
    int max_visible = count < 100 ? count : 100;
    int i;
    for (i = 0; i < max_visible; i++) {
        char text[512];
        const char* title = favs[i].title;
        if (title[0] == '\0') title = favs[i].filename;
        snprintf(text, sizeof(text), "   %d. %s - %s",
                 i + 1, title, favs[i].artist[0] ? favs[i].artist : "--");

        widget_t* item = label_create(list, 0, i * item_h, 984, item_h);
        widget_set_text_utf8(item, text);
        widget_set_style_str(item, "font_size", "16");
        widget_set_style_str(item, "text_color", "#FF6B6B");
        /* TODO: Click to play from favorites playlist */
    }

    widget_t* lbl = find(W_LBL_COUNT);
    if (lbl) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%d favorites", count);
        widget_set_text_utf8(lbl, buf);
    }

    widget_invalidate_force(list, NULL);
}

static ret_t on_slider_value_changed(void* ctx, event_t* e) {
    (void)ctx;
    (void)e;

    /* Issue #5: During drag, only update the time label visually.
     * The actual seek command is sent once in on_slider_pointer_up().
     * Sending seek on every value change causes audio stutter because
     * the player restarts decoding from a new position each time. */
    if (!s_slider_dragging) return RET_OK;

    /* Update the current-time label to follow the user's finger */
    widget_t* slider = find(W_SLIDER);
    if (slider) {
        int value = (int)widget_get_prop_int(slider, WIDGET_PROP_VALUE, 0);
        char buf[16];
        format_time(buf, sizeof(buf), value);
        widget_t* lbl_cur = find(W_TIME_CUR);
        if (lbl_cur) {
            widget_set_text_utf8(lbl_cur, buf);
        }
    }
    return RET_OK;
}

static ret_t on_slider_pointer_down(void* ctx, event_t* e) {
    (void)ctx;
    (void)e;
    s_slider_dragging = true;
    return RET_OK;
}

static ret_t on_slider_pointer_up(void* ctx, event_t* e) {
    (void)ctx;
    (void)e;
    s_slider_dragging = false;
    /* Seek to final position */
    widget_t* slider = find(W_SLIDER);
    if (slider) {
        int value = (int)widget_get_prop_int(slider, WIDGET_PROP_VALUE, 0);
        music_app_seek(value);
    }
    return RET_OK;
}

/*============================================================================
 * Playlist item click handler
 *==========================================================================*/
static ret_t on_list_item_click(void* ctx, event_t* e) {
    (void)ctx;
    widget_t* item = WIDGET(e->target);
    if (item) {
        int index = widget_get_prop_int(item, "item_index", -1);
        if (index >= 0) {
            music_app_play(index);
        }
    }
    return RET_OK;
}

/*============================================================================
 * UI creation — programmatic widget construction
 *==========================================================================*/
ret_t music_ui_create(widget_t* win) {
    s_win = win;

    /* === Top area: song info (title, artist, album) === */
    widget_t* lbl_title = label_create(win, 20, 20, 700, 40);
    widget_set_name(lbl_title, W_TITLE);
    widget_set_text_utf8(lbl_title, "No Track");
    widget_set_style_str(lbl_title, "font_size", "28");
    widget_set_style_str(lbl_title, "text_color", "#FFFFFF");

    widget_t* lbl_artist = label_create(win, 20, 65, 500, 30);
    widget_set_name(lbl_artist, W_ARTIST);
    widget_set_text_utf8(lbl_artist, "---");
    widget_set_style_str(lbl_artist, "font_size", "20");
    widget_set_style_str(lbl_artist, "text_color", "#80eee1");

    widget_t* lbl_album = label_create(win, 530, 65, 300, 30);
    widget_set_name(lbl_album, W_ALBUM);
    widget_set_text_utf8(lbl_album, "---");
    widget_set_style_str(lbl_album, "font_size", "18");
    widget_set_style_str(lbl_album, "text_color", "#888888");

    /* === Progress area: time labels + slider === */
    widget_t* lbl_cur = label_create(win, 20, 110, 60, 25);
    widget_set_name(lbl_cur, W_TIME_CUR);
    widget_set_text_utf8(lbl_cur, "00:00");
    widget_set_style_str(lbl_cur, "font_size", "16");
    widget_set_style_str(lbl_cur, "text_color", "#CCCCCC");

    widget_t* slider = slider_create(win, 85, 112, 600, 20);
    widget_set_name(slider, W_SLIDER);
    slider_set_min(slider, 0);
    slider_set_max(slider, 100);
    slider_set_value(slider, 0);
    widget_on(slider, EVT_VALUE_CHANGED, on_slider_value_changed, NULL);
    widget_on(slider, EVT_POINTER_DOWN, on_slider_pointer_down, NULL);
    widget_on(slider, EVT_POINTER_UP, on_slider_pointer_up, NULL);

    widget_t* lbl_total = label_create(win, 695, 110, 60, 25);
    widget_set_name(lbl_total, W_TIME_TOTAL);
    widget_set_text_utf8(lbl_total, "00:00");
    widget_set_style_str(lbl_total, "font_size", "16");
    widget_set_style_str(lbl_total, "text_color", "#CCCCCC");

    /* === Control buttons === */
    int btn_y = 150;
    int btn_w = 80;
    int btn_h = 50;
    int btn_gap = 20;
    int btn_x = (1024 - 3 * btn_w - 2 * btn_gap) / 2;

    widget_t* btn_prev = button_create(win, btn_x, btn_y, btn_w, btn_h);
    widget_set_name(btn_prev, W_BTN_PREV);
    widget_set_text_utf8(btn_prev, "|<");
    widget_on(btn_prev, EVT_CLICK, on_btn_prev_click, NULL);

    widget_t* btn_play = button_create(win, btn_x + btn_w + btn_gap, btn_y,
                                       btn_w, btn_h);
    widget_set_name(btn_play, W_BTN_PLAY);
    widget_set_text_utf8(btn_play, "Play");
    widget_on(btn_play, EVT_CLICK, on_btn_play_click, NULL);

    widget_t* btn_next = button_create(win, btn_x + 2*(btn_w + btn_gap), btn_y,
                                       btn_w, btn_h);
    widget_set_name(btn_next, W_BTN_NEXT);
    widget_set_text_utf8(btn_next, ">|");
    widget_on(btn_next, EVT_CLICK, on_btn_next_click, NULL);

    /* Mode button (right side) */
    widget_t* btn_mode = button_create(win, 850, btn_y, 80, btn_h);
    widget_set_name(btn_mode, W_BTN_MODE);
    widget_set_text_utf8(btn_mode, "Mode");
    widget_on(btn_mode, EVT_CLICK, on_btn_mode_click, NULL);

    /* Issue #1: Favorite button */
    widget_t* btn_fav = button_create(win, 940, btn_y, 64, btn_h);
    widget_set_name(btn_fav, W_BTN_FAV);
    widget_set_text_utf8(btn_fav, "Fav");
    widget_on(btn_fav, EVT_CLICK, on_btn_fav_click, NULL);

    widget_t* lbl_mode = label_create(win, 850, btn_y + btn_h + 5, 140, 25);
    widget_set_name(lbl_mode, W_LBL_MODE);
    widget_set_text_utf8(lbl_mode, "Repeat All");
    widget_set_style_str(lbl_mode, "font_size", "16");
    widget_set_style_str(lbl_mode, "text_color", "#80eee1");

    /* === Status label (scanning, device info) === */
    widget_t* lbl_status = label_create(win, 20, 215, 984, 25);
    widget_set_name(lbl_status, W_LBL_STATUS);
    widget_set_text_utf8(lbl_status, "Insert USB to start");
    widget_set_style_str(lbl_status, "font_size", "16");
    widget_set_style_str(lbl_status, "text_color", "#AAAAAA");

    /* Track count label */
    widget_t* lbl_count = label_create(win, 850, 215, 160, 25);
    widget_set_name(lbl_count, W_LBL_COUNT);
    widget_set_text_utf8(lbl_count, "0 tracks");
    widget_set_style_str(lbl_count, "font_size", "14");
    widget_set_style_str(lbl_count, "text_color", "#888888");

    /* === Issue #41: Device switch bar (top-right corner) === */
    /* Shows USB0, USB1, SD0 etc. buttons for switching storage devices.
     * Mirrors Android MusicUI rb_usb_bg / rb_sd_bg RadioButtons.
     * Populated dynamically in music_ui_on_app_event(STORAGE_MOUNTED). */
    /* Device bar is a container; buttons are added/removed dynamically */
    widget_t* dev_bar = view_create(win, 760, 20, 244, 30);
    widget_set_name(dev_bar, W_DEVICE_BAR);

    /* === Issue #42: Playlist type Tab bar ===
     * Mirrors Android MusicViewPaperFragment Tab (全部/文件夹/收藏/专辑/艺术家) */
    {
        int tab_y = 220;
        int tab_w = 80;
        int tab_h = 28;
        int tab_gap = 8;
        int tab_x = 20;

        widget_t* t_all = button_create(win, tab_x, tab_y, tab_w, tab_h);
        widget_set_name(t_all, W_BTN_TAB_ALL);
        widget_set_text_utf8(t_all, "All");
        widget_set_style_str(t_all, "font_size", "14");
        widget_on(t_all, EVT_CLICK, on_tab_all_click, NULL);

        tab_x += tab_w + tab_gap;
        widget_t* t_folder = button_create(win, tab_x, tab_y, tab_w, tab_h);
        widget_set_name(t_folder, W_BTN_TAB_FOLDER);
        widget_set_text_utf8(t_folder, "Folder");
        widget_set_style_str(t_folder, "font_size", "14");
        widget_on(t_folder, EVT_CLICK, on_tab_folder_click, NULL);

        tab_x += tab_w + tab_gap;
        widget_t* t_fav = button_create(win, tab_x, tab_y, tab_w, tab_h);
        widget_set_name(t_fav, W_BTN_TAB_FAV);
        widget_set_text_utf8(t_fav, "Fav");
        widget_set_style_str(t_fav, "font_size", "14");
        widget_on(t_fav, EVT_CLICK, on_tab_fav_click, NULL);

        tab_x += tab_w + tab_gap;
        widget_t* t_album = button_create(win, tab_x, tab_y, tab_w, tab_h);
        widget_set_name(t_album, W_BTN_TAB_ALBUM);
        widget_set_text_utf8(t_album, "Album");
        widget_set_style_str(t_album, "font_size", "14");
        widget_on(t_album, EVT_CLICK, on_tab_album_click, NULL);

        tab_x += tab_w + tab_gap;
        widget_t* t_artist = button_create(win, tab_x, tab_y, tab_w, tab_h);
        widget_set_name(t_artist, W_BTN_TAB_ARTIST);
        widget_set_text_utf8(t_artist, "Artist");
        widget_set_style_str(t_artist, "font_size", "14");
        widget_on(t_artist, EVT_CLICK, on_tab_artist_click, NULL);

        /* Set initial highlight on "All" tab */
        update_tab_highlight(TAB_ALL);
    }

    /* === Issue #44: Lyrics display label === */
    widget_t* lbl_lyrics = label_create(win, 760, 110, 244, 100);
    widget_set_name(lbl_lyrics, W_LBL_LYRICS);
    widget_set_text_utf8(lbl_lyrics, "");
    widget_set_style_str(lbl_lyrics, "font_size", "14");
    widget_set_style_str(lbl_lyrics, "text_color", "#80eee1");

    /* === Playlist area (bottom half) === */
    /* Note: For production, this should use a proper list_view with virtual
     * scrolling via AWTK's list_view_create(). Here we use a simple approach
     * suitable for the initial implementation. A full list_view with
     * custom item renderer will be added as a follow-up enhancement. */
    widget_t* list = list_view_create(win, 20, 260, 984, 330);
    widget_set_name(list, W_LIST_VIEW);

    printf("[music_ui] UI created\n");
    return RET_OK;
}

void music_ui_destroy(void) {
    s_win = NULL;
    printf("[music_ui] UI destroyed\n");
}

/*============================================================================
 * Playlist rebuild — populates list_view with current playlist
 *==========================================================================*/
static void rebuild_playlist_view(void) {
    widget_t* list = find(W_LIST_VIEW);
    if (!list) return;

    /* Clear existing children */
    widget_destroy_children(list);

    int count = music_app_get_playlist_count();
    int cur_idx = music_app_get_current_index();

    /* Limit visible items for performance (production: use virtual list) */
    int max_visible = count < 100 ? count : 100;
    int item_h = 35;

    int i;
    for (i = 0; i < max_visible; i++) {
        const MusicInfo* info = music_app_get_track_info(i);
        if (!info) continue;

        char text[512];
        snprintf(text, sizeof(text), "%s%d. %s - %s",
                 (i == cur_idx) ? ">> " : "   ",
                 i + 1, info->title, info->artist);

        widget_t* item = label_create(list, 0, i * item_h, 984, item_h);
        widget_set_text_utf8(item, text);
        widget_set_prop_int(item, "item_index", i);
        widget_set_style_str(item, "font_size", "16");

        if (i == cur_idx) {
            widget_set_style_str(item, "text_color", "#00E0FF");
        } else {
            widget_set_style_str(item, "text_color", "#CCCCCC");
        }

        widget_on(item, EVT_CLICK, on_list_item_click, NULL);
    }

    /* Update count label */
    widget_t* lbl = find(W_LBL_COUNT);
    if (lbl) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%d tracks", count);
        widget_set_text_utf8(lbl, buf);
    }

    /* Issue #20: Remember the current highlight index for incremental updates */
    s_last_highlight_idx = cur_idx;

    widget_invalidate_force(list, NULL);
}

/*============================================================================
 * Issue #20 fix: Lightweight playlist highlight update.
 * Only changes the text color of the old and new highlighted rows,
 * avoiding the full destroy+rebuild cycle that causes flicker.
 *==========================================================================*/
static void update_playlist_highlight(int new_idx) {
    widget_t* list = find(W_LIST_VIEW);
    if (!list) return;

    int child_count = widget_count_children(list);

    /* Un-highlight old row */
    if (s_last_highlight_idx >= 0 && s_last_highlight_idx < child_count) {
        widget_t* old_item = widget_get_child(list, s_last_highlight_idx);
        if (old_item) {
            widget_set_style_str(old_item, "text_color", "#CCCCCC");

            /* Update text to remove ">>" prefix */
            const MusicInfo* info = music_app_get_track_info(s_last_highlight_idx);
            if (info) {
                char text[512];
                snprintf(text, sizeof(text), "   %d. %s - %s",
                         s_last_highlight_idx + 1, info->title, info->artist);
                widget_set_text_utf8(old_item, text);
            }
            widget_invalidate_force(old_item, NULL);
        }
    }

    /* Highlight new row */
    if (new_idx >= 0 && new_idx < child_count) {
        widget_t* new_item = widget_get_child(list, new_idx);
        if (new_item) {
            widget_set_style_str(new_item, "text_color", "#00E0FF");

            const MusicInfo* info = music_app_get_track_info(new_idx);
            if (info) {
                char text[512];
                snprintf(text, sizeof(text), ">> %d. %s - %s",
                         new_idx + 1, info->title, info->artist);
                widget_set_text_utf8(new_item, text);
            }
            widget_invalidate_force(new_item, NULL);
        }
    }

    s_last_highlight_idx = new_idx;
}

/*============================================================================
 * App event handler — called on AWTK main thread
 *==========================================================================*/
void music_ui_on_app_event(music_app_event_t event, void* param) {
    int int_param = param ? *(int*)param : 0;

    switch (event) {
        case APP_EVENT_STORAGE_MOUNTED: {
            widget_t* lbl = find(W_LBL_STATUS);
            const storage_device_state_t* dev = music_app_get_device(int_param);
            if (lbl && dev) {
                char buf[256];
                snprintf(buf, sizeof(buf), "Device inserted: %s",
                         dev->mount_point);
                widget_set_text_utf8(lbl, buf);
            }

            /* Issue #41: Rebuild device switch buttons.
             * Mirrors Android MusicUI RadioButton creation for USB/SD devices. */
            {
                widget_t* dev_bar = find(W_DEVICE_BAR);
                if (dev_bar) {
                    widget_destroy_children(dev_bar);
                    int dev_count = music_app_get_device_count();
                    int btn_w = 60;
                    int btn_gap = 4;
                    int di;
                    for (di = 0; di < dev_count && di < MAX_STORAGE_DEVICES; di++) {
                        const storage_device_state_t* d = music_app_get_device(di);
                        if (!d) continue;

                        /* Extract short label from mount point (e.g. "usb0") */
                        const char* mp = d->mount_point;
                        const char* slash = strrchr(mp, '/');
                        const char* label = slash ? slash + 1 : mp;

                        widget_t* dbtn = button_create(dev_bar,
                            di * (btn_w + btn_gap), 0, btn_w, 28);
                        widget_set_text_utf8(dbtn, label);
                        widget_set_prop_int(dbtn, "dev_index", di);
                        widget_set_style_str(dbtn, "font_size", "12");
                        widget_on(dbtn, EVT_CLICK, on_device_btn_click, NULL);

                        const music_app_state_t* st = music_app_get_state();
                        if (di == st->current_device_idx) {
                            widget_set_style_str(dbtn, "text_color", "#00E0FF");
                        } else {
                            widget_set_style_str(dbtn, "text_color", "#AAAAAA");
                        }
                    }
                    widget_invalidate_force(dev_bar, NULL);
                }
            }
            break;
        }

        case APP_EVENT_STORAGE_UNMOUNTED: {
            widget_t* lbl = find(W_LBL_STATUS);
            if (lbl) {
                widget_set_text_utf8(lbl, "Device removed");
            }
            rebuild_playlist_view();
            break;
        }

        case APP_EVENT_SCAN_STARTED: {
            widget_t* lbl = find(W_LBL_STATUS);
            if (lbl) {
                widget_set_text_utf8(lbl, "Scanning...");
            }
            break;
        }

        case APP_EVENT_SCAN_FINISHED: {
            widget_t* lbl = find(W_LBL_STATUS);
            if (lbl) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Scan complete: %d tracks",
                         music_app_get_playlist_count());
                widget_set_text_utf8(lbl, buf);
            }
            break;
        }

        case APP_EVENT_PLAYLIST_CHANGED: {
            /* Issue #42 fix: When the playlist changes (e.g. user played a folder
             * or album group), we should NOT blindly call rebuild_playlist_view()
             * which would overwrite the current tab view. Instead:
             * - If user is browsing a non-ALL tab (folder/album/artist list),
             *   only update the count label but keep the tab view.
             * - If user clicked a folder/album item which triggered playback,
             *   the tab view stays; the playlist_view will rebuild when they
             *   return to the ALL tab.
             * - If the event came from a device scan (initial load), switch to ALL. */
            {
                const music_app_state_t* st = music_app_get_state();

                /* If the playlist type matches ALL, rebuild the song list view */
                if (st->playlist_type == PLAYLIST_TYPE_DEVICE ||
                    s_active_tab == TAB_ALL) {
                    rebuild_playlist_view();
                }
                /* Otherwise the user is in a sub-tab; keep their view intact */
            }

            /* Issue #10: Update index/total count after playlist loads */
            {
                int total = music_app_get_playlist_count();
                int cur_idx = music_app_get_current_index();
                widget_t* lbl = find(W_LBL_COUNT);
                if (lbl) {
                    char buf[64];
                    if (cur_idx >= 0 && total > 0) {
                        snprintf(buf, sizeof(buf), "%d/%d", cur_idx + 1, total);
                    } else {
                        snprintf(buf, sizeof(buf), "%d tracks", total);
                    }
                    widget_set_text_utf8(lbl, buf);
                }
            }
            break;
        }

        case APP_EVENT_TRACK_CHANGED: {
            const music_app_state_t* st = music_app_get_state();
            if (st->current_info) {
                widget_t* t = find(W_TITLE);
                widget_t* a = find(W_ARTIST);
                widget_t* al = find(W_ALBUM);

                /* Issue #13: If title is empty, show filename without extension.
                 * If artist/album is empty, show "--" instead of blank.
                 * Mirrors Android MusicInfoLayout.updateId3TextInfo() logic. */
                if (t) {
                    const char* title = st->current_info->title;
                    if (title[0] == '\0' || strcmp(title, "<Unknown>") == 0) {
                        /* Fallback: show filename without extension */
                        char name_buf[MUSIC_MAX_TAG_LEN];
                        snprintf(name_buf, sizeof(name_buf), "%s", st->current_info->filename);
                        char* dot = strrchr(name_buf, '.');
                        if (dot) *dot = '\0';
                        widget_set_text_utf8(t, name_buf[0] ? name_buf : "Unknown");
                    } else {
                        widget_set_text_utf8(t, title);
                    }
                }
                if (a) {
                    const char* artist = st->current_info->artist;
                    widget_set_text_utf8(a,
                        (artist[0] == '\0' || strcmp(artist, "<Unknown>") == 0) ? "--" : artist);
                }
                if (al) {
                    const char* album = st->current_info->album;
                    widget_set_text_utf8(al,
                        (album[0] == '\0' || strcmp(album, "<Unknown>") == 0) ? "--" : album);
                }
            }

            /* Issue #10: Update "current/total" display.
             * Mirrors Android MusicInfoLayout.changeTotalValue(). */
            {
                int cur_idx = music_app_get_current_index();
                int total = music_app_get_playlist_count();
                widget_t* lbl = find(W_LBL_COUNT);
                if (lbl && total > 0) {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%d/%d", cur_idx + 1, total);
                    widget_set_text_utf8(lbl, buf);
                }
            }

            /* Issue #1: Update favorite button state on track change */
            update_fav_button_text();

            /* Issue #20 fix: Use lightweight highlight update instead of
             * full rebuild. Only changes the text/color of old and new
             * highlighted rows, avoiding destroy+recreate of all widgets. */
            {
                int cur_idx = music_app_get_current_index();
                update_playlist_highlight(cur_idx);
            }
            break;
        }

        case APP_EVENT_FAVORITE_CHANGED: {
            /* Issue #1: Favorite state changed — update button */
            update_fav_button_text();
            break;
        }

        case APP_EVENT_STATE_CHANGED: {
            widget_t* btn = find(W_BTN_PLAY);
            if (btn) {
                PlayerState ps = (PlayerState)int_param;
                if (ps == PLAYER_STATE_PLAYING) {
                    widget_set_text_utf8(btn, "Pause");
                } else {
                    widget_set_text_utf8(btn, "Play");
                }
            }
            break;
        }

        case APP_EVENT_POSITION_CHANGED: {
            if (s_slider_dragging) break;

            const music_app_state_t* st = music_app_get_state();
            int pos = st->current_position_ms;
            int dur = st->current_duration_ms;

            widget_t* slider = find(W_SLIDER);
            if (slider && dur > 0) {
                slider_set_max(slider, dur);
                slider_set_value(slider, pos);
            }

            char buf[16];
            widget_t* lbl_cur = find(W_TIME_CUR);
            if (lbl_cur) {
                format_time(buf, sizeof(buf), pos);
                widget_set_text_utf8(lbl_cur, buf);
            }

            widget_t* lbl_total = find(W_TIME_TOTAL);
            if (lbl_total && dur > 0) {
                format_time(buf, sizeof(buf), dur);
                widget_set_text_utf8(lbl_total, buf);
            }

            /* Issue #44: Update lyrics display if lyrics are available.
             * Mirrors Android MusicInfoLayout.updateLrcRowList(). */
            {
                const lrc_data_t* lrc = music_app_get_lyrics();
                widget_t* lbl_lrc = find(W_LBL_LYRICS);
                if (lbl_lrc) {
                    if (lrc && lrc->count > 0) {
                        int line_idx = music_app_get_lyrics_line(pos);
                        if (line_idx >= 0 && line_idx < lrc->count) {
                            widget_set_text_utf8(lbl_lrc, lrc->lines[line_idx].text);
                        } else {
                            widget_set_text_utf8(lbl_lrc, "");
                        }
                    } else {
                        widget_set_text_utf8(lbl_lrc, "");
                    }
                }
            }
            break;
        }

        case APP_EVENT_ERROR: {
            widget_t* lbl = find(W_LBL_STATUS);
            if (lbl) {
                widget_set_text_utf8(lbl, "Playback error");
            }
            break;
        }

        default:
            break;
    }
}
