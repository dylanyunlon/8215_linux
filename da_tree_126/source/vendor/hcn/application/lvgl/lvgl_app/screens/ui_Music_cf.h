/**
 * @file ui_Music_cf.h
 * @brief Cover flow: album-art cards of the tracks around the playing one, on a diagonal axis (centre card 1.12x,
 *        distant cards shrink to 0.55x and fade to 0.22) - the LVGL counterpart of the AWTK cover_flow widget.
 *        Drag vertically to browse, tap a side card to bring it to the centre, tap the centre card to play it.
 *        Art is loaded asynchronously through music_art.  GUI thread only.
 */
#ifndef UI_MUSIC_CF_H
#define UI_MUSIC_CF_H

#include <stdbool.h>
#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void music_cf_create(lv_obj_t *parent, int x, int y, int w, int h);   /* hidden until music_cf_set_visible(true) */
void music_cf_destroy(void);
void music_cf_set_visible(bool visible);                               /* showing recentres on the playing track */
bool music_cf_is_visible(void);
void music_cf_refresh(void);                                           /* playlist / track changed */
void music_cf_on_art_ready(const char *path);                          /* from music_art ready callback */
float music_cf_get_position(void);                                     /* continuous centre index (tests / diagnostics) */

#ifdef __cplusplus
}
#endif
#endif /* UI_MUSIC_CF_H */
