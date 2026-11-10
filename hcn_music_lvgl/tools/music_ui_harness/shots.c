/* Headless driver: real ui_Music screen + fake music_app.  Renders PNG frames and asserts behaviour. */
#include "lvgl/lvgl.h"
#include "ui_Music.h"
#include "ui_Music_cf.h"
#include "music_art.h"
#include <math.h>
#include "compat/awtk.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define W 1024
#define H 600
void fake_init(void); void fake_set_device(bool, bool, bool); void fake_set_current(int, int);
void fake_set_state(PlayerState); void fake_set_mode(PlayMode); void fake_set_art(bool); void fake_set_lyrics(bool); void fake_clear_favs(void);
void fake_set_playlist_type(playlist_type_t); void fake_shutdown(void);
void fake_set_second_device(bool); void fake_long_lyric(bool); void fake_reset_counters(void);
int fake_seek_fwd(void); int fake_seek_back(void); int fake_next_calls(void); int fake_prev_calls(void); int fake_last_play(void);
static uint32_t fb[W * H]; static uint8_t buf[W * H * 4];
static int g_checks;
#define CHECK(c) do { g_checks++; if (!(c)) { printf("CHECK FAILED %s:%d  %s\n", __FILE__, __LINE__, #c); exit(1); } } while (0)

static void flush_cb(lv_display_t *d, const lv_area_t *a, uint8_t *px) {
    int32_t w = lv_area_get_width(a);
    for (int y = a->y1; y <= a->y2; y++) memcpy(&fb[y * W + a->x1], px + (y - a->y1) * w * 4, w * 4);
    lv_display_flush_ready(d);
}
static void run(int ms) { for (int t = 0; t < ms; t += 20) { lv_tick_inc(20); lv_timer_handler(); music_mainloop_poll(); } }
static void save(const char *name) {
    char p[128]; snprintf(p, sizeof p, "shots/%s.ppm", name); FILE *f = fopen(p, "wb"); fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (int i = 0; i < W * H; i++) { uint32_t v = fb[i]; unsigned char c[3] = {(v >> 16) & 255, (v >> 8) & 255, v & 255}; fwrite(c, 1, 3, f); } fclose(f);
    printf("shot %s\n", name);
}
static void ev(music_app_event_t e, int p) { ui_Music_on_app_event(e, &p); }

/* ---- find widgets by visible text, click like a finger would ---- */
static lv_obj_t *find_label(lv_obj_t *root, const char *text, bool prefix) {
    if (lv_obj_check_type(root, &lv_label_class) && !lv_obj_has_flag(root, LV_OBJ_FLAG_HIDDEN)) {
        const char *t = lv_label_get_text(root);
        if (t && (prefix ? strncmp(t, text, strlen(text)) == 0 : strcmp(t, text) == 0)) return root;
    }
    for (uint32_t i = 0; i < lv_obj_get_child_count(root); i++) {
        lv_obj_t *c = lv_obj_get_child(root, i);
        if (lv_obj_has_flag(c, LV_OBJ_FLAG_HIDDEN)) continue;
        lv_obj_t *r = find_label(c, text, prefix); if (r) return r;
    }
    return NULL;
}
static void tap(lv_obj_t *scr, const char *text) {
    lv_obj_t *l = find_label(scr, text, false); if (!l) l = find_label(scr, text, true);
    if (!l) { printf("tap: '%s' not found\n", text); exit(1); }
    lv_obj_send_event(lv_obj_get_parent(l), LV_EVENT_CLICKED, NULL);   /* label lives inside the clickable row/tab/button */
    run(60);
}
static bool visible_text(lv_obj_t *scr, const char *text) { return find_label(scr, text, true) != NULL; }
static lv_obj_t *vlist_of(lv_obj_t *scr) {            /* the scrollable 520px-wide pane that is not hidden */
    lv_obj_t *right = lv_obj_get_child(scr, lv_obj_get_child_count(scr) - 1);
    for (uint32_t i = 0; i < lv_obj_get_child_count(right); i++) {
        lv_obj_t *c = lv_obj_get_child(right, i);
        if (lv_obj_get_width(c) == 520 && lv_obj_has_flag(c, LV_OBJ_FLAG_SCROLLABLE)) return c;
    }
    return NULL;
}

/* ---- scripted pointer: real PRESSED / PRESSING / LONG_PRESSED / RELEASED / SHORT_CLICKED sequences ---- */
static struct { int x, y; bool down; } g_ptr;
static void ptr_read(lv_indev_t *i, lv_indev_data_t *d) { LV_UNUSED(i); d->point.x = g_ptr.x; d->point.y = g_ptr.y; d->state = g_ptr.down ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED; }
static void ptr_down(int x, int y) { g_ptr.x = x; g_ptr.y = y; g_ptr.down = true; run(60); }
static void ptr_move(int x, int y) { g_ptr.x = x; g_ptr.y = y; run(40); }
static void ptr_up(void) { g_ptr.down = false; run(80); }
static void tap_xy(int x, int y) { ptr_down(x, y); ptr_up(); }
static void drag_y(int x, int y0, int y1, int steps) { ptr_down(x, y0); for (int i = 1; i <= steps; i++) ptr_move(x, y0 + (y1 - y0) * i / steps); ptr_up(); }
static void center_of(lv_obj_t *o, int *x, int *y) { lv_area_t a; lv_obj_get_coords(o, &a); *x = (a.x1 + a.x2) / 2; *y = (a.y1 + a.y2) / 2; }
static void tap_text_real(lv_obj_t *scr, const char *text) {   /* the label sits inside the clickable tab / chip / pill / row */
    lv_obj_t *l = find_label(scr, text, false); int x, y;
    if (!l) l = find_label(scr, text, true);
    if (!l) { printf("tap_text_real: '%s' not found\n", text); exit(1); }
    center_of(lv_obj_get_parent(l), &x, &y); tap_xy(x, y);
}
static lv_obj_t *find_class(lv_obj_t *root, const lv_obj_class_t *cls) {
    if (lv_obj_check_type(root, cls)) return root;
    for (uint32_t i = 0; i < lv_obj_get_child_count(root); i++) { lv_obj_t *r = find_class(lv_obj_get_child(root, i), cls); if (r) return r; }
    return NULL;
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    if (system("mkdir -p shots")) return 1;
    lv_init();
    lv_display_t *d = lv_display_create(W, H);
    lv_display_set_buffers(d, buf, NULL, sizeof buf, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(d, flush_cb);
    lv_display_set_color_format(d, LV_COLOR_FORMAT_ARGB8888);
    lv_theme_t *th = lv_theme_default_init(d, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_display_set_theme(d, th);   /* same theme setup as ui_variant.c: the screen must not depend on theme defaults */
    fake_init();
    { lv_indev_t *ptr = lv_indev_create(); lv_indev_set_type(ptr, LV_INDEV_TYPE_POINTER); lv_indev_set_read_cb(ptr, ptr_read); lv_indev_set_display(ptr, d); }
    music_app_init(ui_Music_on_app_event);        /* fake engine events now arrive asynchronously, like music_adapter does */
    lv_obj_t *scr = NULL; ui_Music_screen_init(&scr); lv_screen_load(scr); run(400);

    /* 1. playing, all tracks */
    CHECK(visible_text(scr, "夜曲")); CHECK(visible_text(scr, "U盘 · 48 首")); CHECK(visible_text(scr, "01:23"));
    CHECK(visible_text(scr, "歌词"));
    { lv_mem_monitor_t mm; lv_mem_monitor(&mm); if (mm.total_size) printf("LVGL heap in use with the music screen up: %zu KB of %zu KB\n", (mm.total_size - mm.free_size) / 1024, mm.total_size / 1024); }
    save("01_playing_all");

    /* 2. list scrolled: rows are recycled, labels must follow */
    lv_obj_t *vl = vlist_of(scr); CHECK(vl);
    lv_obj_scroll_to_y(vl, 17 * 72 + 30, LV_ANIM_OFF); run(100);
    CHECK(visible_text(scr, "后来 (Remix)")); CHECK(!visible_text(scr, "晴天"));
    save("02_list_scrolled");
    lv_obj_scroll_to_y(vl, 0, LV_ANIM_OFF); run(60); CHECK(visible_text(scr, "晴天"));

    /* 3. paused + repeat-one */
    fake_set_state(PLAYER_STATE_PAUSED); fake_set_mode(PLAY_MODE_REPEAT_ONE); ev(APP_EVENT_STATE_CHANGED, PLAYER_STATE_PAUSED); ui_Music_refresh(); run(100);
    save("03_paused_repeat_one");
    fake_set_state(PLAYER_STATE_PLAYING); fake_set_mode(PLAY_MODE_SHUFFLE);

    /* 4. very long title, no cover: placeholder + marquee; tag-less file falls back to the filename */
    fake_set_art(false); fake_set_current(5, 41000); ev(APP_EVENT_TRACK_CHANGED, 5); run(900); save("04_long_title_no_cover");
    lv_obj_scroll_to_y(vl, 6 * 72, LV_ANIM_OFF); run(100); CHECK(visible_text(scr, "m07.mp3")); CHECK(visible_text(scr, "未知艺术家"));
    lv_obj_scroll_to_y(vl, 0, LV_ANIM_OFF);
    fake_set_art(true); fake_set_current(3, 83000); ev(APP_EVENT_TRACK_CHANGED, 3); run(100);

    /* 5. favourites tab */
    tap(scr, "收藏"); CHECK(visible_text(scr, "光年之外")); CHECK(!visible_text(scr, "七里香")); save("05_tab_favorites");

    /* 6. lyrics via the pill on the cover; current line follows the position */
    tap(scr, "歌词"); fake_set_current(3, 96000); ev(APP_EVENT_POSITION_CHANGED, 96000); run(900);
    CHECK(visible_text(scr, "爱到最后只剩下沉默")); save("06_lyrics");
    fake_set_lyrics(false); ev(APP_EVENT_TRACK_CHANGED, 3); run(100); CHECK(visible_text(scr, "暂无歌词")); save("07_lyrics_none");
    fake_set_lyrics(true); ev(APP_EVENT_TRACK_CHANGED, 3);
    tap(scr, "歌词"); CHECK(!visible_text(scr, "暂无歌词"));      /* pill toggles back to the list */

    /* 7. folders -> tap a folder: playlist becomes the folder and the All tab shows a "back to all" banner */
    tap(scr, "文件夹"); CHECK(visible_text(scr, "周杰伦")); CHECK(visible_text(scr, "Western Hits")); save("08_folders");
    tap(scr, "Western Hits"); run(60);
    CHECK(music_app_get_playlist_count() == 8); CHECK(visible_text(scr, "正在播放筛选列表")); CHECK(visible_text(scr, "U盘 · 8 首 · 文件夹"));
    save("09_folder_playing_banner");
    tap(scr, "正在播放筛选列表"); CHECK(music_app_get_playlist_count() == 48); CHECK(!visible_text(scr, "正在播放筛选列表"));

    /* 8. albums -> drill into one -> back */
    tap(scr, "专辑"); CHECK(visible_text(scr, "叶惠美")); save("10_albums");
    tap(scr, "叶惠美"); CHECK(visible_text(scr, "返回专辑列表")); CHECK(visible_text(scr, "晴天")); save("11_album_tracks");
    tap(scr, "晴天"); CHECK(music_app_get_state()->playlist_type == PLAYLIST_TYPE_ALBUM);
    tap(scr, "返回专辑列表"); CHECK(visible_text(scr, "七里香"));
    fake_set_playlist_type(PLAYLIST_TYPE_DEVICE);

    /* 9. artists */
    tap(scr, "歌手"); CHECK(visible_text(scr, "周杰伦")); save("12_artists");
    tap(scr, "全部");


    /* ================= new features ================= */
    fake_set_current(3, 83000); ev(APP_EVENT_TRACK_CHANGED, 3); run(100);

    /* A. blurred backdrop follows the cover; no art = plain background */
    { lv_obj_t *bd = lv_obj_get_child(scr, 0);                        /* first child of the screen = backdrop image */
      CHECK(!lv_obj_has_flag(bd, LV_OBJ_FLAG_HIDDEN));
      fake_set_art(false); ev(APP_EVENT_TRACK_CHANGED, 3); run(100); CHECK(lv_obj_has_flag(bd, LV_OBJ_FLAG_HIDDEN));
      fake_set_art(true); ev(APP_EVENT_TRACK_CHANGED, 3); run(100); CHECK(!lv_obj_has_flag(bd, LV_OBJ_FLAG_HIDDEN)); }

    /* B. two storage devices: chips appear, tapping one switches the device */
    CHECK(!visible_text(scr, "SD卡 · 12 首"));
    fake_set_second_device(true); ev(APP_EVENT_STORAGE_MOUNTED, 1); run(100);
    CHECK(visible_text(scr, "U盘 · 48 首")); CHECK(visible_text(scr, "SD卡 · 12 首")); save("16_two_devices");
    tap_text_real(scr, "SD卡 · 12 首"); run(200);
    CHECK(music_app_get_state()->current_device_idx == 1); CHECK(visible_text(scr, "SD Track 01")); CHECK(!visible_text(scr, "晴天")); save("17_sd_card");
    tap_text_real(scr, "U盘 · 48 首"); run(200);
    CHECK(music_app_get_state()->current_device_idx == 0); CHECK(visible_text(scr, "晴天"));
    fake_set_second_device(false); ev(APP_EVENT_STORAGE_UNMOUNTED, 1); run(100); CHECK(!visible_text(scr, "SD卡 · 12 首"));   /* chips go away with one device */
    fake_set_current(3, 83000); ev(APP_EVENT_TRACK_CHANGED, 3); run(100);

    /* C. |< / >| : tap = skip, hold = rewind / fast-forward 5 s steps (and a hold must NOT also skip) */
    { int x, y; fake_reset_counters();
      center_of(lv_obj_get_parent(find_label(scr, LV_SYMBOL_PREV, false)), &x, &y);
      tap_xy(x, y); CHECK(fake_prev_calls() == 1 && fake_seek_back() == 0);
      ptr_down(x, y); run(1300); ptr_up();
      CHECK(fake_seek_back() >= 2 && fake_seek_back() <= 4); CHECK(fake_prev_calls() == 1);
      center_of(lv_obj_get_parent(find_label(scr, LV_SYMBOL_NEXT, false)), &x, &y);
      tap_xy(x, y); CHECK(fake_next_calls() == 1 && fake_seek_fwd() == 0);
      ptr_down(x, y); run(1300); ptr_up();
      CHECK(fake_seek_fwd() >= 2 && fake_seek_fwd() <= 4); CHECK(fake_next_calls() == 1); }

    /* D. lyrics: a line wider than the pane scrolls sideways instead of being cut off; the others stay single-line */
    fake_long_lyric(true); fake_set_current(3, 96000); ev(APP_EVENT_TRACK_CHANGED, 3); tap_text_real(scr, "歌词"); ev(APP_EVENT_POSITION_CHANGED, 96000); run(900);
    { lv_obj_t *cur = find_label(scr, "这是一句特别特别长", true), *other = find_label(scr, "我们曾经快乐过", false);
      CHECK(cur && other);
      CHECK(lv_label_get_long_mode(cur) == LV_LABEL_LONG_SCROLL_CIRCULAR); CHECK(lv_label_get_long_mode(other) == LV_LABEL_LONG_DOT); }
    run(700); save("18_lyrics_marquee");
    fake_long_lyric(false); ev(APP_EVENT_TRACK_CHANGED, 3); tap_text_real(scr, "歌词"); run(100); CHECK(!visible_text(scr, "爱到最后只剩下沉默"));

    /* E. search: pinyin initials on a Latin keyboard */
    tap_text_real(scr, "搜索"); run(100);
    { lv_obj_t *ta = find_class(scr, &lv_textarea_class), *kb = find_class(scr, &lv_keyboard_class);
      CHECK(ta && kb && !lv_obj_has_flag(kb, LV_OBJ_FLAG_HIDDEN) && lv_keyboard_get_textarea(kb) == ta);
      CHECK(visible_text(scr, "输入歌名、歌手或拼音首字母")); save("19_search_empty");
      { /* a real finger on the real keyboard: first key of the first row is "q" */
        lv_area_t a; lv_obj_get_coords(kb, &a);
        CHECK(a.y1 >= 88 + 68 && a.y2 <= 88 + 488 && a.x1 >= 464);                        /* the keyboard is inside the right panel */
        /* LVGL text-lower map, row 1: "1#"(5 units) q w e r t y u i o p (4 each) backspace(7): 52 units, 11 gaps of 6 px */
        float unit = (float)((a.x2 - a.x1 + 1) - 11 * 6) / 52.0f; int ky = a.y1 + 23;
        int qx = (int)(a.x1 + 5 * unit + 6 + 2 * unit), wx = (int)(a.x1 + 9 * unit + 12 + 2 * unit);
        tap_xy(qx, ky); CHECK(strcmp(lv_textarea_get_text(ta), "q") == 0);
        tap_xy(wx, ky); CHECK(strcmp(lv_textarea_get_text(ta), "qw") == 0);
        lv_textarea_set_text(ta, ""); run(40); }
      lv_textarea_add_text(ta, "zjl"); run(100);                                  /* 周杰伦 */
      CHECK(visible_text(scr, "12 首")); CHECK(visible_text(scr, "晴天")); CHECK(!visible_text(scr, "Hotel California")); save("20_search_zjl");
      lv_obj_send_event(kb, LV_EVENT_READY, NULL); run(100);                        /* OK key: keyboard away, results get the room */
      CHECK(lv_obj_has_flag(kb, LV_OBJ_FLAG_HIDDEN));
      fake_reset_counters(); tap_text_real(scr, "稻香"); CHECK(fake_last_play() == 2);          /* device-list index, not result index */
      tap_text_real(scr, "清除"); CHECK(strlen(lv_textarea_get_text(ta)) == 0); CHECK(visible_text(scr, "输入歌名、歌手或拼音首字母"));
      lv_textarea_add_text(ta, "qqqqq"); run(100); CHECK(visible_text(scr, "没有找到匹配的歌曲"));
      lv_textarea_set_text(ta, ""); lv_textarea_add_text(ta, "Blind"); run(100); CHECK(visible_text(scr, "2 首"));      /* substring, case-insensitive (track 5 has a long fake title) */
      lv_textarea_set_text(ta, ""); lv_textarea_add_text(ta, "zjl"); run(100); save("21_search_results");
      tap_text_real(scr, "全部"); run(100);
      CHECK(lv_obj_has_flag(ta, LV_OBJ_FLAG_HIDDEN) && lv_obj_has_flag(kb, LV_OBJ_FLAG_HIDDEN)); CHECK(visible_text(scr, "晴天")); }

    /* F. cover flow: art arrives asynchronously, drag to browse, tap centre to play, tap a side card to bring it in */
    fake_set_current(3, 83000); ev(APP_EVENT_TRACK_CHANGED, 3); run(100);                                  /* (scenario E started another track) */
    tap_text_real(scr, "封面流"); run(900);
    CHECK(music_cf_is_visible()); CHECK(visible_text(scr, "4 / 48")); CHECK(visible_text(scr, "正在播放"));
    { bool known = false;
      CHECK(music_art_get("/mnt/usb0/m03.mp3", 160, 16, &known) != NULL && known);             /* centre card art loaded off-thread */
      CHECK(music_art_get("/mnt/usb0/m04.mp3", 160, 16, &known) != NULL && known); }          /* and its neighbours */
    save("22_coverflow");
    drag_y(815, 380, 380 - 172, 8); run(500);                                                    /* 2 cards up = browse forward */
    CHECK(fabsf(music_cf_get_position() - 5.0f) < 0.01f); CHECK(visible_text(scr, "6 / 48")); CHECK(!visible_text(scr, "正在播放"));
    save("23_coverflow_browsed");
    fake_reset_counters(); tap_xy(815, 356); run(300);                                          /* centre card: play it */
    CHECK(fake_last_play() == 5);
    tap_xy(799, 480); run(500); CHECK(fabsf(music_cf_get_position() - 6.0f) < 0.01f);            /* side card (+1), the part the centre card does not cover */
    CHECK(fake_last_play() == 5);                                                                /* ...without playing it */
    for (int i = 0; i < 40; i++) drag_y(815, 175, 175 + 300, 6);                                 /* fling to the start, clamped */
    run(400); CHECK(fabsf(music_cf_get_position() - 0.0f) < 0.01f);
    for (int i = 0; i < 40; i++) drag_y(815, 540, 540 - 300, 6);                                 /* and to the end: every cover loads, caches, evicts */
    run(600); CHECK(fabsf(music_cf_get_position() - 47.0f) < 0.01f); CHECK(visible_text(scr, "48 / 48"));
    fake_set_current(10, 0); ev(APP_EVENT_TRACK_CHANGED, 10); run(500);                          /* the player moved on: the flow follows */
    CHECK(fabsf(music_cf_get_position() - 10.0f) < 0.01f);
    tap_text_real(scr, "封面流"); run(100); CHECK(!music_cf_is_visible());                      /* pill toggles it off */
    tap_text_real(scr, "封面流"); run(300); CHECK(music_cf_is_visible());
    tap_text_real(scr, "收藏"); run(100); CHECK(!music_cf_is_visible());                          /* a tab returns to the list */
    tap_text_real(scr, "全部"); run(100);
    fake_set_current(3, 83000); ev(APP_EVENT_TRACK_CHANGED, 3); run(100);

    /* 10. empty / status states */
    tap(scr, "收藏"); fake_clear_favs(); ev(APP_EVENT_FAVORITE_CHANGED, 0); run(100); CHECK(visible_text(scr, "还没有收藏的歌曲")); save("13_no_favorites");
    tap(scr, "全部");
    fake_set_device(true, true, false); fake_set_current(-1, 0); ev(APP_EVENT_SCAN_STARTED, 0); ui_Music_refresh(); run(100);
    CHECK(visible_text(scr, "正在扫描音乐")); CHECK(!visible_text(scr, "歌词")); save("14_scanning");
    fake_set_device(false, false, false); ev(APP_EVENT_STORAGE_UNMOUNTED, 0); ui_Music_refresh(); run(100);
    CHECK(visible_text(scr, "未检测到存储设备")); save("15_no_device");

    /* 11. event storm while hidden / visible, then destroy twice (idempotent) */
    fake_init(); ui_Music_refresh(); run(60);
    for (int i = 0; i < 2000; i++) { ev((music_app_event_t)(1 + i % 11), i); if (i % 50 == 0) run(20); }
    lv_obj_t *other = lv_obj_create(NULL); lv_screen_load(other); run(40);
    for (int i = 0; i < 500; i++) ev((music_app_event_t)(1 + i % 11), i);                  /* hidden: must be cheap and safe */
    lv_screen_load(scr); ui_Music_refresh(); run(100);
    ui_Music_screen_destroy(); ui_Music_screen_destroy();
    lv_obj_delete(other);
    /* 12. LVGL heap must return to baseline after repeated build/use/destroy (catches leaked objects, bufs, anims) */
    {
        lv_obj_t *blank = lv_obj_create(NULL); lv_screen_load(blank); run(40);
        lv_mem_monitor_t m0, m1;
        fake_init(); ui_Music_screen_init(&scr); lv_screen_load(scr); run(100); lv_screen_load(blank); run(20); ui_Music_screen_destroy(); run(40);
        lv_mem_monitor(&m0);
        for (int k = 0; k < 25; k++) {
            fake_init(); ui_Music_screen_init(&scr); lv_screen_load(scr); run(100);
            tap(scr, "歌词"); run(60); tap(scr, "专辑"); tap(scr, "叶惠美"); tap(scr, "晴天"); tap(scr, "文件夹"); tap(scr, "收藏");
            tap_text_real(scr, "封面流"); run(400); drag_y(815, 540, 300, 5); drag_y(815, 200, 480, 5); tap_xy(815, 356); run(100);     /* cover flow: worker + cache */
            tap_text_real(scr, "搜索"); lv_textarea_add_text(find_class(scr, &lv_textarea_class), "zjl"); run(60); tap_text_real(scr, "清除");
            fake_set_second_device(true); ev(APP_EVENT_STORAGE_MOUNTED, 1); run(60); tap_text_real(scr, "SD卡 · 12 首"); run(100);     /* device switch */
            ev(APP_EVENT_TRACK_CHANGED, k); run(40);
            lv_screen_load(blank); run(20); ui_Music_screen_destroy(); run(40);
        }
        lv_mem_monitor(&m1);
        long d = (long)m0.free_size - (long)m1.free_size;
        if (m0.total_size == 0) {   /* LV_STDLIB_CLIB: LVGL uses libc malloc, there is no pool to monitor */
            printf("LVGL heap: libc malloc (LV_STDLIB_CLIB), no pool monitor; leaks are caught by LeakSanitizer in the SAN run\n");
        } else {
            printf("LVGL heap free: baseline=%zu after 25 cycles=%zu  (delta=%ld B)\n", m0.free_size, m1.free_size, d);
            CHECK(d >= -64 && d <= 256);
        }
        lv_obj_delete(blank);
    }
    fake_shutdown();
    printf("destroy ok, %d checks passed\n", g_checks);
    return 0;
}
