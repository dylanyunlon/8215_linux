/**
 * @file music_art.h
 * @brief Album-art pipeline: decode (stb_image) -> square ARGB8888 images, async loader with an LRU cache for the
 *        cover-flow, and a blurred "colour wash" backdrop.
 *
 * Threading (LV_USE_OS == LV_OS_NONE, LVGL is single-threaded):
 *   - The worker thread never calls LVGL.  It only uses plain malloc + stb_image + music_app_extract_art().
 *   - Finished images are handed to the GUI thread through idle_queue(); the cache and the ready-callback live there.
 *   - music_art_get() / music_art_set_*() / init / deinit: GUI thread only.
 *   - Images are `lv_image_dsc_t` pointing at malloc'd pixels, so they can be given straight to lv_image_set_src().
 */
#ifndef MUSIC_ART_H
#define MUSIC_ART_H

#include <stdbool.h>
#include <stdint.h>
#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    lv_image_dsc_t dsc;          /* ARGB8888, px x px, data -> pixels */
    uint8_t *pixels;
} music_art_img_t;

#define MUSIC_ART_CACHE_MAX 24

/** Synchronous: embedded art bytes (JPEG/PNG) -> new image, centre-cropped to a square, box-filtered, optional rounded
 *  corners (radius px, anti-aliased).  NULL if undecodable or absurdly large (> 2048x2048).  Thread-safe (no LVGL). */
music_art_img_t *music_art_decode(const uint8_t *data, int size, int px, int radius);
/** Solid rounded square with a faint vinyl disc: shown while art loads / when a file has none.  Thread-safe. */
music_art_img_t *music_art_make_placeholder(int px, int radius, uint32_t bg_rgb, uint32_t disc_rgb);
/** Very small blurred colour wash (w x h) made from `src`: 8x5 average grid, bilinear up-sampled.  Thread-safe. */
music_art_img_t *music_art_make_backdrop(const music_art_img_t *src, int w, int h);
void music_art_free(music_art_img_t *img);

/** Start / stop the worker.  deinit drops pending work and frees the cache. */
void music_art_init(void);
void music_art_deinit(void);

/** Called on the GUI thread whenever an async load finishes (img may then be "no art"). */
typedef void (*music_art_ready_cb)(const char *path, void *user);
void music_art_set_ready_cb(music_art_ready_cb cb, void *user);
/** Called just before a cached image is freed, so users can drop references to it. */
typedef void (*music_art_evict_cb)(const music_art_img_t *img, void *user);
void music_art_set_evict_cb(music_art_evict_cb cb, void *user);

/** Cached image for `path` at px/radius, or NULL.  *known tells "result is final": true + NULL = the file has no art.
 *  On a miss (known == false) an async load is queued (de-duplicated, newest request first) and ready_cb fires later.
 *  The returned pointer stays valid until the entry is evicted (evict_cb) or music_art_deinit(). */
const music_art_img_t *music_art_get(const char *path, int px, int radius, bool *known);

#ifdef __cplusplus
}
#endif
#endif /* MUSIC_ART_H */
