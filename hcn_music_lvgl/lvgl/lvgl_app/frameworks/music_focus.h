/**
 * @file music_focus.h
 * @brief Audio-focus bridge for local music (counterpart of bt_focus_adapter for Bluetooth music).
 *
 * Wire it as a music_adapter event hook.  It keeps local music mutually exclusive with Bluetooth
 * music / calls / navigation prompts / CarPlay through the in-process audiofocusmanager.
 */
#ifndef MUSIC_FOCUS_H
#define MUSIC_FOCUS_H

#include "music_app.h"

#ifdef __cplusplus
extern "C" {
#endif

int  music_focus_init(void);                     /* after audiofocusmanager_init() */
void music_focus_deinit(void);
void music_focus_on_app_event(music_app_event_t event, void *param);   /* music_adapter event hook */

#ifdef __cplusplus
}
#endif
#endif /* MUSIC_FOCUS_H */
