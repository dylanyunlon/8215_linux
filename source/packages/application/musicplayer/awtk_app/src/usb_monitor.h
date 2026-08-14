/**
 * @file usb_monitor.h
 * @brief USB/SD storage hotplug monitor for Linux.
 *
 * Replaces Android BroadcastReceiver (MEDIA_MOUNTED/UNMOUNTED) + FileStorageState.
 * Uses kernel uevent netlink socket to detect block device add/remove,
 * plus /proc/mounts polling for mount-point readiness.
 *
 * Reference: android_ref/autoapps/AutoMediaPlayer/.../FileStorageState.java
 *            android_ref/autoapps/AutoMediaPlayer/.../LocalService.java (mount events)
 *            source/packages/mountservice/src/ATCMountServiceListener.cpp
 *
 * Thread model:
 *   - usb_monitor_start() spawns a background thread
 *   - Callbacks fire from that background thread
 *   - Caller must dispatch to UI thread if needed (via AWTK idle_queue_ex)
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#ifndef USB_MONITOR_H
#define USB_MONITOR_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Storage device types (mirrors Android IConstant path categories)
 *==========================================================================*/
typedef enum {
    STORAGE_TYPE_USB   = 0,   /* /mnt/usb*, /media/usb* */
    STORAGE_TYPE_SD    = 1,   /* /mnt/sd*, /media/sd*   */
    STORAGE_TYPE_FLASH = 2,   /* /mnt/flash, internal   */
} storage_type_t;

/*============================================================================
 * Storage device event
 *==========================================================================*/
typedef enum {
    STORAGE_EVENT_MOUNTED   = 0,
    STORAGE_EVENT_UNMOUNTED = 1,
    STORAGE_EVENT_EJECT     = 2,   /* user-requested eject */
} storage_event_t;

/*============================================================================
 * Storage device info
 *==========================================================================*/
#define STORAGE_PATH_MAX  256

typedef struct {
    storage_type_t  type;
    storage_event_t event;
    char            mount_point[STORAGE_PATH_MAX]; /* e.g. "/mnt/usb0" */
    char            dev_node[STORAGE_PATH_MAX];    /* e.g. "/dev/sda1" */
} storage_device_info_t;

/*============================================================================
 * Callback type — called from monitor thread
 *==========================================================================*/
typedef void (*usb_monitor_callback_t)(const storage_device_info_t* info,
                                       void* user_data);

/*============================================================================
 * Public API
 *==========================================================================*/

/**
 * @brief Start the USB/storage monitor background thread.
 * @param cb        Callback fired on mount/unmount events.
 * @param user_data Opaque pointer passed to cb.
 * @return 0 on success, -1 on failure.
 *
 * Only one monitor instance is supported. Calling start() twice
 * without stop() returns -1.
 */
int usb_monitor_start(usb_monitor_callback_t cb, void* user_data);

/**
 * @brief Stop the monitor thread. Blocks until the thread exits.
 */
void usb_monitor_stop(void);

/**
 * @brief Check if a given path is currently mounted.
 * Reads /proc/mounts synchronously.
 */
bool usb_monitor_is_mounted(const char* path);

/**
 * @brief Scan /proc/mounts and invoke cb once for every USB/SD mount point
 *        found. Useful at startup to discover already-inserted media.
 * @param cb        Same callback type as start().
 * @param user_data Opaque pointer.
 * @return Number of mounted devices found.
 */
int usb_monitor_scan_existing(usb_monitor_callback_t cb, void* user_data);

/**
 * @brief Classify a mount-point path into storage type.
 */
storage_type_t usb_monitor_classify_path(const char* mount_point);

#ifdef __cplusplus
}
#endif

#endif /* USB_MONITOR_H */
