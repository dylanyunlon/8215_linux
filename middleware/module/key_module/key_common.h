#ifndef __KEY_COMMON_H__
#define __KEY_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "dev_config.h"

/**
 * @brief 按键事件枚举
 */
typedef enum {
    NO_KEY_EVENT = 0x0,
    MODE_KEY_SHORT_PR = 0x01,
    MODE_KEY_LONG_PR = 0x02,
    SET_KEY_SHORT_PR = 0x03,
    SET_KEY_LONG_PR = 0x04,
    COM_KEY_SHORT_PR = 0x05,
    COM_KEY_LONG_PR = 0x06,
    UP_KEY_SHORT_PR = 0x07,
    UP_KEY_LONG_PR = 0x08,
    DOWN_KEY_SHORT_PR = 0x09,
    DOWN_KEY_LONG_PR = 0x0A,
    ENTER_KEY_SHORT_PR = 0x10,
    ENTER_KEY_LONG_PR = 0x11,
    BACK_KEY_SHORT_PR = 0x12,
    BACK_KEY_LONG_PR = 0x13,
    COM_KEY_SHORT_PR1 = 0x14,
    COM_KEY_LONG_PR1 = 0x15,
    SET_KEY_SUPER_LONG_PR = 0x16,
    UP_KEY_SUPER_LONG_PR = 0x17,
} key_event_e;

typedef uint8_t key_event_t_;
typedef void (*key_event_cb_t)(key_event_t_);

/**
 * @brief  注册按键事件回调（可注册多个，去重）
 * @param  event_cb 按键事件回调函数
 * @return 0:成功  1:已注册过或已达上限(MW_MAX_CALLBACK_NUM)  -1:参数错误
 */
int set_key_event_cb(key_event_cb_t event_cb);

/**
 * @brief  分发按键事件到所有已注册回调（锁内快照、锁外调用，
 *         回调内可安全调用 set/remove_key_event_cb）
 * @param  key_event 按键事件值
 * @return none
 */
void send_key_event(uint8_t key_event);

/**
 * @brief  移除按键事件回调（前移收缩数组，容量可复用）
 * @param  event_cb 已注册的回调函数
 * @return 0:成功  -1:未找到
 */
int remove_key_event_cb(key_event_cb_t event_cb);

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  // __KEY_COMMON_H__
