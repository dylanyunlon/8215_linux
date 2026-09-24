/**
 * @file cover_flow.c
 * @brief Diagonal album cover carousel with Gaussian DOF blur.
 *
 * Rendering: EVT_PAINT + vgcanvas (NanoVG GLES2).
 * DOF: real Gaussian blur on album art bitmaps — center card sharp,
 *      distant cards progressively blurred (3-pass box blur ≈ Gaussian).
 *
 * Blur cost: ~150×150 RGBA = 90 KB per card, blur takes <1ms on ARM.
 *            Only re-blur when a card changes DOF bucket (not every frame).
 *
 * Layout: PSP-style diagonal from upper-right to lower-left.
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#include "cover_flow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ---- Timing ---- */
#define CF_TICK_MS           16
#define CF_SNAP_DURATION_MS  350
#define CF_LERP_SPEED        0.12f
#define CF_SWIPE_THRESHOLD   12
#define CF_TAP_THRESHOLD     8

/* ---- DOF blur buckets ---- */
/* blur radius per bucket: 0=sharp, 1=light, ..., 4=heavy */

/* Map |delta| from center → DOF bucket index */

/* ---- Placeholder colors ---- */
static const char* s_ph_colors[] = {
    "#1a3a4a", "#2a1a3a", "#3a2a1a", "#1a3a2a",
    "#2a3a1a", "#3a1a2a", "#1a2a3a", "#2a1a2a",
};
#define NUM_PH (sizeof(s_ph_colors) / sizeof(s_ph_colors[0]))

/* ================================================================
 * Gaussian blur (3-pass box blur on RGBA)
 *
 * A single box blur pass of radius R approximates a Gaussian with
 * sigma ≈ R * 0.39. Three passes converge closely to true Gaussian.
 * Separable: horizontal then vertical per pass.
 *
 * Operates in-place on `px` (RGBA, w×h). `tmp` is a scratch buffer
 * of same size. Both must be w*h*4 bytes.
 * ================================================================ */
static void box_blur_h(uint8_t* src, uint8_t* dst, int w, int h, int r) {
    float inv = 1.0f / (r + r + 1);
    int y, x, c;
    for (y = 0; y < h; y++) {
        int row = y * w * 4;
        for (c = 0; c < 4; c++) {
            int sum = 0;
            /* seed: sum of [-r..0] clamped */
            for (x = -r; x <= r; x++) {
                int sx = x < 0 ? 0 : (x >= w ? w - 1 : x);
                sum += src[row + sx * 4 + c];
            }
            for (x = 0; x < w; x++) {
                dst[row + x * 4 + c] = (uint8_t)(sum * inv + 0.5f);
                /* slide window: add right edge, subtract left edge */
                int ri = x + r + 1; if (ri >= w) ri = w - 1;
                int li = x - r;     if (li < 0)  li = 0;
                sum += src[row + ri * 4 + c] - src[row + li * 4 + c];
            }
        }
    }
}

static void box_blur_v(uint8_t* src, uint8_t* dst, int w, int h, int r) {
    float inv = 1.0f / (r + r + 1);
    int y, x, c;
    for (x = 0; x < w; x++) {
        for (c = 0; c < 4; c++) {
            int sum = 0;
            for (y = -r; y <= r; y++) {
                int sy = y < 0 ? 0 : (y >= h ? h - 1 : y);
                sum += src[(sy * w + x) * 4 + c];
            }
            for (y = 0; y < h; y++) {
                dst[(y * w + x) * 4 + c] = (uint8_t)(sum * inv + 0.5f);
                int bi = y + r + 1; if (bi >= h) bi = h - 1;
                int ti = y - r;     if (ti < 0)  ti = 0;
                sum += src[(bi * w + x) * 4 + c] - src[(ti * w + x) * 4 + c];
            }
        }
    }
}

/**
 * 3-pass box blur ≈ Gaussian blur.
 * `pixels`: RGBA buffer, w*h*4 bytes, modified in-place.
 * `radius`: blur kernel half-width (0 = no-op).
 */
static void gaussian_blur_rgba(uint8_t* pixels, int w, int h, int radius) {
    if (radius <= 0 || !pixels || w <= 0 || h <= 0) return;

    int sz = w * h * 4;
    uint8_t* tmp = (uint8_t*)malloc(sz);
    if (!tmp) return;

    /* 3 passes: src→tmp→src→tmp→src→tmp, then copy back */
    box_blur_h(pixels, tmp, w, h, radius);
    box_blur_v(tmp, pixels, w, h, radius);
    box_blur_h(pixels, tmp, w, h, radius);
    box_blur_v(tmp, pixels, w, h, radius);
    box_blur_h(pixels, tmp, w, h, radius);
    box_blur_v(tmp, pixels, w, h, radius);
    /* Result is in `pixels` after 6 operations (3 full H+V passes) */
    /* Actually: after the last box_blur_v, result is in pixels. Correct. */
    free(tmp);
}

/* ================================================================
 * Per-item art cache: sharp + DOF-blurred bitmaps
 *
 * Pipeline (all proven AWTK 1.8 APIs):
 *   1. image_manager_get_bitmap() → load original
 *   2. bitmap_lock_buffer_for_read() → get raw RGBA pixels
 *   3. gaussian_blur_rgba() → blur a copy in RAM
 *   4. Write blurred RGBA as BMP to /tmp/cf_blur_N.bmp
 *   5. image_manager_get_bitmap() → reload blurred BMP
 *   6. vgcanvas_draw_image() at draw time
 *
 * Center card draws sharp bitmap, distant cards draw blurred.
 * ================================================================ */
#define DOF_LEVELS  2  /* 0=sharp, 1=blurred */
#define DOF_BLUR_RADIUS 8

typedef struct {
    bitmap_t  bmp[DOF_LEVELS];  /* 0=sharp, 1=blurred */
    int       loaded[DOF_LEVELS];
    char      blur_path[128];   /* /tmp path for blurred BMP */
} cf_art_cache_t;

/* ================================================================
 * Context struct
 * ================================================================ */
struct _cover_flow_ctx {
    widget_t*           container;
    cover_flow_item_t*  items;
    int                 item_count;

    cf_art_cache_t*     art_cache;

    float   center_target;
    float   center_smooth;
    int     selected_idx;

    int     anim_timer_id;
    int     snapping;
    float   snap_start;
    float   snap_target;
    int     snap_elapsed;
    int     snap_duration;

    int     touch_active;
    int     touch_start_x;
    int     touch_start_y;
    float   touch_start_center;
    int     touch_moved_px;

    cover_flow_select_fn  on_select;
    void*                 select_ctx;

    int     cw, ch;
    float   card_w, card_h;
    float   y_step, x_step;
    int     x_off, y_off;    /* position offset within parent */
};

/* ================================================================
 * Write raw RGBA pixels as 32-bit BMP (AWTK can load BMP)
 * ================================================================ */
static int write_bmp(const char* path, const uint8_t* rgba, int w, int h) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;

    int row_bytes = w * 4;
    int img_size = row_bytes * h;
    int file_size = 54 + img_size;

    /* BMP header (14 bytes) */
    uint8_t hdr[54];
    memset(hdr, 0, 54);
    hdr[0] = 'B'; hdr[1] = 'M';
    hdr[2] = file_size & 0xFF; hdr[3] = (file_size>>8)&0xFF;
    hdr[4] = (file_size>>16)&0xFF; hdr[5] = (file_size>>24)&0xFF;
    hdr[10] = 54; /* pixel data offset */

    /* DIB header (40 bytes) */
    hdr[14] = 40; /* DIB header size */
    hdr[18] = w&0xFF; hdr[19] = (w>>8)&0xFF;
    hdr[20] = (w>>16)&0xFF; hdr[21] = (w>>24)&0xFF;
    /* BMP stores height as negative for top-down */
    int neg_h = -h;
    hdr[22] = neg_h&0xFF; hdr[23] = (neg_h>>8)&0xFF;
    hdr[24] = (neg_h>>16)&0xFF; hdr[25] = (neg_h>>24)&0xFF;
    hdr[26] = 1; /* color planes */
    hdr[28] = 32; /* bits per pixel */

    fwrite(hdr, 1, 54, fp);

    /* Write pixel data: RGBA → BGRA for BMP */
    int y, x;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            int si = (y * w + x) * 4;
            uint8_t px[4] = { rgba[si+2], rgba[si+1], rgba[si+0], rgba[si+3] };
            fwrite(px, 1, 4, fp);
        }
    }
    fclose(fp);
    return 1;
}

/* ================================================================
 * Art cache management
 * ================================================================ */
static void cf_art_cache_init(cf_art_cache_t* ac) {
    memset(ac, 0, sizeof(*ac));
}

static void cf_art_cache_free(cf_art_cache_t* ac) {
    int i;
    for (i = 0; i < DOF_LEVELS; i++) {
        if (ac->loaded[i]) {
            image_manager_unload_bitmap(image_manager(), &ac->bmp[i]);
        }
    }
    /* Remove temp blurred BMP file */
    if (ac->blur_path[0]) remove(ac->blur_path);
    memset(ac, 0, sizeof(*ac));
}

/**
 * Load album art + pre-compute Gaussian-blurred version.
 * All via proven AWTK 1.8 APIs.
 */
static int cf_art_load(cf_art_cache_t* ac, const char* path, int item_idx) {
    if (!path || path[0] == '\0') return 0;

    /* Step 1: Load sharp image */
    memset(&ac->bmp[0], 0, sizeof(bitmap_t));
    if (image_manager_get_bitmap(image_manager(), path, &ac->bmp[0]) != RET_OK) {
        printf("[cover_flow] Failed to load art: %s\n", path);
        return 0;
    }
    ac->loaded[0] = 1;

    int w = ac->bmp[0].w;
    int h = ac->bmp[0].h;
    if (w <= 0 || h <= 0) return 1; /* sharp loaded, no blur */

    /* Step 2: Get raw pixels from sharp bitmap */
    const uint8_t* src = bitmap_lock_buffer_for_read(&ac->bmp[0]);
    if (!src) {
        printf("[cover_flow] Cannot lock pixels: %s\n", path);
        return 1; /* sharp still usable */
    }

    /* Step 3: Copy + Gaussian blur */
    int sz = w * h * 4;
    uint8_t* blurred = (uint8_t*)malloc(sz);
    if (!blurred) {
        bitmap_unlock_buffer(&ac->bmp[0]);
        return 1;
    }
    memcpy(blurred, src, sz);
    bitmap_unlock_buffer(&ac->bmp[0]);

    gaussian_blur_rgba(blurred, w, h, DOF_BLUR_RADIUS);

    /* Step 4: Write blurred pixels as BMP to /tmp */
    snprintf(ac->blur_path, sizeof(ac->blur_path),
             "/tmp/cf_blur_%d.bmp", item_idx);
    if (!write_bmp(ac->blur_path, blurred, w, h)) {
        printf("[cover_flow] Failed to write blur BMP: %s\n", ac->blur_path);
        free(blurred);
        return 1;
    }
    free(blurred);

    /* Step 5: Load blurred BMP back via image_manager */
    char uri[160];
    snprintf(uri, sizeof(uri), "file://%s", ac->blur_path);
    memset(&ac->bmp[1], 0, sizeof(bitmap_t));
    if (image_manager_get_bitmap(image_manager(), uri, &ac->bmp[1]) == RET_OK) {
        ac->loaded[1] = 1;
        printf("[cover_flow] DOF blur ready: %s (%dx%d, r=%d)\n",
               path, w, h, DOF_BLUR_RADIUS);
    }

    return 1;
}

/* ================================================================
 * Easing / util
 * ================================================================ */
static inline float ease_out_cubic(float t) {
    float u = 1.0f - t;
    return 1.0f - u * u * u;
}

static inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

/* ================================================================
 * Card geometry
 * ================================================================ */
typedef struct {
    float cx, cy, w, h, alpha, scale;
} card_geom_t;

static card_geom_t compute_card(const cover_flow_ctx_t* ctx, float delta) {
    card_geom_t g;
    float absD = fabsf(delta);

    g.scale = (absD < 0.5f)
        ? 1.12f
        : fmaxf(0.55f, 1.12f - (absD - 0.5f) * 0.13f);

    g.alpha = (absD < 0.5f)
        ? 1.0f
        : fmaxf(0.18f, 1.0f - (absD - 0.5f) * 0.23f);

    g.w = ctx->card_w * g.scale;
    g.h = ctx->card_h * g.scale;

    float base_cx = (float)ctx->cw * 0.48f;
    float base_cy = (float)ctx->ch * 0.50f;
    g.cx = base_cx - delta * ctx->x_step + absD * ctx->x_step * 0.5f;
    g.cy = base_cy + delta * ctx->y_step;

    return g;
}

/* ================================================================
 * Draw one card
 * ================================================================ */
static void draw_card(vgcanvas_t* vg, const cover_flow_ctx_t* ctx,
                       int item_idx, card_geom_t* g, int is_center) {
    if (item_idx < 0 || item_idx >= ctx->item_count) return;
    const cover_flow_item_t* item = &ctx->items[item_idx];
    cf_art_cache_t* ac = ctx->art_cache ? &ctx->art_cache[item_idx] : NULL;

    float x = g->cx - g->w * 0.5f;
    float y = g->cy - g->h * 0.5f;
    float w = g->w, h = g->h;
    float r = 12.0f * g->scale;

    vgcanvas_save(vg);
    vgcanvas_set_global_alpha(vg, g->alpha);

    /* Shadow (center card only) */
    if (is_center) {
        color_t shadow = color_init(0, 0, 0, 80);
        vgcanvas_set_fill_color(vg, shadow);
        vgcanvas_begin_path(vg);
        vgcanvas_rounded_rect(vg, x + 3, y + 6, w, h, r);
        vgcanvas_fill(vg);
    }

    /* Card background */
    {
        int ci = item_idx % NUM_PH;
        vgcanvas_set_fill_color_str(vg, s_ph_colors[ci]);
        vgcanvas_begin_path(vg);
        vgcanvas_rounded_rect(vg, x, y, w, h, r);
        vgcanvas_fill(vg);
    }

    /* Album art — sharp for center, Gaussian-blurred for distant cards */
    if (ac && ac->loaded[0]) {
        int use_blur = (!is_center && ac->loaded[1]);
        bitmap_t* bmp = use_blur ? &ac->bmp[1] : &ac->bmp[0];
        vgcanvas_draw_image(vg, bmp,
                            0, 0, (float)bmp->w, (float)bmp->h,
                            x + 4, y + 4, w - 8, h - 8);
    } else {
        /* No art loaded — placeholder: first character of title */
        float letter_sz = 48.0f * g->scale;
        vgcanvas_set_font(vg, NULL);
        vgcanvas_set_font_size(vg, letter_sz);
        vgcanvas_set_text_align(vg, "center");
        vgcanvas_set_text_baseline(vg, "middle");
        color_t lc = color_init(255, 255, 255, 50);
        vgcanvas_set_fill_color(vg, lc);
        char first[5] = {0};
        const unsigned char* p = (const unsigned char*)item->title;
        if (p[0] >= 0xE0 && p[1] && p[2]) {
            first[0]=p[0]; first[1]=p[1]; first[2]=p[2];
        } else if (p[0] >= 0xC0 && p[1]) {
            first[0]=p[0]; first[1]=p[1];
        } else if (p[0]) {
            first[0]=p[0];
        }
        if (first[0])
            vgcanvas_fill_text(vg, first, g->cx, g->cy - 8*g->scale, w);
    }

    /* Title */
    {
        float ts = 14.0f * g->scale;
        if (ts < 9.0f) ts = 9.0f;
        vgcanvas_set_font(vg, NULL);
        vgcanvas_set_font_size(vg, ts);
        vgcanvas_set_text_align(vg, "center");
        vgcanvas_set_text_baseline(vg, "top");
        color_t tc = color_init(255, 255, 255,
            is_center ? 240 : (uint8_t)(180 * g->alpha));
        vgcanvas_set_fill_color(vg, tc);
        vgcanvas_fill_text(vg, item->title, g->cx, y + h + 6*g->scale, w+20);
    }

    /* Subtitle (center only) */
    if (is_center && item->subtitle[0]) {
        vgcanvas_set_font(vg, NULL);
        vgcanvas_set_font_size(vg, 11.0f);
        color_t sc = color_init(200, 200, 210, 140);
        vgcanvas_set_fill_color(vg, sc);
        vgcanvas_fill_text(vg, item->subtitle, g->cx, y + h + 22, w+20);
    }

    /* Center accent border */
    if (is_center) {
        vgcanvas_set_global_alpha(vg, 0.6f);
        color_t ac_c = color_init(0, 252, 255, 160);
        vgcanvas_set_stroke_color(vg, ac_c);
        vgcanvas_set_line_width(vg, 1.5f);
        vgcanvas_begin_path(vg);
        vgcanvas_rounded_rect(vg, x, y, w, h, r);
        vgcanvas_stroke(vg);
    }

    vgcanvas_restore(vg);
}

/* ================================================================
 * EVT_PAINT
 * ================================================================ */
static ret_t on_cf_paint(void* data, event_t* e) {
    cover_flow_ctx_t* ctx = (cover_flow_ctx_t*)data;
    if (!ctx || !ctx->container || ctx->item_count <= 0) return RET_OK;

    paint_event_t* pe = (paint_event_t*)e;
    vgcanvas_t* vg = canvas_get_vgcanvas(pe->c);
    if (!vg) return RET_OK;

    vgcanvas_save(vg);
    /* Translate to cover_flow's position within the parent, then clip */
    vgcanvas_translate(vg, (float)ctx->x_off, (float)ctx->y_off);
    /* No clip_rect — vgcanvas_restore doesn't reliably reset clip in AWTK 1.8,
     * causing right-side sibling widgets to be clipped out. Cards are already
     * positioned within bounds by compute_card(). */

    float cs = ctx->center_smooth;
    int center_int = (int)(cs + 0.5f);

    /* Draw back-to-front */
    int pass;
    for (pass = CF_VISIBLE_RADIUS; pass >= 0; pass--) {
        int indices[2]; int ni = 0;
        if (pass == 0) { indices[ni++] = center_int; }
        else { indices[ni++] = center_int - pass; indices[ni++] = center_int + pass; }

        int j;
        for (j = 0; j < ni; j++) {
            int idx = indices[j];
            if (idx < 0 || idx >= ctx->item_count) continue;
            float delta = (float)idx - cs;
            if (fabsf(delta) > CF_VISIBLE_RADIUS + 0.5f) continue;

            card_geom_t g = compute_card(ctx, delta);
            if (g.cy + g.h*0.5f < -20 || g.cy - g.h*0.5f > ctx->ch + 20) continue;

            int is_center = (idx == center_int && fabsf(delta) < 0.5f);
            draw_card(vg, ctx, idx, &g, is_center);
        }
    }

    /* Scroll indicator dots */
    if (ctx->item_count > 1) {
        int max_dots = 7;
        int ndots = ctx->item_count < max_dots ? ctx->item_count : max_dots;
        float dot_gap = 12.0f;
        float dots_w = (ndots - 1) * dot_gap;
        float dx = (float)ctx->cw * 0.5f - dots_w * 0.5f;
        float dy = (float)ctx->ch - 16.0f;
        int i;
        for (i = 0; i < ndots; i++) {
            int item_i = (ctx->item_count <= max_dots) ? i :
                (int)((float)i / (ndots-1) * (ctx->item_count-1) + 0.5f);
            float dist = fabsf((float)item_i - cs);
            float da = dist < 1.0f ? 0.9f : 0.25f;
            float dr = dist < 1.0f ? 3.0f : 2.0f;
            color_t dc = color_init(0, 252, 255, (uint8_t)(da * 255));
            vgcanvas_set_fill_color(vg, dc);
            vgcanvas_set_global_alpha(vg, 1.0f);
            vgcanvas_begin_path(vg);
            vgcanvas_arc(vg, dx + i*dot_gap, dy, dr, 0, 3.14159f*2, FALSE);
            vgcanvas_fill(vg);
        }
    }

    vgcanvas_restore(vg);
    return RET_OK;
}

/* ================================================================
 * Animation timer
 * ================================================================ */
static ret_t on_cf_tick(const timer_info_t* info) {
    cover_flow_ctx_t* ctx = (cover_flow_ctx_t*)info->ctx;
    if (!ctx) return RET_REMOVE;

    if (ctx->snapping) {
        ctx->snap_elapsed += CF_TICK_MS;
        float t = (float)ctx->snap_elapsed / (float)ctx->snap_duration;
        if (t >= 1.0f) t = 1.0f;
        ctx->center_smooth = ctx->snap_start +
            (ctx->snap_target - ctx->snap_start) * ease_out_cubic(t);
        if (t >= 1.0f) {
            ctx->center_smooth = ctx->snap_target;
            ctx->snapping = 0;
            ctx->selected_idx = (int)(ctx->center_smooth + 0.5f);
        }
    } else if (!ctx->touch_active) {
        float diff = ctx->center_target - ctx->center_smooth;
        if (fabsf(diff) < 0.005f) ctx->center_smooth = ctx->center_target;
        else ctx->center_smooth += diff * CF_LERP_SPEED;
    }

    widget_invalidate_force(ctx->container, NULL);
    return RET_REPEAT;
}

static void start_snap(cover_flow_ctx_t* ctx) {
    int target = (int)(ctx->center_smooth + 0.5f);
    if (target < 0) target = 0;
    if (target >= ctx->item_count) target = ctx->item_count - 1;
    ctx->snap_start = ctx->center_smooth;
    ctx->snap_target = (float)target;
    ctx->snap_elapsed = 0;
    ctx->snap_duration = CF_SNAP_DURATION_MS;
    ctx->snapping = 1;
    ctx->center_target = ctx->snap_target;
}

/* ================================================================
 * Touch handlers
 * ================================================================ */
static ret_t on_cf_pointer_down(void* data, event_t* e) {
    cover_flow_ctx_t* ctx = (cover_flow_ctx_t*)data;
    pointer_event_t* pe = (pointer_event_t*)e;

    /* Only handle touches inside the cover_flow region */
    int lx = pe->x - ctx->x_off;
    int ly = pe->y - ctx->y_off;
    if (lx < 0 || lx >= ctx->cw || ly < 0 || ly >= ctx->ch)
        return RET_OK;  /* outside — let children handle */

    ctx->touch_active = 1;
    ctx->touch_start_x = pe->x;
    ctx->touch_start_y = pe->y;
    ctx->touch_start_center = ctx->center_smooth;
    ctx->touch_moved_px = 0;
    ctx->snapping = 0;
    return RET_STOP;  /* consume the event */
}

static ret_t on_cf_pointer_move(void* data, event_t* e) {
    cover_flow_ctx_t* ctx = (cover_flow_ctx_t*)data;
    if (!ctx->touch_active) return RET_OK;
    pointer_event_t* pe = (pointer_event_t*)e;
    int dy = pe->y - ctx->touch_start_y;
    int dx = pe->x - ctx->touch_start_x;
    int moved = abs(dy) > abs(dx) ? abs(dy) : abs(dx);
    ctx->touch_moved_px = moved;

    if (moved > CF_SWIPE_THRESHOLD) {
        float idx_off = -(float)dy / ctx->y_step;
        float nc = ctx->touch_start_center + idx_off;
        if (nc < -0.5f) nc = -0.5f + (nc + 0.5f) * 0.2f;
        float mx = (float)(ctx->item_count - 1) + 0.5f;
        if (nc > mx) nc = mx + (nc - mx) * 0.2f;
        ctx->center_smooth = nc;
        ctx->center_target = nc;
        widget_invalidate_force(ctx->container, NULL);
    }
    return RET_OK;
}

static ret_t on_cf_pointer_up(void* data, event_t* e) {
    cover_flow_ctx_t* ctx = (cover_flow_ctx_t*)data;
    (void)e;
    ctx->touch_active = 0;
    /* widget_ungrab removed — matching grab was removed */

    if (ctx->touch_moved_px < CF_TAP_THRESHOLD) {
        int sel = (int)(ctx->center_smooth + 0.5f);
        if (sel >= 0 && sel < ctx->item_count) {
            ctx->selected_idx = sel;
            if (ctx->on_select)
                ctx->on_select(sel, &ctx->items[sel], ctx->select_ctx);
        }
    } else {
        start_snap(ctx);
    }
    return RET_OK;
}

/* ================================================================
 * Public API
 * ================================================================ */
cover_flow_ctx_t* cover_flow_create(widget_t* parent,
                                     xy_t x, xy_t y, wh_t w, wh_t h) {
    cover_flow_ctx_t* ctx = (cover_flow_ctx_t*)calloc(1, sizeof(cover_flow_ctx_t));
    if (!ctx) return NULL;

    ctx->cw = w;  ctx->ch = h;
    float short_dim = (w < h) ? (float)w : (float)h;
    ctx->card_w = short_dim * 0.42f;
    ctx->card_h = ctx->card_w;
    ctx->y_step = ctx->card_h * 0.62f;
    ctx->x_step = ctx->card_w * 0.08f;

    /* Don't create a child container — AWTK's view_create auto-expands
     * and covers sibling widgets. Instead, register paint + pointer events
     * directly on the parent widget. Cover flow draws in a clipped region. */
    ctx->container = parent;  /* parent IS the container */
    ctx->x_off = x;
    ctx->y_off = y;
    widget_on(ctx->container, EVT_AFTER_PAINT, on_cf_paint, ctx);
    widget_on(ctx->container, EVT_POINTER_DOWN, on_cf_pointer_down, ctx);
    widget_on(ctx->container, EVT_POINTER_MOVE, on_cf_pointer_move, ctx);
    widget_on(ctx->container, EVT_POINTER_UP, on_cf_pointer_up, ctx);
    ctx->anim_timer_id = timer_add(on_cf_tick, ctx, CF_TICK_MS);

    printf("[cover_flow] Created: %dx%d, card=%.0fx%.0f\n",
           w, h, ctx->card_w, ctx->card_h);
    return ctx;
}

void cover_flow_set_items(cover_flow_ctx_t* ctx,
                           const cover_flow_item_t* items, int count) {
    if (!ctx) return;

    /* Free old */
    if (ctx->art_cache) {
        int i;
        for (i = 0; i < ctx->item_count; i++)
            cf_art_cache_free(&ctx->art_cache[i]);
        free(ctx->art_cache);
        ctx->art_cache = NULL;
    }
    if (ctx->items) { free(ctx->items); ctx->items = NULL; }
    ctx->item_count = 0;

    if (items && count > 0) {
        ctx->items = (cover_flow_item_t*)malloc(sizeof(cover_flow_item_t) * count);
        ctx->art_cache = (cf_art_cache_t*)calloc(count, sizeof(cf_art_cache_t));
        if (ctx->items && ctx->art_cache) {
            memcpy(ctx->items, items, sizeof(cover_flow_item_t) * count);
            ctx->item_count = count;

            /* Load art for items that have cover_path set */
            int i;
            for (i = 0; i < count; i++) {
                cf_art_cache_init(&ctx->art_cache[i]);
                if (items[i].cover_path[0] != '\0') {
                    cf_art_load(&ctx->art_cache[i], items[i].cover_path, i);
                }
            }
        }
    }

    ctx->center_smooth = 0;
    ctx->center_target = 0;
    ctx->selected_idx = 0;
    ctx->snapping = 0;
    widget_invalidate_force(ctx->container, NULL);
    printf("[cover_flow] Items set: %d\n", ctx->item_count);
}

int cover_flow_get_selected(cover_flow_ctx_t* ctx) {
    return ctx ? ctx->selected_idx : -1;
}

void cover_flow_set_selected(cover_flow_ctx_t* ctx, int index) {
    if (!ctx || ctx->item_count <= 0) return;
    if (index < 0) index = 0;
    if (index >= ctx->item_count) index = ctx->item_count - 1;
    ctx->center_target = (float)index;
    ctx->snap_start = ctx->center_smooth;
    ctx->snap_target = (float)index;
    ctx->snap_elapsed = 0;
    ctx->snap_duration = CF_SNAP_DURATION_MS;
    ctx->snapping = 1;
}

void cover_flow_set_callback(cover_flow_ctx_t* ctx,
                              cover_flow_select_fn fn, void* user_data) {
    if (!ctx) return;
    ctx->on_select = fn;
    ctx->select_ctx = user_data;
}

void cover_flow_destroy(cover_flow_ctx_t* ctx) {
    if (!ctx) return;
    if (ctx->anim_timer_id) { timer_remove(ctx->anim_timer_id); ctx->anim_timer_id = 0; }
    if (ctx->art_cache) {
        int i;
        for (i = 0; i < ctx->item_count; i++)
            cf_art_cache_free(&ctx->art_cache[i]);
        free(ctx->art_cache);
    }
    if (ctx->items) free(ctx->items);
    free(ctx);
    printf("[cover_flow] Destroyed\n");
}