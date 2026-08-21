/**
 * @file favorite_manager.h
 * @brief Music favorite (collection) list manager.
 *
 * C implementation of Android AutoMediaPlayer's FavoriteManager.java.
 *
 * Features:
 *   - Add/remove/query favorites by filepath
 *   - Persist to a simple text DB file
 *   - Max capacity limit (128, matching Android MAX_FAVORITE_INFO_THRESHOLD)
 *   - Event callback for UI refresh
 *
 * Thread safety: All public functions are thread-safe (internal mutex).
 *
 * File location: source/packages/application/musicplayer/awtk_app/src/favorite_manager.h
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#ifndef FAVORITE_MANAGER_H
#define FAVORITE_MANAGER_H

#include "music_scanner.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Soft limit — Android: MAX_FAVORITE_INFO_THRESHOLD = 128.
 * Now a warning threshold, not a hard cutoff.
 * darray will grow beyond this if user insists. */
#define FAVORITE_SOFT_LIMIT  128

/* Persistence file path */
#define FAVORITE_DB_PATH    "/data/music/favorites.db"

/* Favorite operation types (mirrors Android FavoriteManager.OPERATE_*) */
typedef enum {
    FAVORITE_OP_INITED    = 0,
    FAVORITE_OP_ADD       = 1,
    FAVORITE_OP_REMOVE    = 2,
    FAVORITE_OP_UPDATE    = 3,
    FAVORITE_OP_MAX_LIMIT = 4,
} favorite_op_t;

/**
 * @brief Callback for favorite list changes.
 * @param op      Operation type
 * @param info    The affected item (NULL for INITED/UPDATE/MAX_LIMIT)
 * @param index   Index in the favorite list (-1 if N/A)
 */
typedef void (*favorite_callback_t)(favorite_op_t op,
                                    const MusicInfo* info,
                                    int index);

/**
 * @brief Initialize the favorite manager. Loads from DB.
 * @param cb  Optional callback for state changes.
 * @return 0 on success.
 */
int favorite_init(favorite_callback_t cb);

/**
 * @brief Shutdown and persist to DB.
 */
void favorite_deinit(void);

/**
 * @brief Add a track to favorites.
 * @return true if added, false if already exists or at limit.
 */
bool favorite_add(const MusicInfo* info);

/**
 * @brief Remove a track from favorites (by filepath match).
 * @return true if removed.
 */
bool favorite_remove(const char* filepath);

/**
 * @brief Check if a filepath is in the favorites list.
 */
bool favorite_contains(const char* filepath);

/**
 * @brief Toggle favorite state for a filepath.
 * @return true if now favorited, false if un-favorited.
 */
bool favorite_toggle(const MusicInfo* info);

/**
 * @brief Get the favorites list (read-only).
 * @param out_count  Receives the number of favorites.
 * @return Pointer to internal array. Valid until next add/remove.
 */
const MusicInfo* favorite_get_list(int* out_count);

/**
 * @brief Save favorites to persistent storage.
 */
void favorite_save(void);

/**
 * @brief Validate favorites against a file scan result.
 *        Removes entries whose files no longer exist.
 * @param music_list  Current scan result to validate against.
 * @return Number of entries removed.
 */
int favorite_validate(const MusicList* music_list);

#ifdef __cplusplus
}
#endif

#endif /* FAVORITE_MANAGER_H */
