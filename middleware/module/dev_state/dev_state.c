#include "dev_state.h"
#include <string.h>
#include "callback_config.h"
#include "mw_log.h"
#include "mw_lock.h"

static check_self_state_e current_state = CHECK_SELF_STATE_INIT;
static animation_state_e animation = ANIMATION_STATE_IDLE;
static set_param_state_e param_state = SET_PARAM_STATE_FAILED;
static power_state_e power_state = POWER_STATE_OFF;
static com_port_state_e com_port_state = COM_PORT_STATE_NONE;

static osal_mutex_t s_dev_state_mutex = OSAL_MUTEX_INIT;

static dev_state_cb_t dev_state_cb[MW_MAX_CALLBACK_NUM] = {NULL};
static uint8_t s_dev_state_cb_count = 0;

static osal_mutex_t s_dev_state_cb_mutex = OSAL_MUTEX_INIT;

int set_dev_state_cb(dev_state_cb_t event_cb) {
    MW_MUTEX_GUARD(&s_dev_state_cb_mutex);
    if (!event_cb) {
        return -1; /* 参数错误 */
    }
    if (s_dev_state_cb_count >= MW_MAX_CALLBACK_NUM) {
        mw_log_error("dev_state_cb count exceed max %d\n",
                     MW_MAX_CALLBACK_NUM);
        return 1;
    }

    ///< 检测是否已经注册过相同的回调函数
    for (uint8_t i = 0; i < s_dev_state_cb_count; i++) {
        if (dev_state_cb[i] == event_cb) {
            return 1; /* 已注册过 */
        }
    }

    dev_state_cb[s_dev_state_cb_count] = event_cb;
    s_dev_state_cb_count++;
    return 0;
}

void send_dev_state_event(dev_state_type_e type, uint32_t value) {
    ///< 锁内快照整表，锁外逐个调用（见文件头并发设计说明）
    dev_state_cb_t snap[MW_MAX_CALLBACK_NUM];
    uint8_t n;
    {
        MW_MUTEX_GUARD(&s_dev_state_cb_mutex);
        memcpy(snap, dev_state_cb, sizeof(snap));
        n = s_dev_state_cb_count;
    }

    for (uint8_t i = 0; i < n; i++) {
        if (snap[i]) {
            snap[i](type, value);
        }
    }
}

int remove_dev_state_cb(dev_state_cb_t event_cb) {
    MW_MUTEX_GUARD(&s_dev_state_cb_mutex);
    for (uint8_t i = 0; i < s_dev_state_cb_count; i++) {
        if (dev_state_cb[i] == event_cb) {
            ///< 前移收缩：不留空洞，计数同步递减，容量可复用
            for (uint8_t j = i; j < s_dev_state_cb_count - 1; j++) {
                dev_state_cb[j] = dev_state_cb[j + 1];
            }
            dev_state_cb[--s_dev_state_cb_count] = NULL;
            mw_log_info("dev_state_cb removed\n");
            return 0;
        }
    }
    mw_log_info("dev_state_cb not found\n");
    return -1;
}

void set_check_self_state(check_self_state_e state) {
    int changed = 0;
    {
        MW_MUTEX_GUARD(&s_dev_state_mutex);
        if (current_state != state) {
            current_state = state;
            changed = 1;
        }
    }
    if (changed) {
        send_dev_state_event(DEV_STATE_CHECK_SELF, (uint32_t)state);
    }
}

check_self_state_e get_check_self_state(void) {
    MW_MUTEX_GUARD(&s_dev_state_mutex);
    return current_state;
}

animation_state_e get_boot_animation_status(void) {
    MW_MUTEX_GUARD(&s_dev_state_mutex);
    return animation;
}

void set_boot_animation_status(animation_state_e state) {
    int changed = 0;
    {
        MW_MUTEX_GUARD(&s_dev_state_mutex);
        if (animation != state) {
            animation = state;
            changed = 1;
        }
    }
    if (changed) {
        send_dev_state_event(DEV_STATE_BOOT_ANIM, (uint32_t)state);
    }
}

void set_param_state(set_param_state_e state) {
    int changed = 0;
    {
        MW_MUTEX_GUARD(&s_dev_state_mutex);
        if (param_state != state) {
            param_state = state;
            changed = 1;
        }
    }
    if (changed) {
        send_dev_state_event(DEV_STATE_SET_PARAM, (uint32_t)state);
    }
}

set_param_state_e get_param_state(void) {
    MW_MUTEX_GUARD(&s_dev_state_mutex);
    return param_state;
}
void set_power_state(power_state_e state) {
    int changed = 0;
    {
        MW_MUTEX_GUARD(&s_dev_state_mutex);
        if (power_state != state) {
            power_state = state;
            changed = 1;
        }
    }
    if (changed) {
        send_dev_state_event(DEV_STATE_POWER, (uint32_t)state);
    }
}

power_state_e get_power_state(void) {
    MW_MUTEX_GUARD(&s_dev_state_mutex);
    return power_state;
}
void set_com_port_state(com_port_state_e state) {
    int changed = 0;
    {
        MW_MUTEX_GUARD(&s_dev_state_mutex);
        if (com_port_state != state) {
            com_port_state = state;
            changed = 1;
        }
    }
    if (changed) {
        send_dev_state_event(DEV_STATE_COM_PORT, (uint32_t)state);
    }
}

com_port_state_e get_com_port_state(void) {
    MW_MUTEX_GUARD(&s_dev_state_mutex);
    return com_port_state;
}

