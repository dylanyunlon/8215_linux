/**
 * @file ui_Music.h
 * @brief Local-music screen (LVGL v9, 1024x600) for hcn_application.
 *
 * View only.  Data and behaviour come from the unchanged music_app layer
 * (music_app.h); events arrive on the GUI thread via music_adapter_poll().
 */
#ifndef UI_MUSIC_H
#define UI_MUSIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"
#include "music_app.h"

extern lv_obj_t *ui_Music;

/* app_desc_t.init_cb / destroy_cb signature (see displayfocusmanager.h) */
void ui_Music_screen_init(lv_obj_t **screen);
void ui_Music_screen_destroy(void);

/** Full resync from music_app state.  Call from app_desc_t.on_enter. */
void ui_Music_refresh(void);

/** music_app_ui_callback_t: pass to music_app_init().  GUI thread only. */
void ui_Music_on_app_event(music_app_event_t event, void *param);

#ifdef __cplusplus
}
#endif
#endif /* UI_MUSIC_H */
