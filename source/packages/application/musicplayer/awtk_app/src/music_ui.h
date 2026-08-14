/**
 * @file music_ui.h
 * @brief AWTK UI layer for the music player.
 *
 * Creates and manages all AWTK widgets:
 *   - Song title / artist / album labels
 *   - Play/pause/prev/next buttons
 *   - Progress slider
 *   - Playlist list_view
 *   - Play mode indicator
 *   - Storage device status
 *   - Scanning progress indicator
 *
 * Reference: Android AutoMediaPlayer MusicInfoLayout / MusicListLayout
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#ifndef MUSIC_UI_H
#define MUSIC_UI_H

#include "awtk.h"
#include "music_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create the music player UI on the given window.
 * @param win   The main AWTK window (already created).
 * @return RET_OK on success.
 *
 * Call this after music_app_init(). This function:
 *   1. Creates all child widgets on `win`
 *   2. Binds button clicks → music_app_xxx()
 *   3. Registers itself as the music_app UI callback
 */
ret_t music_ui_create(widget_t* win);

/**
 * @brief Destroy the music UI (cleanup resources).
 * Called before music_app_deinit().
 */
void music_ui_destroy(void);

/**
 * @brief The UI event handler — pass as music_app_ui_callback_t to music_app_init().
 * Receives events on AWTK main thread and updates widgets.
 */
void music_ui_on_app_event(music_app_event_t event, void* param);

#ifdef __cplusplus
}
#endif

#endif /* MUSIC_UI_H */
