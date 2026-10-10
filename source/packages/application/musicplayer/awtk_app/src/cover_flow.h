/**
 * @file cover_flow.h
 * @brief Diagonal album cover carousel — Mineradio PSP-style.
 *
 * Renders a scrollable stack of album cover cards arranged on a
 * right-upper → left-lower diagonal axis, using vgcanvas (NanoVG GLES2).
 *
 * Visual properties (per Mineradio placeCard analysis):
 *   - Scroll axis: mainly vertical (Y step=0.68) with subtle X drift (0.04)
 *   - Center card: scale 1.12×, full opacity
 *   - Distant cards: shrink to 0.55×, fade to 0.22 opacity
 *   - Depth cue: distant cards drawn smaller (simulates Z pushback)
 *   - Touch: swipe up/down to scroll, tap center card to select
 *   - Settle: cubic ease-out snap to nearest integer index
 *
 * Usage:
 *   cover_flow_ctx_t* cf = cover_flow_create(parent, x, y, w, h);
 *   cover_flow_set_items(cf, items, count);
 *   cover_flow_set_callback(cf, on_album_selected, user_data);
 *   // ... later ...
 *   cover_flow_destroy(cf);
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#ifndef COVER_FLOW_H
#define COVER_FLOW_H

#include "awtk.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CF_MAX_TITLE    128
#define CF_MAX_SUBTITLE  64
#define CF_MAX_PATH     512
#define CF_VISIBLE_RADIUS 4   /* draw ±4 cards around center (9 total) */

/* ---- Data item ---- */
typedef struct {
    char title[CF_MAX_TITLE];
    char subtitle[CF_MAX_SUBTITLE];
    char cover_path[CF_MAX_PATH];  /* album art file, "" = placeholder */
    int  uid;                      /* app-defined ID (track/album/folder) */
} cover_flow_item_t;

/* ---- Selection callback ---- */
typedef void (*cover_flow_select_fn)(int index,
                                      const cover_flow_item_t* item,
                                      void* user_data);

/* ---- Opaque context ---- */
typedef struct _cover_flow_ctx cover_flow_ctx_t;

/**
 * Create the cover flow widget inside parent at (x, y, w, h).
 * Registers EVT_PAINT + pointer events for touch scrolling.
 */
cover_flow_ctx_t* cover_flow_create(widget_t* parent,
                                     xy_t x, xy_t y, wh_t w, wh_t h);

/**
 * Set the item array (deep-copied). Resets scroll to index 0.
 * Pass count=0 to clear.
 */
void cover_flow_set_items(cover_flow_ctx_t* ctx,
                           const cover_flow_item_t* items, int count);

/** Get current center (selected) index. */
int cover_flow_get_selected(cover_flow_ctx_t* ctx);

/** Scroll to a specific index (animated). */
void cover_flow_set_selected(cover_flow_ctx_t* ctx, int index);

/** Register tap-on-center callback. */
void cover_flow_set_callback(cover_flow_ctx_t* ctx,
                              cover_flow_select_fn fn, void* user_data);

/** Destroy and free. Call before parent widget is destroyed. */
void cover_flow_destroy(cover_flow_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif /* COVER_FLOW_H */
