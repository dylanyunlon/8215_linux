/**
 * @file lyrics_view.h
 * @brief vgcanvas custom-drawn scrolling lyrics — Mineradio style.
 *
 * Method 3: Uses EVT_PAINT + vgcanvas (NanoVG GLES2) for:
 *   - Multi-line lyrics with highlighted current line
 *   - Smooth vertical scroll between line changes (cubic ease-out)
 *   - Horizontal auto-scroll for long lines that exceed container width
 *   - Edge fade (gradient mask) at top/bottom boundaries
 *   - Per-line font size / opacity based on distance from center
 *
 * Rendering pipeline:
 *   timer_tick (16ms) → update offsets → widget_invalidate_force
 *   → EVT_PAINT → on_lyrics_paint() → vgcanvas_* calls
 *
 * Reference: Mineradio stage-lyrics + LyricsView.java (Android)
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#ifndef LYRICS_VIEW_H
#define LYRICS_VIEW_H

#include "awtk.h"
#include "music_app.h"   /* lrc_data_t, lrc_line_t */

#ifdef __cplusplus
extern "C" {
#endif

/* Max lines rendered (above + current + below) */
#define LRC_VISIBLE_LINES  9
#define LRC_CENTER_IDX     4   /* index of the center (highlighted) line */

typedef struct _lyrics_view_ctx {
    widget_t*         container;     /* parent view widget (receives EVT_PAINT) */
    const lrc_data_t* lrc;           /* borrowed LRC data pointer */
    int               cur_line;      /* current LRC line index */

    /* Vertical smooth scroll */
    int     scroll_timer_id;     /* AWTK timer ID, 0 = idle */
    float   vy;                  /* current vertical sub-line offset (animated) */
    float   vy_start;            /* offset at animation start */
    float   vy_target;           /* target offset (0 when settled) */
    int     vy_elapsed;          /* ms elapsed */
    int     vy_duration;         /* total animation ms */

    /* Horizontal auto-scroll for long center line */
    float   hx;                  /* current horizontal pixel offset */
    int     hx_timer_id;         /* timer for horizontal scroll, 0 = idle */
    int     hx_pause_ms;         /* initial pause before scroll starts */
    int     hx_paused;           /* ms accumulated in pause phase */
    float   hx_max;              /* max scroll distance (text_w - view_w) */
    int     hx_direction;        /* 1 = scrolling left, -1 = scrolling right (yoyo) */

    /* Appearance params */
    int     line_height;         /* px per line */
    int     container_w;         /* cached width */
    int     container_h;         /* cached height */
    int     fade_height;         /* top/bottom gradient fade zone (px) */
} lyrics_view_ctx_t;

/**
 * Create the lyrics view inside the given parent at (x, y, w, h).
 * The view registers an EVT_PAINT callback for custom NanoVG drawing.
 */
lyrics_view_ctx_t* lyrics_view_create(widget_t* parent,
                                       xy_t x, xy_t y, wh_t w, wh_t h);

/** Set LRC data (borrowed pointer). Resets scroll state. */
void lyrics_view_set_data(lyrics_view_ctx_t* ctx, const lrc_data_t* lrc);

/** Update playback position — finds current line, triggers scroll. */
void lyrics_view_seek(lyrics_view_ctx_t* ctx, int time_ms);

/** Clear to "no lyrics" placeholder. */
void lyrics_view_reset(lyrics_view_ctx_t* ctx);

/** Cleanup (remove timers). Call before parent is destroyed. */
void lyrics_view_destroy(lyrics_view_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif /* LYRICS_VIEW_H */