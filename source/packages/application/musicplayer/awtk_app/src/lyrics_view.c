/**
 * @file lyrics_view.c
 * @brief Scrolling lyrics — label-array implementation.
 *
 * Uses 9 AWTK labels stacked vertically inside a view container.
 * On each seek(), the labels' text content and Y positions are updated.
 * A timer-based animation smoothly slides labels between line changes.
 *
 * Layout (9 lines, center = index 4):
 *   line 0 (top)    — smallest, dimmest
 *   line 1
 *   line 2
 *   line 3
 *   line 4 (CENTER) — highlighted, large font, bright color
 *   line 5
 *   line 6
 *   line 7
 *   line 8 (bottom) — smallest, dimmest
 *
 * Reference: android_ref/.../LyricsView.java onDraw() + seekTo()
 */

#include "lyrics_view.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Colors (hex strings for widget_set_style_str) */
#define COLOR_HIGHLIGHT  "#00FFFF"   /* cyan — current line */
#define COLOR_NEAR       "#999999"   /* ±1 from center */
#define COLOR_MID        "#666666"   /* ±2 */
#define COLOR_FAR        "#444444"   /* ±3 */
#define COLOR_FARTHEST   "#333333"   /* ±4 */
#define COLOR_NO_LRC     "#555555"   /* placeholder */

/* Font sizes by distance from center */
static const int s_font_sizes[LRC_VISIBLE_LINES] = {
    12, 13, 14, 16,   /* above center: far→near */
    22,               /* CENTER (index 4) */
    16, 14, 13, 12    /* below center: near→far */
};

/* Colors by position */
static const char* s_colors[LRC_VISIBLE_LINES] = {
    COLOR_FARTHEST, COLOR_FAR, COLOR_MID, COLOR_NEAR,
    COLOR_HIGHLIGHT,
    COLOR_NEAR, COLOR_MID, COLOR_FAR, COLOR_FARTHEST
};

/* Scroll animation */
#define SCROLL_DURATION_MS   400
#define SCROLL_TICK_MS       16   /* ~60fps */

/* ================================================================
 * Internal: update label texts from LRC data
 * ================================================================ */
static void update_label_texts(lyrics_view_ctx_t* ctx) {
    if (!ctx || !ctx->container) return;

    for (int i = 0; i < LRC_VISIBLE_LINES; i++) {
        if (!ctx->labels[i]) continue;

        int lrc_idx = ctx->cur_line + (i - LRC_CENTER_IDX);

        if (!ctx->lrc || ctx->lrc->count <= 0) {
            /* No lyrics */
            if (i == LRC_CENTER_IDX) {
                widget_set_text_utf8(ctx->labels[i], "♪ ♪ ♪");
            } else {
                widget_set_text_utf8(ctx->labels[i], "");
            }
        } else if (lrc_idx < 0 || lrc_idx >= ctx->lrc->count) {
            widget_set_text_utf8(ctx->labels[i], "");
        } else {
            widget_set_text_utf8(ctx->labels[i], ctx->lrc->lines[lrc_idx].text);
        }
    }
}

/* ================================================================
 * Internal: position labels vertically (with animation offset)
 * ================================================================ */
static void update_label_positions(lyrics_view_ctx_t* ctx) {
    if (!ctx || !ctx->container) return;

    int h = ctx->container->h;
    int lh = ctx->line_height;
    /* Center line (index 4) should be at vertical center of container */
    int center_y = (h - lh) / 2;

    for (int i = 0; i < LRC_VISIBLE_LINES; i++) {
        if (!ctx->labels[i]) continue;
        int base_y = center_y + (i - LRC_CENTER_IDX) * lh;
        int final_y = base_y + (int)ctx->offset_y;
        widget_move(ctx->labels[i], ctx->labels[i]->x, final_y);
    }
}

/* ================================================================
 * Scroll animation timer
 * ================================================================ */
static ret_t scroll_tick(const timer_info_t* info) {
    lyrics_view_ctx_t* ctx = (lyrics_view_ctx_t*)info->ctx;
    if (!ctx) return RET_REMOVE;

    ctx->scroll_elapsed += SCROLL_TICK_MS;

    float t = (float)ctx->scroll_elapsed / (float)ctx->scroll_duration;
    if (t >= 1.0f) t = 1.0f;

    /* Cubic ease-out: 1 - (1-t)^3 */
    float ease = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
    ctx->offset_y = ctx->offset_start + (ctx->offset_target - ctx->offset_start) * ease;

    update_label_positions(ctx);

    if (t >= 1.0f) {
        ctx->offset_y = ctx->offset_target;
        ctx->scroll_anim_id = 0;
        return RET_REMOVE;
    }

    return RET_REPEAT;
}

static void start_scroll(lyrics_view_ctx_t* ctx, float from, float to) {
    if (ctx->scroll_anim_id) {
        timer_remove(ctx->scroll_anim_id);
        ctx->scroll_anim_id = 0;
    }

    ctx->offset_start = from;
    ctx->offset_target = to;
    ctx->scroll_elapsed = 0;
    ctx->scroll_duration = SCROLL_DURATION_MS;

    ctx->scroll_anim_id = timer_add(scroll_tick, ctx, SCROLL_TICK_MS);
}

/* ================================================================
 * Public API
 * ================================================================ */
lyrics_view_ctx_t* lyrics_view_create(widget_t* parent,
                                       xy_t x, xy_t y, wh_t w, wh_t h) {
    lyrics_view_ctx_t* ctx = (lyrics_view_ctx_t*)calloc(1, sizeof(lyrics_view_ctx_t));
    if (!ctx) return NULL;

    ctx->line_height = 28;
    ctx->highlight_size = 22;
    ctx->normal_size = 16;
    ctx->cur_line = -1;
    ctx->offset_y = 0;

    /* Container: a plain view that clips children */
    ctx->container = view_create(parent, x, y, w, h);
    /* No background — transparent over play view bg */

    /* Create label array */
    int center_y = (h - ctx->line_height) / 2;

    for (int i = 0; i < LRC_VISIBLE_LINES; i++) {
        int ly = center_y + (i - LRC_CENTER_IDX) * ctx->line_height;
        widget_t* lbl = label_create(ctx->container, 0, ly, w, ctx->line_height);

        char fs[8];
        snprintf(fs, sizeof(fs), "%d", s_font_sizes[i]);
        widget_set_style_str(lbl, "font_size", fs);
        widget_set_style_str(lbl, "text_color", s_colors[i]);
        widget_set_style_str(lbl, "text_align_h", "center");
        widget_set_text_utf8(lbl, "");

        ctx->labels[i] = lbl;
    }

    /* Show placeholder */
    widget_set_text_utf8(ctx->labels[LRC_CENTER_IDX], "♪ ♪ ♪");

    printf("[lyrics_view] Created: %dx%d, line_h=%d\n", w, h, ctx->line_height);
    return ctx;
}

void lyrics_view_set_data(lyrics_view_ctx_t* ctx, const lrc_data_t* lrc) {
    if (!ctx) return;

    ctx->lrc = lrc;
    ctx->cur_line = -1;
    ctx->offset_y = 0;

    if (ctx->scroll_anim_id) {
        timer_remove(ctx->scroll_anim_id);
        ctx->scroll_anim_id = 0;
    }

    update_label_texts(ctx);
    update_label_positions(ctx);

    printf("[lyrics_view] Data set: %d lines\n", lrc ? lrc->count : 0);
}

void lyrics_view_seek(lyrics_view_ctx_t* ctx, int time_ms) {
    if (!ctx || !ctx->lrc || ctx->lrc->count <= 0) return;

    /* Binary search for current line (same algo as music_app_get_lyrics_line) */
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

    /* Update label texts for new position */
    update_label_texts(ctx);

    /* Animate: slide from offset to 0
     * When line advances by 1, labels need to shift up by line_height.
     * We start offset_y at +line_height and animate to 0.
     * When line goes back, we start at -line_height. */
    if (prev_line >= 0 && result >= 0) {
        int delta = result - prev_line;
        float start_offset = (float)(-delta * ctx->line_height);
        /* Clamp to reasonable range */
        if (start_offset > 100) start_offset = 100;
        if (start_offset < -100) start_offset = -100;
        ctx->offset_y = start_offset;
        start_scroll(ctx, start_offset, 0.0f);
    } else {
        ctx->offset_y = 0;
        update_label_positions(ctx);
    }
}

void lyrics_view_reset(lyrics_view_ctx_t* ctx) {
    if (!ctx) return;

    ctx->lrc = NULL;
    ctx->cur_line = -1;
    ctx->offset_y = 0;

    if (ctx->scroll_anim_id) {
        timer_remove(ctx->scroll_anim_id);
        ctx->scroll_anim_id = 0;
    }

    update_label_texts(ctx);
    update_label_positions(ctx);
}

void lyrics_view_destroy(lyrics_view_ctx_t* ctx) {
    if (!ctx) return;

    if (ctx->scroll_anim_id) {
        timer_remove(ctx->scroll_anim_id);
        ctx->scroll_anim_id = 0;
    }
    /* Labels are children of container, AWTK will destroy them
     * when parent is destroyed. Just free our context. */
    free(ctx);
}