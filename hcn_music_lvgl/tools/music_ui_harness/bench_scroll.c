/* 连续滑动列表的渲染基准：真实 ui_Music 界面 + fake_music_app，每帧让列表滚动 N 像素，量 lv_timer_handler() 的耗时。
 * 只量 CPU 软件绘制（x86，FULL 渲染模式，不含板上 lv_opengles_texture 每帧整屏上传纹理的部分），
 * 绝对值不能对应到 A7；用来比较改动前后的相对变化、以及配合 gprof 看时间花在哪（见 bench.sh）。
 * 帧数: BENCH_FRAMES（默认 300）。*/
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
#include <time.h>
static double now_ms(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec*1e3+t.tv_nsec/1e6;}
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


static int g_ptr_dummy;
static void ptr_read(lv_indev_t *i, lv_indev_data_t *d) { LV_UNUSED(i); d->point.x=0; d->point.y=0; d->state=LV_INDEV_STATE_RELEASED; }

/* 连续滑动: 每帧让列表滚动 px 像素，量 lv_timer_handler(=渲染一帧) 的耗时 */
static void bench(const char *name, lv_obj_t *vl, int frames, int px) {
    double tot = 0, mx = 0; int y = 0, dir = 1; int maxy = 40 * 72;
    lv_obj_scroll_to_y(vl, 0, LV_ANIM_OFF); lv_tick_inc(40); lv_timer_handler();
    for (int f = 0; f < frames; f++) {
        y += dir * px; if (y > maxy) { y = maxy; dir = -1; } if (y < 0) { y = 0; dir = 1; }
        lv_obj_scroll_to_y(vl, y, LV_ANIM_OFF);
        lv_tick_inc(33);
        double t0 = now_ms(); lv_timer_handler(); double dt = now_ms() - t0;
        tot += dt; if (dt > mx) mx = dt;
    }
    printf("%-34s 每帧平均 %6.2f ms   最大 %6.2f ms   (%d 帧, 每帧滚 %d px)\n", name, tot / frames, mx, frames, px);
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    lv_init();
    lv_display_t *d = lv_display_create(W, H);
    lv_display_set_buffers(d, buf, NULL, sizeof buf, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(d, flush_cb);
    lv_display_set_color_format(d, LV_COLOR_FORMAT_ARGB8888);
    lv_theme_t *th = lv_theme_default_init(d, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_display_set_theme(d, th);
    fake_init();
    { lv_indev_t *ptr = lv_indev_create(); lv_indev_set_type(ptr, LV_INDEV_TYPE_POINTER); lv_indev_set_read_cb(ptr, ptr_read); lv_indev_set_display(ptr, d); }
    music_app_init(ui_Music_on_app_event);
    lv_obj_t *scr = NULL; ui_Music_screen_init(&scr); lv_screen_load(scr); run(400);
    lv_obj_t *vl = vlist_of(scr); if (!vl) { printf("no vlist\n"); return 1; }
    int n = getenv("BENCH_FRAMES") ? atoi(getenv("BENCH_FRAMES")) : 300;
    bench("热身", vl, 60, 14);
    bench("滑动 14px/帧 (中速)", vl, n, 14);
    bench("滑动 28px/帧 (快速甩动)", vl, n, 28);
    /* 对照: 无变化的帧 */
    { double tot=0; for (int f=0; f<n; f++) { lv_tick_inc(33); double t0=now_ms(); lv_timer_handler(); tot+=now_ms()-t0; } printf("%-34s 每帧平均 %6.2f ms\n","静止(无重绘)", tot/n); }
    return 0;
}
