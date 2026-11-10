/**
 * @file ui_Music_cf.c
 */
#include "ui_Music_cf.h"
#include "ui_Music_style.h"
#include "music_app.h"
#include "music_art.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define SLOTS        9                     /* centre +- 4 */
#define RADIUS_SLOTS 4
#define CARD_PX      160                   /* decoded size; the centre card is shown at 1.12x */
#define CARD_RADIUS  16
#define STEP_PX      86.0f                 /* finger travel for one card */
#define SPACING      176.0f                /* base for the perspective spacing */
#define SNAP_MS      240

static struct {
    lv_obj_t *cont, *cards[SLOTS];
    lv_obj_t *lbl_idx, *lbl_title, *lbl_sub, *lbl_now;
    music_art_img_t *placeholder;
    float pos;                             /* continuous index of the centre card */
    int w, h;
    bool visible, dragging, moved, pressed;
    int track_seen;                        /* playing index we last followed */
    int32_t moved_px;
} C;

/* ------------------------------------------------------------------ geometry */
static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

static float scale_of(float d)   { float a = fabsf(d); float s = 1.12f - 0.14f * a; return s < 0.55f ? 0.55f : s; }
static float opa_of(float d)     { float a = fabsf(d); float o = 1.0f - 0.24f * a; return o < 0.18f ? 0.18f : o; }
static float y_of(float d)       { float a = fabsf(d); float y = SPACING * 0.62f * a / (1.0f + 0.18f * a); return d < 0 ? -y : y; }
static float x_of(float d)       { return -d * 16.0f; }                  /* diagonal: lower cards drift left */

static int count_tracks(void) { return music_app_get_playlist_count(); }

/* ------------------------------------------------------------------ layout */
static void caption_update(int j)
{
    const MusicInfo *mi = (j >= 0 && j < count_tracks()) ? music_app_get_track_info(j) : NULL;
    char b[64];
    if (!mi) {
        lv_label_set_text(C.lbl_idx, ""); lv_label_set_text(C.lbl_title, ""); lv_label_set_text(C.lbl_sub, "");
        lv_obj_add_flag(C.lbl_now, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    snprintf(b, sizeof b, "%d / %d", j + 1, count_tracks());
    lv_label_set_text(C.lbl_idx, b);
    lv_label_set_text(C.lbl_title, mi->title[0] ? mi->title : (mi->filename[0] ? mi->filename : "未知歌曲"));
    snprintf(b, sizeof b, "%s", mi->artist[0] ? mi->artist : "未知艺术家");
    lv_label_set_text(C.lbl_sub, b);
    if (j == music_app_get_current_index()) lv_obj_remove_flag(C.lbl_now, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(C.lbl_now, LV_OBJ_FLAG_HIDDEN);
}

static void layout(void)
{
    int n = count_tracks(), c, k, order[SLOTS], i, j;
    float frac, cx = C.w * 0.66f, cy = C.h * 0.49f;

    if (!C.cont) return;
    if (n <= 0) {
        for (i = 0; i < SLOTS; i++) lv_obj_add_flag(C.cards[i], LV_OBJ_FLAG_HIDDEN);
        caption_update(-1);
        return;
    }
    C.pos = clampf(C.pos, 0.0f, (float)(n - 1));
    c = (int)lroundf(C.pos);
    frac = C.pos - (float)c;

    /* far cards first, so the nearest request is queued last and served first (the loader is LIFO) */
    for (i = 0; i < SLOTS; i++) order[i] = i;
    for (i = 0; i < SLOTS; i++) for (j = i + 1; j < SLOTS; j++)
        if (abs(order[j] - RADIUS_SLOTS) > abs(order[i] - RADIUS_SLOTS)) { int t = order[i]; order[i] = order[j]; order[j] = t; }

    for (i = 0; i < SLOTS; i++) {
        int slot = order[i];
        k = slot - RADIUS_SLOTS;
        j = c + k;
        float d = (float)k - frac;
        lv_obj_t *card = C.cards[slot];
        if (j < 0 || j >= n) { lv_obj_add_flag(card, LV_OBJ_FLAG_HIDDEN); continue; }
        {
            const MusicInfo *mi = music_app_get_track_info(j);
            const music_art_img_t *art = NULL;
            bool known = false;
            if (mi && mi->filepath[0]) art = music_art_get(mi->filepath, CARD_PX, CARD_RADIUS, &known);
            {
                const void *want = art ? (const void *)&art->dsc : (const void *)&C.placeholder->dsc;
                if (lv_image_get_src(card) != want) lv_image_set_src(card, want);     /* avoid needless invalidation */
            }
        }
        lv_obj_remove_flag(card, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(card, (int)(cx + x_of(d)) - CARD_PX / 2, (int)(cy + y_of(d)) - CARD_PX / 2);
        lv_image_set_scale(card, (uint32_t)(scale_of(d) * 256.0f));
        lv_obj_set_style_opa(card, (lv_opa_t)(opa_of(d) * 255.0f), 0);
    }
    /* depth order: the farthest card at the back, the centre card on top (done after all cards are positioned) */
    for (i = 0; i < SLOTS; i++) {
        int slot = order[i];
        if (!lv_obj_has_flag(C.cards[slot], LV_OBJ_FLAG_HIDDEN)) lv_obj_move_foreground(C.cards[slot]);
    }
    caption_update(c);
}

/* ------------------------------------------------------------------ animation */
static void anim_exec(void *var, int32_t v)
{
    (void)var;
    C.pos = (float)v / 1000.0f;
    layout();
}

static void animate_to(float target)
{
    int n = count_tracks();
    lv_anim_t a;
    if (n <= 0) return;
    target = clampf(target, 0.0f, (float)(n - 1));
    lv_anim_delete(C.cont, anim_exec);
    if (fabsf(target - C.pos) < 0.001f) { C.pos = target; layout(); return; }
    lv_anim_init(&a);
    lv_anim_set_var(&a, C.cont);
    lv_anim_set_exec_cb(&a, anim_exec);
    lv_anim_set_values(&a, (int32_t)lroundf(C.pos * 1000.0f), (int32_t)lroundf(target * 1000.0f));
    lv_anim_set_duration(&a, SNAP_MS);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

/* ------------------------------------------------------------------ input */
static void on_pressed(lv_event_t *e)
{
    LV_UNUSED(e);
    lv_anim_delete(C.cont, anim_exec);
    C.pressed = true; C.dragging = false; C.moved_px = 0;
}

static void on_pressing(lv_event_t *e)
{
    lv_indev_t *indev = lv_indev_active();
    lv_point_t v;
    LV_UNUSED(e);
    if (!indev || !C.pressed) return;
    lv_indev_get_vect(indev, &v);
    C.moved_px += abs(v.y) + abs(v.x);
    if (!C.dragging && C.moved_px > 10) C.dragging = true;
    if (C.dragging) { C.pos -= (float)v.y / STEP_PX; layout(); }
}

static void on_released(lv_event_t *e)
{
    LV_UNUSED(e);
    if (C.pressed && C.dragging) animate_to(roundf(C.pos));
    C.pressed = false;
}

static void on_short_click(lv_event_t *e)
{
    lv_indev_t *indev = lv_indev_active();
    lv_point_t p;
    lv_area_t a;
    int n = count_tracks(), c, best_k = 99, k;
    float best_d = 99.0f, frac, cx = C.w * 0.66f, cy = C.h * 0.49f;
    LV_UNUSED(e);
    if (C.dragging || !indev || n <= 0) return;                   /* a drag also ends in a short click: ignore it */
    lv_indev_get_point(indev, &p);
    lv_obj_get_coords(C.cont, &a);
    p.x -= a.x1; p.y -= a.y1;
    c = (int)lroundf(C.pos); frac = C.pos - (float)c;
    for (k = -RADIUS_SLOTS; k <= RADIUS_SLOTS; k++) {
        int j = c + k;
        float d = (float)k - frac, half = CARD_PX * scale_of(d) / 2.0f;
        if (j < 0 || j >= n) continue;
        if (fabsf(p.x - (cx + x_of(d))) <= half && fabsf(p.y - (cy + y_of(d))) <= half && fabsf(d) < best_d) { best_d = fabsf(d); best_k = k; }
    }
    if (best_k == 99) return;
    if (best_k == 0 || best_d < 0.5f) {                           /* centre card: play it */
        if (c != music_app_get_current_index()) music_app_safe_play(c);
    } else {
        animate_to((float)(c + best_k));
    }
}

/* ------------------------------------------------------------------ cache eviction */
static void on_evict(const music_art_img_t *img, void *user)
{
    int i;
    LV_UNUSED(user);
    if (!C.cont || !C.placeholder) return;
    for (i = 0; i < SLOTS; i++)
        if (lv_image_get_src(C.cards[i]) == (const void *)&img->dsc) lv_image_set_src(C.cards[i], &C.placeholder->dsc);
}

/* ------------------------------------------------------------------ build */
static lv_obj_t *mk_label(lv_obj_t *parent, const lv_font_t *f, lv_color_t col, int x, int y, int w, int h, lv_label_long_mode_t mode)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_obj_set_style_text_font(l, f, 0);
    lv_obj_set_style_text_color(l, col, 0);
    lv_label_set_long_mode(l, mode);
    lv_obj_set_pos(l, x, y);
    lv_obj_set_size(l, w, h);
    lv_label_set_text(l, "");
    return l;
}

void music_cf_create(lv_obj_t *parent, int x, int y, int w, int h)
{
    int i;
    memset(&C, 0, sizeof C);
    C.w = w; C.h = h;
    C.cont = lv_obj_create(parent);
    lv_obj_remove_flag(C.cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(C.cont, x, y);
    lv_obj_set_size(C.cont, w, h);
    lv_obj_set_style_bg_opa(C.cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(C.cont, 0, 0);
    lv_obj_set_style_outline_width(C.cont, 0, 0);
    lv_obj_set_style_shadow_width(C.cont, 0, 0);
    lv_obj_set_style_pad_all(C.cont, 0, 0);
    lv_obj_set_style_radius(C.cont, 0, 0);
    lv_obj_set_style_clip_corner(C.cont, true, 0);
    lv_obj_add_flag(C.cont, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_HIDDEN);

    C.placeholder = music_art_make_placeholder(CARD_PX, CARD_RADIUS, 0x1F2933, 0x8D9BA8);
    for (i = 0; i < SLOTS; i++) {
        lv_obj_t *card = lv_image_create(C.cont);
        lv_obj_remove_flag(card, LV_OBJ_FLAG_CLICKABLE);               /* the container owns all input */
        lv_image_set_antialias(card, true);
        lv_image_set_pivot(card, CARD_PX / 2, CARD_PX / 2);
        lv_obj_add_flag(card, LV_OBJ_FLAG_HIDDEN);
        C.cards[i] = card;
    }
    C.lbl_idx   = mk_label(C.cont, F_BODY, C_MUTED, 20, 110, 200, 26, LV_LABEL_LONG_DOT);
    C.lbl_title = mk_label(C.cont, F_TITLE, C_TEXT, 20, 142, 200, 3 * lv_font_get_line_height(F_TITLE), LV_LABEL_LONG_DOT);
    C.lbl_sub   = mk_label(C.cont, F_BODY, C_MUTED, 20, 142 + 3 * lv_font_get_line_height(F_TITLE) + 6, 200, 28, LV_LABEL_LONG_DOT);
    C.lbl_now   = mk_label(C.cont, F_BODY, C_ACCENT, 20, 142 + 3 * lv_font_get_line_height(F_TITLE) + 40, 200, 26, LV_LABEL_LONG_CLIP);
    lv_label_set_text(C.lbl_now, "正在播放");
    lv_obj_add_flag(C.lbl_now, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_event_cb(C.cont, on_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(C.cont, on_pressing, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(C.cont, on_released, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(C.cont, on_released, LV_EVENT_PRESS_LOST, NULL);
    lv_obj_add_event_cb(C.cont, on_short_click, LV_EVENT_SHORT_CLICKED, NULL);
    C.track_seen = -1;
    music_art_set_evict_cb(on_evict, NULL);
}

void music_cf_destroy(void)
{
    music_art_set_evict_cb(NULL, NULL);
    if (C.cont) lv_anim_delete(C.cont, anim_exec);
    music_art_free(C.placeholder);
    memset(&C, 0, sizeof C);                                           /* the objects die with the screen */
}

void music_cf_set_visible(bool visible)
{
    if (!C.cont) return;
    C.visible = visible;
    if (visible) {
        int cur = music_app_get_current_index();
        lv_obj_remove_flag(C.cont, LV_OBJ_FLAG_HIDDEN);
        C.pos = cur >= 0 ? (float)cur : 0.0f;
        C.track_seen = cur;
        layout();
    } else {
        lv_anim_delete(C.cont, anim_exec);
        lv_obj_add_flag(C.cont, LV_OBJ_FLAG_HIDDEN);
    }
}

bool music_cf_is_visible(void) { return C.cont && C.visible; }

void music_cf_refresh(void)
{
    int cur;
    if (!C.cont || !C.visible) return;
    cur = music_app_get_current_index();
    if (cur != C.track_seen && cur >= 0 && !C.pressed) { C.track_seen = cur; animate_to((float)cur); }     /* follow the player */
    else layout();
}

void music_cf_on_art_ready(const char *path)
{
    LV_UNUSED(path);
    if (C.cont && C.visible) layout();
}

float music_cf_get_position(void) { return C.pos; }
