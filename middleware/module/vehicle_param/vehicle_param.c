#include <stddef.h>
#include <stdbool.h>
#include "mw_log.h"
#include "mw_lock.h"
#include "vehicle_param.h"

#define VEH_INVALID_DATA INT32_MIN

static int32_t veh_data[VEH_DATA_END] = {0};

/** veh_data 由多线程并发读写（UI 读取 / 通信写入），需互斥保护 */
static osal_mutex_t s_veh_mutex = OSAL_MUTEX_INIT;

/** 变更回调（单订阅者；受 s_veh_mutex 保护，锁内快照、锁外调用） */
static vehicle_change_cb_t s_change_cb = NULL;

void vehicle_param_set_change_cb(vehicle_change_cb_t cb) {
    MW_MUTEX_GUARD(&s_veh_mutex);
    s_change_cb = cb;
}

static bool is_valid_id(veh_data_e id) {
    /* 拒绝负数 id：enum 是否无符号由编译器决定，防负下标越界 */
    return ((int)id >= 0 && id < VEH_DATA_END);
}

int32_t vehicle_get_data(veh_data_e id) {
    MW_MUTEX_GUARD(&s_veh_mutex);
    if (is_valid_id(id)) {
        return veh_data[id];
    } else {
        mw_log_info("Attempt to retrieve data for invalid ID: %u", id);
        return VEH_INVALID_DATA;
    }
}

void vehicle_set_data(veh_data_e id, int32_t value) {
    vehicle_change_cb_t cb = NULL;
    {
        MW_MUTEX_GUARD(&s_veh_mutex);
        if (!is_valid_id(id)) {
            mw_log_info("Attempt to set data for invalid ID: %u", id);
            return;
        }

        if (veh_data[id] != value) {
            veh_data[id] = value;
            cb = s_change_cb; /* 锁内快照，回调内可安全再调 get/set */
        }
    }

    if (cb) {
        cb(id, value); /* 锁外调用（同 dev_state 分发模式）*/
    }
}
