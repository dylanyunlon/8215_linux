/**
 * @file usb_monitor.c
 * @brief USB/SD storage hotplug monitor implementation.
 *
 * Implementation strategy:
 *   1. Open a NETLINK_KOBJECT_UEVENT socket to receive kernel uevents
 *   2. Filter for block device add/remove/change actions
 *   3. After a "add" uevent, poll /proc/mounts with exponential backoff
 *      until the device appears mounted (or timeout)
 *   4. Fire the user callback with mount-point information
 *
 * This replaces:
 *   - Android BroadcastReceiver MEDIA_MOUNTED / MEDIA_UNMOUNTED / MEDIA_EJECT
 *   - ATCMountServiceListener (uevent-based)
 *   - FileStorageState (mount state query)
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#define _DEFAULT_SOURCE  /* for usleep() on glibc */
#include "usb_monitor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <linux/netlink.h>

/*============================================================================
 * Mount-point prefixes (aligned with Android IConstant paths)
 *
 * The actual AC8215 Linux system uses /mnt/usb0, /mnt/usb1, etc.
 * Some builds also use /media/usb*. We check both.
 *==========================================================================*/
static const char* USB_PREFIXES[] = {
    "/mnt/usb", "/media/usb", "/run/media/", "/tmp/mnt/usb", NULL
};
static const char* SD_PREFIXES[] = {
    "/mnt/sd", "/media/sd", "/mnt/ext_sd", "/media/ext_sd", NULL
};

/*============================================================================
 * Singleton state
 *==========================================================================*/
static struct {
    pthread_t           thread;
    volatile bool       running;
    int                 nl_sock;
    usb_monitor_callback_t callback;
    void*               user_data;
    bool                started;
} s_monitor = {
    .thread   = 0,
    .running  = false,
    .nl_sock  = -1,
    .callback = NULL,
    .user_data = NULL,
    .started  = false,
};

static pthread_mutex_t s_mutex = PTHREAD_MUTEX_INITIALIZER;

/*============================================================================
 * Path classification
 *==========================================================================*/
storage_type_t usb_monitor_classify_path(const char* mount_point) {
    if (mount_point == NULL) return STORAGE_TYPE_FLASH;

    const char** p;
    for (p = USB_PREFIXES; *p; p++) {
        if (strncmp(mount_point, *p, strlen(*p)) == 0) {
            return STORAGE_TYPE_USB;
        }
    }
    for (p = SD_PREFIXES; *p; p++) {
        if (strncmp(mount_point, *p, strlen(*p)) == 0) {
            return STORAGE_TYPE_SD;
        }
    }
    return STORAGE_TYPE_FLASH;
}

/*============================================================================
 * /proc/mounts helpers
 *==========================================================================*/

/**
 * Check if dev_path is mounted. If so, copies mount point into mp_buf.
 * Returns true if found.
 */
static bool find_mount_point(const char* dev_path, char* mp_buf, int mp_buf_len) {
    FILE* fp = fopen("/proc/mounts", "r");
    if (!fp) return false;

    char line[1024];
    bool found = false;

    while (fgets(line, sizeof(line), fp)) {
        char dev[256], mp[256], fs[64];
        if (sscanf(line, "%255s %255s %63s", dev, mp, fs) >= 2) {
            if (strcmp(dev, dev_path) == 0) {
                snprintf(mp_buf, mp_buf_len, "%s", mp);
                found = true;
                break;
            }
        }
    }
    fclose(fp);
    return found;
}

bool usb_monitor_is_mounted(const char* path) {
    if (path == NULL) return false;

    FILE* fp = fopen("/proc/mounts", "r");
    if (!fp) return false;

    char line[1024];
    bool found = false;

    while (fgets(line, sizeof(line), fp)) {
        char dev[256], mp[256];
        if (sscanf(line, "%255s %255s", dev, mp) >= 2) {
            if (strcmp(mp, path) == 0) {
                found = true;
                break;
            }
        }
    }
    fclose(fp);
    return found;
}

int usb_monitor_scan_existing(usb_monitor_callback_t cb, void* user_data) {
    if (cb == NULL) return 0;

    FILE* fp = fopen("/proc/mounts", "r");
    if (!fp) return 0;

    char line[1024];
    int count = 0;

    while (fgets(line, sizeof(line), fp)) {
        char dev[256], mp[256], fs[64];
        if (sscanf(line, "%255s %255s %63s", dev, mp, fs) < 2) {
            continue;
        }

        storage_type_t t = usb_monitor_classify_path(mp);
        if (t == STORAGE_TYPE_USB || t == STORAGE_TYPE_SD) {
            storage_device_info_t info;
            memset(&info, 0, sizeof(info));
            info.type = t;
            info.event = STORAGE_EVENT_MOUNTED;
            snprintf(info.mount_point, sizeof(info.mount_point), "%s", mp);
            snprintf(info.dev_node, sizeof(info.dev_node), "%s", dev);
            cb(&info, user_data);
            count++;
        }
    }
    fclose(fp);
    return count;
}

/*============================================================================
 * Mount point cache — maps devname → mount_point for remove events.
 * On "add" we record the mapping; on "remove" we look it up since
 * /proc/mounts no longer has the entry.
 * [GAP-1 fix: replaces broken sda→/mnt/usb0 heuristic]
 *==========================================================================*/
#define MOUNT_CACHE_MAX 16
static struct {
    char devname[64];
    char mount_point[STORAGE_PATH_MAX];
} s_mount_cache[MOUNT_CACHE_MAX];
static int s_mount_cache_count = 0;
static pthread_mutex_t s_cache_mutex = PTHREAD_MUTEX_INITIALIZER;

static void cache_add(const char* devname, const char* mp) {
    pthread_mutex_lock(&s_cache_mutex);
    int i;
    for (i = 0; i < s_mount_cache_count; i++) {
        if (strcmp(s_mount_cache[i].devname, devname) == 0) {
            snprintf(s_mount_cache[i].mount_point, STORAGE_PATH_MAX, "%s", mp);
            pthread_mutex_unlock(&s_cache_mutex);
            return;
        }
    }
    if (s_mount_cache_count < MOUNT_CACHE_MAX) {
        snprintf(s_mount_cache[s_mount_cache_count].devname, 64, "%s", devname);
        snprintf(s_mount_cache[s_mount_cache_count].mount_point, STORAGE_PATH_MAX, "%s", mp);
        s_mount_cache_count++;
    }
    pthread_mutex_unlock(&s_cache_mutex);
}

static bool cache_lookup(const char* devname, char* mp_out, int mp_out_len) {
    pthread_mutex_lock(&s_cache_mutex);
    int i;
    for (i = 0; i < s_mount_cache_count; i++) {
        if (strcmp(s_mount_cache[i].devname, devname) == 0) {
            snprintf(mp_out, mp_out_len, "%s", s_mount_cache[i].mount_point);
            pthread_mutex_unlock(&s_cache_mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&s_cache_mutex);
    return false;
}

static void cache_remove(const char* devname) {
    pthread_mutex_lock(&s_cache_mutex);
    int i;
    for (i = 0; i < s_mount_cache_count; i++) {
        if (strcmp(s_mount_cache[i].devname, devname) == 0) {
            int j;
            for (j = i; j < s_mount_cache_count - 1; j++) {
                s_mount_cache[j] = s_mount_cache[j + 1];
            }
            s_mount_cache_count--;
            break;
        }
    }
    pthread_mutex_unlock(&s_cache_mutex);
}

/*============================================================================
 * Uevent parsing
 *
 * A uevent message is a sequence of null-terminated strings like:
 *   "add@/devices/platform/.../block/sda/sda1\0
 *    ACTION=add\0DEVTYPE=partition\0DEVNAME=sda1\0..."
 *==========================================================================*/

typedef struct {
    const char* action;     /* "add", "remove", "change" */
    const char* devtype;    /* "disk" or "partition" */
    const char* devname;    /* "sda1" */
    const char* devpath;    /* sysfs path */
    const char* subsystem;  /* "block" */
} parsed_uevent_t;

static void parse_uevent(const char* buf, int len, parsed_uevent_t* ev) {
    memset(ev, 0, sizeof(*ev));

    /* First string is "action@devpath" */
    const char* at = strchr(buf, '@');
    if (at) {
        ev->devpath = at + 1;
    }

    /* Subsequent key=value pairs */
    const char* p = buf;
    const char* end = buf + len;

    /* Skip first string */
    p += strlen(p) + 1;

    while (p < end && *p) {
        if (strncmp(p, "ACTION=", 7) == 0) {
            ev->action = p + 7;
        } else if (strncmp(p, "DEVTYPE=", 8) == 0) {
            ev->devtype = p + 8;
        } else if (strncmp(p, "DEVNAME=", 8) == 0) {
            ev->devname = p + 8;
        } else if (strncmp(p, "SUBSYSTEM=", 10) == 0) {
            ev->subsystem = p + 10;
        }
        p += strlen(p) + 1;
    }
}

/**
 * After detecting a block device "add", poll /proc/mounts until
 * it appears (the automounter may take a moment).
 */
static bool wait_for_mount(const char* devname, char* mp_buf, int mp_buf_len,
                           int max_retries) {
    char dev_path[128];
    snprintf(dev_path, sizeof(dev_path), "/dev/%s", devname);

    int i;
    for (i = 0; i < max_retries; i++) {
        if (find_mount_point(dev_path, mp_buf, mp_buf_len)) {
            return true;
        }
        /* Exponential backoff: 200, 400, 800, 1600, 3200 ms */
        usleep((200 * 1000) << (i < 4 ? i : 4));
    }
    return false;
}

/*============================================================================
 * Monitor thread
 *==========================================================================*/
static void* monitor_thread_func(void* arg) {
    (void)arg;

    char buf[4096];

    printf("[usb_monitor] Thread started, listening for uevent...\n");

    while (s_monitor.running) {
        struct pollfd pfd;
        pfd.fd = s_monitor.nl_sock;
        pfd.events = POLLIN;
        pfd.revents = 0;

        int ret = poll(&pfd, 1, 1000); /* 1 second timeout for shutdown check */
        if (ret <= 0) continue;

        if (!(pfd.revents & POLLIN)) continue;

        int len = recv(s_monitor.nl_sock, buf, sizeof(buf) - 1, 0);
        if (len <= 0) continue;
        buf[len] = '\0';

        parsed_uevent_t ev;
        parse_uevent(buf, len, &ev);

        /* We only care about block subsystem events */
        if (!ev.subsystem || strcmp(ev.subsystem, "block") != 0) continue;
        if (!ev.action || !ev.devname) continue;

        /* We only care about partitions, not whole disks (unless it's a
         * partition-less device like some USB sticks) */
        if (ev.devtype && strcmp(ev.devtype, "disk") == 0) {
            /* For now, skip whole-disk events; wait for partition event.
             * If the USB stick has no partition table, the kernel creates
             * a partition-less entry which has devtype=NULL or "disk" but
             * the devname won't have a number suffix. We handle that by
             * also processing devtype==NULL events below. */
            continue;
        }

        printf("[usb_monitor] uevent: action=%s devname=%s devtype=%s\n",
               ev.action, ev.devname, ev.devtype ? ev.devtype : "(null)");

        storage_device_info_t info;
        memset(&info, 0, sizeof(info));
        snprintf(info.dev_node, sizeof(info.dev_node), "/dev/%s", ev.devname);

        if (strcmp(ev.action, "add") == 0) {
            /* Wait for mount, then notify */
            char mp[STORAGE_PATH_MAX];
            if (wait_for_mount(ev.devname, mp, sizeof(mp), 10)) {
                /* Cache the devname→mount_point mapping for later remove */
                cache_add(ev.devname, mp);

                info.event = STORAGE_EVENT_MOUNTED;
                snprintf(info.mount_point, sizeof(info.mount_point), "%s", mp);
                info.type = usb_monitor_classify_path(mp);

                printf("[usb_monitor] MOUNTED: %s -> %s (type=%d)\n",
                       info.dev_node, info.mount_point, info.type);

                pthread_mutex_lock(&s_mutex);
                if (s_monitor.callback) {
                    s_monitor.callback(&info, s_monitor.user_data);
                }
                pthread_mutex_unlock(&s_mutex);
            } else {
                printf("[usb_monitor] add uevent but mount not found: %s\n",
                       ev.devname);
            }
        } else if (strcmp(ev.action, "remove") == 0) {
            info.event = STORAGE_EVENT_UNMOUNTED;
            /* [GAP-1 fix] Look up cached mount point instead of guessing.
             * On remove, /proc/mounts no longer has the entry, so we
             * rely on the cache populated during the earlier "add" event. */
            if (!cache_lookup(ev.devname, info.mount_point, sizeof(info.mount_point))) {
                /* Fallback: try heuristic if cache misses (shouldn't happen normally) */
                if (ev.devname[0] == 's' && ev.devname[1] == 'd') {
                    int drive_idx = ev.devname[2] - 'a';
                    snprintf(info.mount_point, sizeof(info.mount_point),
                             "/mnt/usb%d", drive_idx);
                }
            }
            cache_remove(ev.devname);
            info.type = usb_monitor_classify_path(info.mount_point);

            printf("[usb_monitor] UNMOUNTED: %s (guessed mp=%s type=%d)\n",
                   info.dev_node, info.mount_point, info.type);

            pthread_mutex_lock(&s_mutex);
            if (s_monitor.callback) {
                s_monitor.callback(&info, s_monitor.user_data);
            }
            pthread_mutex_unlock(&s_mutex);
        }
    }

    printf("[usb_monitor] Thread exiting\n");
    return NULL;
}

/*============================================================================
 * Public API
 *==========================================================================*/

int usb_monitor_start(usb_monitor_callback_t cb, void* user_data) {
    pthread_mutex_lock(&s_mutex);

    if (s_monitor.started) {
        pthread_mutex_unlock(&s_mutex);
        fprintf(stderr, "[usb_monitor] Already started\n");
        return -1;
    }

    /* Create netlink socket */
    int sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (sock < 0) {
        fprintf(stderr, "[usb_monitor] socket() failed: %s\n", strerror(errno));
        pthread_mutex_unlock(&s_mutex);
        return -1;
    }

    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 1; /* UEVENT multicast group */

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "[usb_monitor] bind() failed: %s\n", strerror(errno));
        close(sock);
        pthread_mutex_unlock(&s_mutex);
        return -1;
    }

    /* Set non-blocking so poll() works cleanly */
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    s_monitor.nl_sock = sock;
    s_monitor.callback = cb;
    s_monitor.user_data = user_data;
    s_monitor.running = true;
    s_monitor.started = true;

    int ret = pthread_create(&s_monitor.thread, NULL, monitor_thread_func, NULL);
    if (ret != 0) {
        fprintf(stderr, "[usb_monitor] pthread_create failed: %s\n", strerror(ret));
        close(sock);
        s_monitor.started = false;
        s_monitor.running = false;
        pthread_mutex_unlock(&s_mutex);
        return -1;
    }

    pthread_mutex_unlock(&s_mutex);

    printf("[usb_monitor] Started successfully\n");
    return 0;
}

void usb_monitor_stop(void) {
    pthread_mutex_lock(&s_mutex);

    if (!s_monitor.started) {
        pthread_mutex_unlock(&s_mutex);
        return;
    }

    s_monitor.running = false;
    pthread_mutex_unlock(&s_mutex);

    pthread_join(s_monitor.thread, NULL);

    pthread_mutex_lock(&s_mutex);
    if (s_monitor.nl_sock >= 0) {
        close(s_monitor.nl_sock);
        s_monitor.nl_sock = -1;
    }
    s_monitor.callback = NULL;
    s_monitor.user_data = NULL;
    s_monitor.started = false;
    pthread_mutex_unlock(&s_mutex);

    printf("[usb_monitor] Stopped\n");
}
