/**
 * @file lyrics_view.h
 * @brief Scrolling lyrics display — uses a plain AWTK label with
 *        manual text rendering via position callback.
 *
 * Instead of a full custom widget (which needs vtable registration
 * that varies across AWTK versions), we use a simple approach:
 *   - A group of labels inside a view container
 *   - lyrics_view_seek() updates label texts + positions each tick
 *   - Smooth scroll via incremental Y offset
 *
 * This matches the proven Issue #60 "5 static labels" approach
 * but with more lines and animated vertical offset.
 *
 * Reference: android_ref/.../LyricsView.java
 */

#ifndef LYRICS_VIEW_H
#define LYRICS_VIEW_H

#include "awtk.h"
#include "music_app.h"   /* lrc_data_t, lrc_line_t */

#ifdef __cplusplus
extern "C" {
#endif

/* Max lines displayed simultaneously (above + current + below) */
#define LRC_VISIBLE_LINES  9
#define LRC_CENTER_IDX     4   /* index of the center (highlighted) line */

typedef struct _lyrics_view_ctx {
    widget_t*         container;                    /* parent view widget */
    widget_t*         labels[LRC_VISIBLE_LINES];    /* label widgets */
    const lrc_data_t* lrc;                          /* borrowed LRC data */
    int               cur_line;                     /* current LRC line index */

    /* Smooth scroll state */
    int     scroll_anim_id;   /* AWTK timer ID, 0 = idle */
    float   offset_y;         /* current sub-line pixel offset (animated) */
    float   offset_start;     /* offset at animation start */
    float   offset_target;    /* target offset (0 when settled) */
    int     scroll_elapsed;   /* ms elapsed in animation */
    int     scroll_duration;  /* total animation ms */

    /* Appearance */
    int     line_height;      /* px per line (font + padding) */
    int     highlight_size;   /* center line font size */
    int     normal_size;      /* other lines font size */
} lyrics_view_ctx_t;

/**
 * Create the lyrics view inside the given parent at (x, y, w, h).
 * Returns the context (caller keeps pointer, freed by lyrics_view_destroy).
 */
lyrics_view_ctx_t* lyrics_view_create(widget_t* parent,
                                       xy_t x, xy_t y, wh_t w, wh_t h);

/** Set LRC data (borrowed pointer). Resets scroll. */
void lyrics_view_set_data(lyrics_view_ctx_t* ctx, const lrc_data_t* lrc);

/** Update playback position — finds current line, scrolls, updates labels. */
void lyrics_view_seek(lyrics_view_ctx_t* ctx, int time_ms);

/** Clear to "no lyrics" state. */
void lyrics_view_reset(lyrics_view_ctx_t* ctx);

/** Cleanup (remove timers). Call before parent is destroyed. */
void lyrics_view_destroy(lyrics_view_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif /* LYRICS_VIEW_H */