/**
 * @file music_adapter.h
 * @brief Local-music adapter for hcn_application (same role as adapters/bt).
 *
 * Wraps the existing music_app layer (USB/SD monitor, scanner, player, favourites, lyrics, art)
 * unchanged.  Pure C, no LVGL: the UI is attached through a hook (callback inversion), and
 * every callback runs on the GUI thread inside music_adapter_poll().
 *
 *   main():  ... hcn_ui_boot();  (-> hcn_ui_shared_apps_init() calls music_adapter_init())
 *   loop  :  lv_timer_handler(); ... music_adapter_poll(); ...
 */
#ifndef MUSIC_ADAPTER_H
#define MUSIC_ADAPTER_H

#include <stdbool.h>
#include "music_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Receives every music_app event (device, scan, track, state, position, favourite, error). */
typedef music_app_ui_callback_t MusicAdapterEventHook;

/** Install/replace the event consumers.  Hooks run in order, GUI thread only. NULL removes. */
void music_adapter_add_event_hook(MusicAdapterEventHook hook);
void music_adapter_clear_event_hooks(void);

/** Start USB monitor, scanner, player.  Safe to call once; returns 0 on success, <0 on failure. */
int  music_adapter_init(void);
/** Stop playback, persist state, release everything. */
void music_adapter_deinit(void);
bool music_adapter_is_ready(void);

/** Deliver queued events and fire due timers.  GUI thread, once per main-loop iteration. */
void music_adapter_poll(void);

#ifdef __cplusplus
}
#endif
#endif /* MUSIC_ADAPTER_H */
