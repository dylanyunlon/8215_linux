/**
 * @file lyrics_view.c
 * @brief vgcanvas custom-drawn scrolling lyrics — Mineradio style.
 *
 * Uses EVT_PAINT + vgcanvas (NanoVG GLES2 backend) for all rendering.
 * Timer-driven frame loop at 60fps updates offsets and invalidates.
 *
 * Visual design:
 *   - Center line: 22px, cyan (#00FCFF), full opacity
 *   - Adjacent lines: graduated smaller size (18→14→13→12px)
 *   - Adjacent lines: graduated lower opacity (0.6→0.4→0.25→0.15)
 *   - Top/bottom 40px: gradient fade to transparent (edge mask)
 *   - Long center line: auto-scrolls horizontally after 2s pause
 *   - Line change: smooth vertical slide (400ms cubic ease-out)
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#include "lyrics_view.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ---- Appearance constants ---- */
#define SCROLL_V_DURATION_MS   400
#define SCROLL_TICK_MS         16   /* ~60fps */
#define HX_PAUSE_MS            2000 /* pause before horizontal scroll starts */
#define HX_SPEED               0.8f /* px per tick for horizontal scroll */

/* Font sizes by distance from center (index 0=top, 4=center, 8=bottom) */
static const int s_font_sizes[LRC_VISIBLE_LINES] = {
    12, 13, 14, 18,   /* above center: far→near */
    22,               /* CENTER (index 4) */
    18, 14, 13, 12    /* below center: near→far */
};

/* Opacity by distance from center (0.0 → 1.0) */
static const float s_opacities[LRC_VISIBLE_LINES] = {
    0.12f, 0.20f, 0.35f, 0.55f,
    1.00f,
    0.55f, 0.35f, 0.20f, 0.12f
};

/* Center line color (cyan) */
#define LRC_R  0
#define LRC_G  252
#define LRC_B  255

/* Normal line color (white-ish) */
#define NRM_R  200
#define NRM_G  200
#define NRM_B  210

/* ================================================================
 * Cubic ease-out: 1 - (1-t)^3
 * ================================================================ */
static inline float ease_out_cubic(float t) {
    float it = 1.0f - t;
    return 1.0f - it * it * it;
}

/* ================================================================
 * Get line text safely
 * ================================================================ */
static const char* get_line_text(const lyrics_view_ctx_t* ctx, int lrc_idx) {
    if (!ctx->lrc || lrc_idx < 0 || lrc_idx >= ctx->lrc->count)
        return NULL;
    return ctx->lrc->lines[lrc_idx].text;
}

/* ================================================================
 * EVT_PAINT handler — the core rendering function
 * ================================================================ */
static ret_t on_lyrics_paint(void* data, event_t* e) {
    lyrics_view_ctx_t* ctx = (lyrics_view_ctx_t*)data;
    if (!ctx || !ctx->container) return RET_OK;

    paint_event_t* pe = (paint_event_t*)e;
    canvas_t* c = pe->c;
    vgcanvas_t* vg = canvas_get_vgcanvas(c);
    if (!vg) return RET_OK;

    int w = ctx->container_w;
    int h = ctx->container_h;
    int lh = ctx->line_height;
    int center_y = h / 2;
    float vy = ctx->vy;

    vgcanvas_save(vg);

    /* Translate to lyrics area within parent */
    vgcanvas_translate(vg, (float)ctx->x_off, (float)ctx->y_off);

    /* ---- Draw each visible line ---- */
    int i;
    for (i = 0; i < LRC_VISIBLE_LINES; i++) {
        int lrc_idx = ctx->cur_line + (i - LRC_CENTER_IDX);
        const char* text = get_line_text(ctx, lrc_idx);
        if (!text || text[0] == '\0') continue;

        int fs = s_font_sizes[i];
        float alpha = s_opacities[i];
        int is_center = (i == LRC_CENTER_IDX);

        /* Y position: center of line relative to container center */
        float ly = (float)center_y + (float)(i - LRC_CENTER_IDX) * lh + vy - fs * 0.5f;

        /* Skip lines fully outside container */
        if (ly + fs < 0 || ly > h) continue;

        /* Apply edge fade: reduce alpha near top/bottom */
        float fade_zone = (float)ctx->fade_height;
        if (ly < fade_zone) {
            float edge_t = ly / fade_zone;
            if (edge_t < 0) edge_t = 0;
            alpha *= edge_t;
        }
        if (ly + fs > h - fade_zone) {
            float edge_t = (h - ly - fs) / fade_zone;
            if (edge_t < 0) edge_t = 0;
            alpha *= edge_t;
        }

        if (alpha < 0.01f) continue;

        /* Set font */
        vgcanvas_set_font(vg, NULL);
        vgcanvas_set_font_size(vg, (float)fs);
        vgcanvas_set_text_align(vg, is_center ? "left" : "center");
        vgcanvas_set_text_baseline(vg, "top");

        /* Set color */
        if (is_center) {
            vgcanvas_set_fill_color_str(vg, "#00FCFF");
            /* Glow: draw text twice with slight blur via alpha layering */
            vgcanvas_set_global_alpha(vg, alpha * 0.3f);
            /* Slight offset for glow effect */
            float tx = ctx->hx + (float)w * 0.5f;
            if (ctx->hx_max > 0) {
                /* Horizontal scrolling active — left-align with offset */
                tx = 10.0f + ctx->hx;
            } else {
                /* Short text — center it */
                vgcanvas_set_text_align(vg, "center");
                tx = (float)w * 0.5f;
            }
            vgcanvas_fill_text(vg, text, tx, ly + 1.0f, (float)w * 3);
            /* Main text */
            vgcanvas_set_global_alpha(vg, alpha);
            vgcanvas_fill_text(vg, text, tx, ly, (float)w * 3);
        } else {
            int r = NRM_R, g = NRM_G, b = NRM_B;
            color_t clr = color_init(r, g, b, (uint8_t)(alpha * 255));
            vgcanvas_set_fill_color(vg, clr);
            vgcanvas_set_global_alpha(vg, 1.0f); /* alpha already in color */
            vgcanvas_fill_text(vg, text, (float)w * 0.5f, ly, (float)w * 3);
        }
    }

    /* ---- No lyrics placeholder ---- */
    if (!ctx->lrc || ctx->lrc->count <= 0) {
        vgcanvas_set_font(vg, NULL);
        vgcanvas_set_font_size(vg, 20.0f);
        vgcanvas_set_text_align(vg, "center");
        vgcanvas_set_text_baseline(vg, "middle");
        color_t dim = color_init(100, 100, 110, 120);
        vgcanvas_set_fill_color(vg, dim);
        vgcanvas_set_global_alpha(vg, 1.0f);
        vgcanvas_fill_text(vg, "\xe2\x99\xaa \xe2\x99\xaa \xe2\x99\xaa", /* ♪ ♪ ♪ */
                           (float)w * 0.5f, (float)h * 0.5f, (float)w);
    }

    /* ---- Top/bottom gradient fade overlay ---- */
    /* NanoVG linear gradient from bg color (opaque) to transparent */
    {
        float fh = (float)ctx->fade_height;
        /* Top fade */
        vgcanvas_set_global_alpha(vg, 1.0f);
        vgcanvas_begin_path(vg);
        vgcanvas_rect(vg, 0, 0, (float)w, fh);
        /* We draw a semi-transparent background-color rect;
         * The actual bg shows through from behind the container */
        /* Use fill color with alpha gradient approximation:
         * Draw 4 thin strips with decreasing alpha */
        int strip;
        int strips = 4;
        for (strip = 0; strip < strips; strip++) {
            float sy = fh * strip / strips;
            float sh = fh / strips;
            float a = 1.0f - (float)strip / (float)strips;
            color_t sc = color_init(0x0a, 0x0c, 0x10, (uint8_t)(a * 200));
            vgcanvas_set_fill_color(vg, sc);
            vgcanvas_begin_path(vg);
            vgcanvas_rect(vg, 0, sy, (float)w, sh + 1);
            vgcanvas_fill(vg);
        }
        /* Bottom fade */
        for (strip = 0; strip < strips; strip++) {
            float sy = h - fh + fh * strip / strips;
            float sh = fh / strips;
            float a = (float)strip / (float)strips;
            color_t sc = color_init(0x0a, 0x0c, 0x10, (uint8_t)(a * 200));
            vgcanvas_set_fill_color(vg, sc);
            vgcanvas_begin_path(vg);
            vgcanvas_rect(vg, 0, sy, (float)w, sh + 1);
            vgcanvas_fill(vg);
        }
    }

    vgcanvas_restore(vg);
    return RET_OK;
}

/* ================================================================
 * Vertical scroll animation timer
 * ================================================================ */
static ret_t scroll_v_tick(const timer_info_t* info) {
    lyrics_view_ctx_t* ctx = (lyrics_view_ctx_t*)info->ctx;
    if (!ctx) return RET_REMOVE;

    ctx->vy_elapsed += SCROLL_TICK_MS;
    float t = (float)ctx->vy_elapsed / (float)ctx->vy_duration;
    if (t >= 1.0f) t = 1.0f;

    float ease = ease_out_cubic(t);
    ctx->vy = ctx->vy_start + (ctx->vy_target - ctx->vy_start) * ease;

    widget_invalidate_force(ctx->container, NULL);

    if (t >= 1.0f) {
        ctx->vy = ctx->vy_target;
        ctx->scroll_timer_id = 0;
        return RET_REMOVE;
    }
    return RET_REPEAT;
}

static void start_scroll_v(lyrics_view_ctx_t* ctx, float from, float to) {
    if (ctx->scroll_timer_id) {
        timer_remove(ctx->scroll_timer_id);
        ctx->scroll_timer_id = 0;
    }
    ctx->vy_start = from;
    ctx->vy_target = to;
    ctx->vy_elapsed = 0;
    ctx->vy_duration = SCROLL_V_DURATION_MS;
    ctx->scroll_timer_id = timer_add(scroll_v_tick, ctx, SCROLL_TICK_MS);
}

/* ================================================================
 * Horizontal scroll timer (for long center line)
 * ================================================================ */
static ret_t scroll_h_tick(const timer_info_t* info) {
    lyrics_view_ctx_t* ctx = (lyrics_view_ctx_t*)info->ctx;
    if (!ctx) return RET_REMOVE;

    /* Pause phase */
    if (ctx->hx_paused < ctx->hx_pause_ms) {
        ctx->hx_paused += SCROLL_TICK_MS;
        return RET_REPEAT;
    }

    /* Scroll phase */
    ctx->hx += HX_SPEED * ctx->hx_direction;

    if (ctx->hx_direction == -1 && ctx->hx <= -ctx->hx_max) {
        /* Reached left end — pause then scroll back */
        ctx->hx = -ctx->hx_max;
        ctx->hx_direction = 1;
        ctx->hx_paused = 0;
    } else if (ctx->hx_direction == 1 && ctx->hx >= 0) {
        /* Back to start — pause then scroll left again */
        ctx->hx = 0;
        ctx->hx_direction = -1;
        ctx->hx_paused = 0;
    }

    widget_invalidate_force(ctx->container, NULL);
    return RET_REPEAT;
}

static void start_scroll_h(lyrics_view_ctx_t* ctx, float text_width) {
    /* Stop existing horizontal scroll */
    if (ctx->hx_timer_id) {
        timer_remove(ctx->hx_timer_id);
        ctx->hx_timer_id = 0;
    }

    float excess = text_width - (float)(ctx->container_w - 20);
    if (excess <= 0) {
        /* Text fits — no horizontal scroll needed */
        ctx->hx = 0;
        ctx->hx_max = 0;
        return;
    }

    ctx->hx = 0;
    ctx->hx_max = excess + 30; /* extra padding */
    ctx->hx_direction = -1;    /* start scrolling left */
    ctx->hx_paused = 0;
    ctx->hx_pause_ms = HX_PAUSE_MS;
    ctx->hx_timer_id = timer_add(scroll_h_tick, ctx, SCROLL_TICK_MS);
}

/* ================================================================
 * Measure text width (approximate using font size × char count)
 * More accurate: use vgcanvas_measure_text if available
 * ================================================================ */
static float estimate_text_width(const char* text, int font_size) {
    if (!text) return 0;
    /* Rough: CJK chars ~= font_size, ASCII ~= font_size * 0.55 */
    float w = 0;
    const unsigned char* p = (const unsigned char*)text;
    while (*p) {
        if (*p >= 0xE0) {
            /* CJK 3-byte UTF-8 */
            w += (float)font_size * 0.95f;
            p += 3;
        } else if (*p >= 0xC0) {
            w += (float)font_size * 0.7f;
            p += 2;
        } else {
            w += (float)font_size * 0.55f;
            p += 1;
        }
    }
    return w;
}

/* ================================================================
 * Public API
 * ================================================================ */
lyrics_view_ctx_t* lyrics_view_create(widget_t* parent,
                                       xy_t x, xy_t y, wh_t w, wh_t h) {
    lyrics_view_ctx_t* ctx = (lyrics_view_ctx_t*)calloc(1, sizeof(lyrics_view_ctx_t));
    if (!ctx) return NULL;

    ctx->line_height = 32;
    ctx->container_w = w;
    ctx->container_h = h;
    ctx->fade_height = 40;
    ctx->cur_line = -1;
    ctx->vy = 0;
    ctx->hx = 0;

    /* Don't create a child view — view_create auto-expands in AWTK 1.8.
     * Use parent directly, store position offset for translate. */
    ctx->container = parent;
    ctx->x_off = x;
    ctx->y_off = y;

    /* Register custom paint handler on parent */
    widget_on(ctx->container, EVT_AFTER_PAINT, on_lyrics_paint, ctx);

    printf("[lyrics_view] Created (vgcanvas): %dx%d, line_h=%d, fade=%d\n",
           w, h, ctx->line_height, ctx->fade_height);
    return ctx;
}

void lyrics_view_set_data(lyrics_view_ctx_t* ctx, const lrc_data_t* lrc) {
    if (!ctx) return;

    ctx->lrc = lrc;
    ctx->cur_line = -1;
    ctx->vy = 0;
    ctx->hx = 0;
    ctx->hx_max = 0;

    /* Stop timers */
    if (ctx->scroll_timer_id) {
        timer_remove(ctx->scroll_timer_id);
        ctx->scroll_timer_id = 0;
    }
    if (ctx->hx_timer_id) {
        timer_remove(ctx->hx_timer_id);
        ctx->hx_timer_id = 0;
    }

    widget_invalidate_force(ctx->container, NULL);
    printf("[lyrics_view] Data set: %d lines\n", lrc ? lrc->count : 0);
}

void lyrics_view_seek(lyrics_view_ctx_t* ctx, int time_ms) {
    if (!ctx || !ctx->lrc || ctx->lrc->count <= 0) return;

    /* Binary search for current line */
    int result = -1;
    if (time_ms >= ctx->lrc->lines[0].time_ms) {
        int lo = 0, hi = ctx->lrc->count - 1;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            if (ctx->lrc->lines[mid].time_ms <= time_ms) {
                result = mid;
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
    }

    if (result == ctx->cur_line) return; /* no change */

    int prev_line = ctx->cur_line;
    ctx->cur_line = result;

    /* Vertical scroll animation */
    if (prev_line >= 0 && result >= 0) {
        int delta = result - prev_line;
        float start_offset = (float)(-delta * ctx->line_height);
        if (start_offset > 100) start_offset = 100;
        if (start_offset < -100) start_offset = -100;
        ctx->vy = start_offset;
        start_scroll_v(ctx, start_offset, 0.0f);
    } else {
        ctx->vy = 0;
        widget_invalidate_force(ctx->container, NULL);
    }

    /* Check if new center line needs horizontal scroll */
    if (result >= 0) {
        const char* text = ctx->lrc->lines[result].text;
        float tw = estimate_text_width(text, s_font_sizes[LRC_CENTER_IDX]);
        start_scroll_h(ctx, tw);
    }
}

void lyrics_view_reset(lyrics_view_ctx_t* ctx) {
    if (!ctx) return;

    ctx->lrc = NULL;
    ctx->cur_line = -1;
    ctx->vy = 0;
    ctx->hx = 0;
    ctx->hx_max = 0;

    if (ctx->scroll_timer_id) {
        timer_remove(ctx->scroll_timer_id);
        ctx->scroll_timer_id = 0;
    }
    if (ctx->hx_timer_id) {
        timer_remove(ctx->hx_timer_id);
        ctx->hx_timer_id = 0;
    }

    widget_invalidate_force(ctx->container, NULL);
}

void lyrics_view_destroy(lyrics_view_ctx_t* ctx) {
    if (!ctx) return;

    if (ctx->scroll_timer_id) {
        timer_remove(ctx->scroll_timer_id);
        ctx->scroll_timer_id = 0;
    }
    if (ctx->hx_timer_id) {
        timer_remove(ctx->hx_timer_id);
        ctx->hx_timer_id = 0;
    }
    /* Container widget is a child of parent — AWTK destroys it.
     * Just free our context struct. */
    free(ctx);
}
