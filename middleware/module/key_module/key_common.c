#include <string.h>
#include "key_common.h"
#include "callback_config.h"
#include "mw_log.h"
#include "mw_lock.h"

static key_event_cb_t key_event_cb[MW_MAX_CALLBACK_NUM] = {NULL};
static uint8_t s_key_cb_count = 0;

static osal_mutex_t s_key_cb_mutex = OSAL_MUTEX_INIT;

int set_key_event_cb(key_event_cb_t event_cb) {
    MW_MUTEX_GUARD(&s_key_cb_mutex);
    if (!event_cb) {
        return -1; /* 参数错误 */
    }
    if (s_key_cb_count >= MW_MAX_CALLBACK_NUM) {
        mw_log_error("key_event_cb count exceed max %d\n", MW_MAX_CALLBACK_NUM);
        return 1;
    }

    ///< 检测是否已经注册过相同的回调函数
    for (uint8_t i = 0; i < s_key_cb_count; i++) {
        if (key_event_cb[i] == event_cb) {
            return 1; /* 已注册过 */
        }
    }

    key_event_cb[s_key_cb_count] = event_cb;
    s_key_cb_count++;
    return 0;
}

void send_key_event(uint8_t key_event) {
    ///< 锁内快照整表，锁外逐个调用（见文件头并发设计说明）
    key_event_cb_t snap[MW_MAX_CALLBACK_NUM];
    uint8_t n;
    {
        MW_MUTEX_GUARD(&s_key_cb_mutex);
        memcpy(snap, key_event_cb, sizeof(snap));
        n = s_key_cb_count;
    }

    for (uint8_t i = 0; i < n; i++) {
        if (snap[i]) {
            snap[i](key_event);
        }
    }
}

int remove_key_event_cb(key_event_cb_t event_cb) {
    MW_MUTEX_GUARD(&s_key_cb_mutex);
    for (uint8_t i = 0; i < s_key_cb_count; i++) {
        if (key_event_cb[i] == event_cb) {
            ///< 前移收缩：不留空洞，计数同步递减，容量可复用
            for (uint8_t j = i; j < s_key_cb_count - 1; j++) {
                key_event_cb[j] = key_event_cb[j + 1];
            }
            key_event_cb[--s_key_cb_count] = NULL;
            mw_log_info("key_event_cb removed\n");
            return 0;
        }
    }
    mw_log_info("key_event_cb not found\n");
    return -1;
}
