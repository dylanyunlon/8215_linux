/* 连续滑动列表的渲染基准：真实 ui_Music 界面 + fake_music_app，每帧让列表滚动 N 像素，量 lv_timer_handler() 的耗时。
 * 只量 CPU 软件绘制（x86，FULL 渲染模式，不含板上 lv_opengles_texture 每帧整屏上传纹理的部分），
 * 绝对值不能对应到 A7；用来比较改动前后的相对变化、以及配合 gprof 看时间花在哪（见 bench.sh）。
 * 帧数: BENCH_FRAMES（默认 300）。BENCH_PROBE=1: 另外量 clip_corner 开关和逐个隐藏行内元素的单帧开销。
 * BENCH_TASKS=1: 数连续滑动时每帧各类绘制任务的个数、按行内角色分开的文字任务数、静态/非静态文字，用来估算板上 GL 绘制单元的负担（FULL 渲染模式会连左侧面板一起画，板上 DIRECT 只画脏区，看行内角色那几行）。*/
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



/* ---- 绘制任务统计: 数每个刷新里各类绘制任务的个数（板上 GL 绘制单元按任务类型走不同路径） ---- */
static int g_cnt[32]; static int g_lbl_nonstatic, g_lbl_static; static long g_lbl_px, g_other_px, g_lbl_chars;
static struct { lv_obj_t *o; int n; int stat; } g_obj_cnt[256]; static int g_obj_n;
static void tasks_cb(lv_event_t *e) {
    lv_draw_task_t *t = lv_event_get_draw_task(e);
    lv_draw_task_type_t ty = lv_draw_task_get_type(t);
    if ((int)ty < 32) g_cnt[(int)ty]++;
    lv_area_t a; lv_draw_task_get_area(t, &a); long px = (long)lv_area_get_width(&a) * lv_area_get_height(&a);
    if (ty == LV_DRAW_TASK_TYPE_LABEL) {
        lv_draw_label_dsc_t *d = lv_draw_task_get_label_dsc(t);
        if (d && d->text_static) g_lbl_static++; else g_lbl_nonstatic++;
        { lv_obj_t *ob = ((lv_draw_dsc_base_t *)d)->obj; int k; for (k = 0; k < g_obj_n; k++) if (g_obj_cnt[k].o == ob) break;
          if (k == g_obj_n && g_obj_n < 256) { g_obj_cnt[k].o = ob; g_obj_cnt[k].n = 0; g_obj_n++; } if (k < 256) g_obj_cnt[k].n++; }
        if (d && d->text) g_lbl_chars += (long)lv_strlen(d->text);
        g_lbl_px += px;
    } else g_other_px += px;
}
static void flag_tree(lv_obj_t *o) {
    lv_obj_add_flag(o, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
    lv_obj_add_event_cb(o, tasks_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
    uint32_t n = lv_obj_get_child_count(o); for (uint32_t i = 0; i < n; i++) flag_tree(lv_obj_get_child(o, i));
}
static const char *ty_name(int t) {
    switch (t) { case LV_DRAW_TASK_TYPE_FILL: return "FILL(填充)"; case LV_DRAW_TASK_TYPE_BORDER: return "BORDER(边框)";
      case LV_DRAW_TASK_TYPE_BOX_SHADOW: return "BOX_SHADOW(阴影)"; case LV_DRAW_TASK_TYPE_LABEL: return "LABEL(文字)";
      case LV_DRAW_TASK_TYPE_IMAGE: return "IMAGE(图片)"; case LV_DRAW_TASK_TYPE_LAYER: return "LAYER(图层)";
      case LV_DRAW_TASK_TYPE_LINE: return "LINE"; case LV_DRAW_TASK_TYPE_ARC: return "ARC"; case LV_DRAW_TASK_TYPE_TRIANGLE: return "TRIANGLE";
      case LV_DRAW_TASK_TYPE_MASK_RECTANGLE: return "MASK_RECT"; case LV_DRAW_TASK_TYPE_MASK_BITMAP: return "MASK_BITMAP"; default: return "其它"; }
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
    if (getenv("BENCH_PROBE")) {
        int nrow = (int)lv_obj_get_child_count(vl);
        printf("vlist 子对象 %d 个, 半径 %d, clip_corner=%d\n", nrow, (int)lv_obj_get_style_radius(vl, 0), (int)lv_obj_get_style_clip_corner(vl, 0));
        bench("基线", vl, 400, 14);
        lv_obj_set_style_clip_corner(vl, false, 0);
        bench("关闭 vlist 的 clip_corner", vl, 400, 14);
        lv_obj_set_style_clip_corner(vl, true, 0);
        /* 逐个隐藏每一行的第 k 个子对象(用透明度 0，行重绑定时不会被重置) */
        lv_obj_t *r0 = NULL; for (int i = 0; i < nrow; i++) { lv_obj_t *c = lv_obj_get_child(vl, i); if (lv_obj_get_child_count(c) > 2) { r0 = c; break; } }
        int nk = r0 ? (int)lv_obj_get_child_count(r0) : 0;
        printf("每行子对象 %d 个\n", nk);
        for (int k = 0; k < nk; k++) {
            lv_obj_t *c0 = lv_obj_get_child(r0, k);
            char nm[96]; const char *t = lv_obj_check_type(c0, &lv_label_class) ? lv_label_get_text(c0) : "(非文本)";
            snprintf(nm, sizeof nm, "隐藏第 %d 个 [%.14s] %dx%d", k, t, (int)lv_obj_get_width(c0), (int)lv_obj_get_height(c0));
            for (int i = 0; i < nrow; i++) { lv_obj_t *r = lv_obj_get_child(vl, i); if (lv_obj_get_child_count(r) == (uint32_t)nk) lv_obj_set_style_opa(lv_obj_get_child(r, k), 0, 0); }
            bench(nm, vl, 400, 14);
            for (int i = 0; i < nrow; i++) { lv_obj_t *r = lv_obj_get_child(vl, i); if (lv_obj_get_child_count(r) == (uint32_t)nk) lv_obj_set_style_opa(lv_obj_get_child(r, k), LV_OPA_COVER, 0); }
        }
        return 0;
    }

    if (getenv("BENCH_TASKS")) {
        flag_tree(scr);
        lv_obj_scroll_to_y(vl, 0, LV_ANIM_OFF); lv_tick_inc(40); lv_timer_handler();
        const int N = 100, PX = 14; int y = 0;
        memset(g_cnt, 0, sizeof g_cnt); g_lbl_nonstatic = g_lbl_static = 0; g_lbl_px = g_other_px = g_lbl_chars = 0;
        for (int f = 0; f < N; f++) { y += PX; lv_obj_scroll_to_y(vl, y, LV_ANIM_OFF); lv_tick_inc(33); lv_timer_handler(); }
        printf("连续滑动 %d 帧(每帧 %d px)，平均每帧的绘制任务:\n", N, PX);
        int total = 0; for (int t = 0; t < 32; t++) if (g_cnt[t]) { printf("  %-22s %6.1f 个\n", ty_name(t), g_cnt[t] / (double)N); total += g_cnt[t]; }
        printf("  %-22s %6.1f 个\n", "合计", total / (double)N);
        { /* 按行内角色汇总: 行 = vlist 的子对象，子对象顺序 bar,lead,title,sub,dur,fav,chev */
          const char *role[] = {"bar","lead(序号/图标)","title(标题)","sub(歌手·专辑)","dur(时长)","fav(♥)","chev(›)"}; double agg[7] = {0};
          uint32_t nr = lv_obj_get_child_count(vl);
          for (uint32_t i = 0; i < nr; i++) { lv_obj_t *row = lv_obj_get_child(vl, i); if (lv_obj_get_child_count(row) != 7) continue;
              for (int c = 0; c < 7; c++) { lv_obj_t *ch = lv_obj_get_child(row, c); for (int k = 0; k < g_obj_n; k++) if (g_obj_cnt[k].o == ch) agg[c] += g_obj_cnt[k].n; } }
          printf("按行内角色，平均每帧的文字任务数:\n"); for (int c = 1; c < 7; c++) printf("  %-18s %5.1f 个\n", role[c], agg[c] / N); }
        printf("其中文字: 非静态 %.1f 个 + 静态 %.1f 个；平均每帧文字约 %.0f 个字符\n", g_lbl_nonstatic / (double)N, g_lbl_static / (double)N, g_lbl_chars / (double)N);
        printf("文字任务面积合计 %.0f 万像素/帧 (按 ARGB8888 约 %.2f MB/帧要光栅化并上传); 其它任务面积 %.0f 万像素/帧\n",
               g_lbl_px / (double)N / 1e4, g_lbl_px / (double)N * 4 / 1048576.0, g_other_px / (double)N / 1e4);
        return 0;
    }
    bench("热身", vl, 60, 14);
    bench("滑动 14px/帧 (中速)", vl, n, 14);
    bench("滑动 28px/帧 (快速甩动)", vl, n, 28);
    /* 对照: 无变化的帧 */
    { double tot=0; for (int f=0; f<n; f++) { lv_tick_inc(33); double t0=now_ms(); lv_timer_handler(); tot+=now_ms()-t0; } printf("%-34s 每帧平均 %6.2f ms\n","静止(无重绘)", tot/n); }
    return 0;
}
