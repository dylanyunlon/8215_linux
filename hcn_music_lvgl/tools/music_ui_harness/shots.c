/* Headless driver: real ui_Music screen + fake music_app.  Renders PNG frames and asserts behaviour. */
#include "lvgl/lvgl.h"
#include "ui_Music.h"
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
    lv_obj_t *scr = NULL; ui_Music_screen_init(&scr); lv_screen_load(scr); run(400);

    /* 1. playing, all tracks */
    CHECK(visible_text(scr, "夜曲")); CHECK(visible_text(scr, "U盘 · 48 首")); CHECK(visible_text(scr, "01:23"));
    CHECK(visible_text(scr, "歌词"));
    { lv_mem_monitor_t mm; lv_mem_monitor(&mm); printf("LVGL heap in use with the music screen up: %zu KB of %zu KB\n", (mm.total_size - mm.free_size) / 1024, mm.total_size / 1024); }
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
            ev(APP_EVENT_TRACK_CHANGED, k); run(40);
            lv_screen_load(blank); run(20); ui_Music_screen_destroy(); run(40);
        }
        lv_mem_monitor(&m1);
        long d = (long)m0.free_size - (long)m1.free_size;
        printf("LVGL heap free: baseline=%zu after 25 cycles=%zu  (delta=%ld B)\n", m0.free_size, m1.free_size, d);
        CHECK(d >= -64 && d <= 256);
        lv_obj_delete(blank);
    }
    fake_shutdown();
    printf("destroy ok, %d checks passed\n", g_checks);
    return 0;
}
