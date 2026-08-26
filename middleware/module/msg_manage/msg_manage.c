/**
 * @brief msg_manage.c -- 存储插拔消息(/proc/mounts 轮询 diff + 多订阅者分发)
 * worker 线程周期读 /proc/mounts，过滤关注前缀(/media/ /mnt/ 等)挂载，快照 diff，
 * 新增->MOUNTED、消失->UNMOUNTED，经订阅者表分发(锁内快照、锁外调用)。
 * 范式同 key_common(pub/sub) + watchdog(worker 线程 + done-sem join + self_check)。 
 * 
 */
#include "msg_manage.h"

#if MW_MSG_MANAGE_ENABLE

#include "mw_modules.h"
#include "mw_lock.h"
#include "mw_pthread.h"
#include "mw_log.h"
#include "osal.h"
#include "callback_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define MSG_DEV_LEN 64
#define MSG_MNT_LEN 128
#define MSG_FS_LEN  16

typedef struct {
    int used;
    char device[MSG_DEV_LEN];
    char mount_point[MSG_MNT_LEN];
    char fs_type[MSG_FS_LEN];
} mount_entry_t;

/* ---- 订阅者表(key_common 范式) ---- */
static msg_manage_cb_t s_cbs[MW_MAX_CALLBACK_NUM] = {0};
static uint8_t s_cb_count = 0;
static osal_mutex_t s_cb_lock = OSAL_MUTEX_INIT;

/* ---- 挂载快照(worker 写、diff 读) ---- */
static mount_entry_t s_cur[MW_MSG_MAX_DEVS];
static osal_mutex_t s_lock = OSAL_MUTEX_INIT;

static volatile int s_running = 0;
static osal_sem_t* s_done = NULL;
static uint64_t s_last_poll_tick = 0;

///< 挂载点前缀表：ATCMountService 挂 /media/(udisk*/ext_sdcard*)，兼容 /mnt/ 等 
static const char* s_mount_prefixes[] = { MW_MSG_MOUNT_PREFIXES, NULL };

/** mnt 是否命中任一关注前缀 */
static int match_mount_prefix(const char* mnt) {
    for (int i = 0; s_mount_prefixes[i] != NULL; i++) {
        size_t len = strlen(s_mount_prefixes[i]);
        if (strncmp(mnt, s_mount_prefixes[i], len) == 0) return 1;
    }
    return 0;
}

static msg_manage_media_type_e classify_media(const char* device) {
    if (strncmp(device, "/dev/mmcblk", 11) == 0) return MW_MSG_MEDIA_SD;
    if (strncmp(device, "/dev/sd", 7) == 0) return MW_MSG_MEDIA_USB;
    if (strncmp(device, "/dev/sr", 7) == 0) return MW_MSG_MEDIA_USB;
    return MW_MSG_MEDIA_UNKNOWN;
}

/** 锁内快照订阅者、锁外逐个调用(send_key_event 同款) */
static void send_event(msg_manage_media_event_e event, const mount_entry_t* e) {
    msg_manage_cb_t snap[MW_MAX_CALLBACK_NUM];
    uint8_t n;
    {
        MW_MUTEX_GUARD(&s_cb_lock);
        memcpy(snap, s_cbs, sizeof(snap));
        n = s_cb_count;
    }
    if (n == 0) return;

    msg_manage_media_info_t info;
    memset(&info, 0, sizeof(info));
    strncpy(info.device, e->device, MSG_DEV_LEN - 1);
    strncpy(info.mount_point, e->mount_point, MSG_MNT_LEN - 1);
    strncpy(info.fs_type, e->fs_type, MSG_FS_LEN - 1);
    info.media_type = classify_media(e->device);
    for (uint8_t i = 0; i < n; i++) {
        if (snap[i]) snap[i](event, &info);
    }
}

static int find_entry(const mount_entry_t* arr, int cap, const char* device) {
    for (int i = 0; i < cap; i++) {
        if (arr[i].used && strcmp(arr[i].device, device) == 0) return i;
    }
    return -1;
}

/** 读 /proc/mounts，过滤命中挂载前缀表(MW_MSG_MOUNT_PREFIXES)的条目填入 out，返回条目数(<=cap) */
static int read_mounts(mount_entry_t* out, int cap) {
    FILE* fp = fopen("/proc/mounts", "r");
    if (!fp) {
        mw_log_error("[msg_mgr] open /proc/mounts fail: %s", strerror(errno));
        return -1;
    }
    char line[512];
    int n = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (n >= cap) {
            mw_log_warn("[msg_mgr] mounts >= cap %d, truncated", cap);
            break;
        }
        char dev[MSG_DEV_LEN], mnt[MSG_MNT_LEN], fs[MSG_FS_LEN];
        if (sscanf(line, "%63s %127s %15s", dev, mnt, fs) != 3) continue;
        if (!match_mount_prefix(mnt)) continue;  /* 只跟关注前缀 */
        mount_entry_t e;
        memset(&e, 0, sizeof(e));
        strncpy(e.device, dev, MSG_DEV_LEN - 1);
        strncpy(e.mount_point, mnt, MSG_MNT_LEN - 1);
        strncpy(e.fs_type, fs, MSG_FS_LEN - 1);
        e.used = 1;
        out[n++] = e;
    }
    fclose(fp);
    return n;
}

static void* poll_thread(void* arg) {
    (void)arg;
    mw_log_info("[msg_mgr] poll thread started, period=%ums\n", MW_MSG_POLL_MS);
    int first_cycle = 1; /* 首帧只建基线不发事件(基线用 get 查) */
    while (s_running) {
        mount_entry_t now[MW_MSG_MAX_DEVS];
        memset(now, 0, sizeof(now));
        int now_n = read_mounts(now, MW_MSG_MAX_DEVS);
        if (now_n < 0) now_n = 0;

        /* diff：事件先入本地表，分发移到锁外 */
        msg_manage_media_event_e ev_tab[MW_MSG_MAX_DEVS];
        mount_entry_t ev_entry[MW_MSG_MAX_DEVS];
        int ev_n = 0;

        {
            MW_MUTEX_GUARD(&s_lock);
            /* 新增：now 中有、s_cur 中无 */
            for (int i = 0; i < now_n && ev_n < MW_MSG_MAX_DEVS; i++) {
                if (find_entry(s_cur, MW_MSG_MAX_DEVS, now[i].device) < 0) {
                    ev_tab[ev_n] = MW_MSG_MEDIA_MOUNTED;
                    ev_entry[ev_n] = now[i];
                    ev_n++;
                }
            }
            /* 消失：s_cur 中有、now 中无 */
            for (int i = 0; i < MW_MSG_MAX_DEVS && ev_n < MW_MSG_MAX_DEVS; i++) {
                if (!s_cur[i].used) continue;
                if (find_entry(now, MW_MSG_MAX_DEVS, s_cur[i].device) < 0) {
                    ev_tab[ev_n] = MW_MSG_MEDIA_UNMOUNTED;
                    ev_entry[ev_n] = s_cur[i];
                    ev_n++;
                }
            }
            /* 更新快照 */
            memset(s_cur, 0, sizeof(s_cur));
            for (int i = 0; i < now_n; i++) s_cur[i] = now[i];
            s_last_poll_tick = osal_tick_ms();
        } /* 锁释放 */

        if (first_cycle) {
            first_cycle = 0; /* 首帧仅建 s_cur 基线，事件全部抑制 */
        } else {
            for (int i = 0; i < ev_n; i++) {
                send_event(ev_tab[i], &ev_entry[i]);
            }
        }

        osal_delay_ms(MW_MSG_POLL_MS);
    }
    if (s_done) osal_sem_post(s_done);
    mw_log_info("[msg_mgr] poll thread exit\n");
    return NULL;
}

/** self_check：worker 存活且末次 poll 距今 < 3 倍周期 -> 0，否则 -1 */
static int msg_manage_selfcheck(void) {
    if (!s_running) return -1;
    uint64_t now = osal_tick_ms();
    return (now - s_last_poll_tick) < (3ull * MW_MSG_POLL_MS) ? 0 : -1;
}

int msg_manage_init(void) {
    if (s_running) return 0;
    memset(s_cur, 0, sizeof(s_cur));
    memset(s_cbs, 0, sizeof(s_cbs));
    s_cb_count = 0;
    s_last_poll_tick = osal_tick_ms();

    s_done = osal_sem_create(0, 1);
    if (!s_done) {
        mw_log_error("[msg_mgr] sem create fail\n");
        return -1;
    }
    s_running = 1;
    if (mw_pthread_create("msg_mgr", NULL, poll_thread) != 0) {
        s_running = 0;
        osal_sem_delete(s_done);
        s_done = NULL;
        mw_log_error("[msg_mgr] thread create fail\n");
        return -1;
    }
    mw_module_register_check(MW_MOD_MSG_MANAGE, msg_manage_selfcheck);
    mw_log_info("[msg_mgr] init OK\n");
    return 0;
}

int msg_manage_deinit(void) {
    if (!s_running) return 0;
    s_running = 0;
    if (s_done) {
        osal_sem_timedwait(s_done, MW_MSG_POLL_MS * 4);
        osal_sem_delete(s_done);
        s_done = NULL;
    }
    return 0;
}

int msg_manage_set_cb(msg_manage_cb_t cb) {
    if (!cb) return -1;
    MW_MUTEX_GUARD(&s_cb_lock);
    if (s_cb_count >= MW_MAX_CALLBACK_NUM) {
        mw_log_error("[msg_mgr] cb count exceed max %d", MW_MAX_CALLBACK_NUM);
        return 1;
    }
    for (uint8_t i = 0; i < s_cb_count; i++) {
        if (s_cbs[i] == cb) return 1;  /* 已注册 */
    }
    s_cbs[s_cb_count++] = cb;
    return 0;
}

int msg_manage_remove_cb(msg_manage_cb_t cb) {
    MW_MUTEX_GUARD(&s_cb_lock);
    for (uint8_t i = 0; i < s_cb_count; i++) {
        if (s_cbs[i] == cb) {
            for (uint8_t j = i; j + 1 < s_cb_count; j++) {
                s_cbs[j] = s_cbs[j + 1];
            }
            s_cbs[--s_cb_count] = NULL;
            return 0;
        }
    }
    return -1;
}

int msg_manage_get_mounts(msg_manage_media_info_t* out, int cap) {
    if (!out || cap <= 0) return -1;
    int n = 0;
    MW_MUTEX_GUARD(&s_lock);
    for (int i = 0; i < MW_MSG_MAX_DEVS && n < cap; i++) {
        if (!s_cur[i].used) continue;
        msg_manage_media_info_t* e = &out[n++];
        memset(e, 0, sizeof(*e));
        strncpy(e->device, s_cur[i].device, MSG_DEV_LEN - 1);
        strncpy(e->mount_point, s_cur[i].mount_point, MSG_MNT_LEN - 1);
        strncpy(e->fs_type, s_cur[i].fs_type, MSG_FS_LEN - 1);
        e->media_type = classify_media(s_cur[i].device);
    }
    return n;
}

#endif /* MW_MSG_MANAGE_ENABLE */
