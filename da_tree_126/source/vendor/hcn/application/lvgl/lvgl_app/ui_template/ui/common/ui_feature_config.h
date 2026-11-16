#ifndef UI_FEATURE_CONFIG_H
#define UI_FEATURE_CONFIG_H

#ifdef __cplusplus
#include "vehicle_services.h"      /* ui 与中间件之间唯一的边界 */
extern "C" {
#endif

/*
 * hcn_application 功能开关（UI 层自有配置，不依赖板级 dev_config.h）
 * ---------------------------------------------------------------------------
 * UI 功能开关收归 UI 自己的命名空间（UI_*），不依赖板级 dev_config.h。
 *   - 板级/PC 两端 dev_config.h 的宏名与默认值并不一致（如背光 MW_BL_PWM_ENABLE
 *     与 MW_BL_PW_ENABLE，且缺 MW_CARLINK_EC_ENABLE / NFC_ENABLE / OTA_*）；
 *   - 未定义宏出现在 #if 里会被预处理器静默当成 0，整块 UI 功能会无声消失；
 *   - 中间件的头不定义 UI_ 前缀，因此 UI 开关不会被板级配置劫持。
 *
 * 默认值按 PC/设备共用行为设定。
 * device 后端按板子需要覆盖即可：cmake ... -DCMAKE_C_FLAGS="-DUI_NFC_ENABLE=0"
 */

/* ---------- 物理按键导航（ADC 电压梯 / GPIO 独立按键）----------
 * 1=UI 处理硬件按键事件（logic 注册 vs_key_cb_set，button_manager 分发）
 * 0=UI 不注册按键回调（纯触摸或无按键板型） */
#ifndef UI_BTN_MANAGER_ENABLE
#define UI_BTN_MANAGER_ENABLE 1
#endif

/* ---------- 背光 PWM 亮度可调 ----------
 * 1=设置页亮度项走 vs_backlight_set()；0=只读灰显 */
#ifndef UI_BACKLIGHT_PWM_ENABLE
#define UI_BACKLIGHT_PWM_ENABLE 1
#endif

/* ---------- 手机互联 EasyConn（投屏）---------- */
#ifndef UI_CARLINK_EC_ENABLE
#define UI_CARLINK_EC_ENABLE 1
#endif

#ifndef UI_CARLINK_EC_UUID
#define UI_CARLINK_EC_UUID "HNCX04XXXXXXHCN012"
#endif

/* ---------- 简易导航（BLE 路口提示）----------
 * 与手机互联互斥：两者抢同一路 UART/BLE，默认关 */
#ifndef UI_EASY_NAVI_ENABLE
#define UI_EASY_NAVI_ENABLE 0
#endif

#if UI_CARLINK_EC_ENABLE && UI_EASY_NAVI_ENABLE
#error "UI_CARLINK_EC_ENABLE 与 UI_EASY_NAVI_ENABLE 互斥（同一路 UART/BLE），不能同时为 1"
#endif

/* ---------- NFC 刷卡 ---------- */
#ifndef UI_NFC_ENABLE
#define UI_NFC_ENABLE 1
#endif

/* ---------- OTA 升级 ----------
 * UI 只负责触发入口与展示，长按上键等待 UI_OTA_KEY_WAIT_TIMEOUT_MS 毫秒进入 */
#ifndef UI_OTA_KEY_WAIT_TIMEOUT_MS
#define UI_OTA_KEY_WAIT_TIMEOUT_MS 30000
#endif

#ifndef UI_OTA_DEFAULT_SSID
#define UI_OTA_DEFAULT_SSID "ota630HCN"
#endif

#ifndef UI_OTA_DEFAULT_PASSWD
#define UI_OTA_DEFAULT_PASSWD "66666666"
#endif

#ifdef __cplusplus
}
#endif

#endif /* UI_FEATURE_CONFIG_H */
