#include <stdbool.h>
#include "dev_state.h"
#include "vehicle_param.h"
#include "osal.h"
#include "hal_gpio.h"
#include "gpio_light.h"
#include "dev_config.h"

#ifndef NFC_ENABLE
#define NFC_ENABLE 0
#endif
#ifndef MW_CARLINK_EC_ENABLE
#define MW_CARLINK_EC_ENABLE 0
#endif

#if GPIO_LIGHT_ENABLE

///< io 刷新周期
#define VEH_IO_CHECK_PERIOD (100)

///< gpio 探测脚
#define VEH_LEFT_TURN_DET_GPIO ("PD.7")
#define VEH_RIGHT_TURN_DET_GPIO ("PD.6")
#define VEH_HEAD_LIGHT_DET_GPIO ("PD.5")
#if !NFC_ENABLE
#define VEH_DAY_RUN_LIGHT_GPIO ("PD.4")
#endif
#ifndef MW_CARLINK_EC_ENABLE
#define VEH_GB_T_DET_GPIO ("PB.11")
#endif

typedef struct {
    const char* pin_name;
    gpio_t pin;  ///< 后端引脚号；GPIO_INVALID=未申请到
} gpio_ctrl_t;

#ifdef GPIO_FILTER_ENABLE
///< gpio 过滤时间值
#define GPIO_FILTER_TIME (300)
///< gpio 过滤计数值
#define GPIO_FILTER_CNT(period) (GPIO_FILTER_TIME / period)
static uint8_t gpio_last_value[6] = {0};
static uint8_t gpio_count[6] = {0};
#endif

static gpio_ctrl_t pin_ctrl[5] = {
    [0] = {VEH_LEFT_TURN_DET_GPIO, GPIO_INVALID},
    [1] = {VEH_RIGHT_TURN_DET_GPIO, GPIO_INVALID},
    [2] = {VEH_HEAD_LIGHT_DET_GPIO, GPIO_INVALID},
#if !NFC_ENABLE
    [3] = {VEH_DAY_RUN_LIGHT_GPIO, GPIO_INVALID},
#endif
#ifndef MW_CARLINK_EC_ENABLE
    [4] = {VEH_GB_T_DET_GPIO, GPIO_INVALID},
#endif
};

static volatile bool g_gpio_light_stop = false;

static void set_frame_light(gpio_t pin, int value) {
    if (pin == pin_ctrl[0].pin) {
        vehicle_set_data(VEH_INDICATOR_TURN_LEFT, !value);
    } else if (pin == pin_ctrl[1].pin) {
        vehicle_set_data(VEH_INDICATOR_TURN_RIGHT, !value);
    } else if (pin == pin_ctrl[2].pin) {
        vehicle_set_data(VEH_LIGHT_HIGH_BEAM, !value);
    }
#if !NFC_ENABLE
    else if (pin == pin_ctrl[3].pin) {
        vehicle_set_data(VEH_AUTO_HEADLIGH, !value);
    }
#endif
#ifndef MW_CARLINK_EC_ENABLE
    else if (pin == pin_ctrl[4].pin) {
        vehicle_set_data(VEH_LIGHT_GB, !value);
    }
#endif
}

/**
 * @brief  检测外框灯
 * @param  pin 后端引脚号
 * @param  io_index 过滤 io 序号
 * @param  is_filter 是否需要进行 io 过滤
 * @return none
 */
static void check_frame_light(gpio_t pin, int io_index, bool is_filter) {
    /* gpio_read 直接返回 0/1，<0 表示出错 */
    int value = gpio_read(pin);
    if (value < 0) return;

    if (is_filter) {
        if (gpio_last_value[io_index] == value) {
            if (gpio_count[io_index] >= GPIO_FILTER_CNT(VEH_IO_CHECK_PERIOD)) {
                if (get_check_self_state() == CHECK_SELF_STATE_SUCCESS) {
                    set_frame_light(pin, value);
                }
            } else {
                gpio_count[io_index] += 1;
            }
        } else {
            gpio_last_value[io_index] = value;
            gpio_count[io_index] = 1;
        }
    } else {
        if (get_check_self_state() != CHECK_SELF_STATE_SUCCESS) {
            return;
        }
        set_frame_light(pin, value);
    }
}

static void scan_frame_light(void) {
    for (uint8_t i = 0; i < 5; i++) {
#if NFC_ENABLE
        if (i == 3) continue;
#endif
#ifdef MW_CARLINK_EC_ENABLE
        if (i == 4) continue;
#endif
        if (pin_ctrl[i].pin == GPIO_INVALID) continue; /* 未申请到的引脚跳过 */
        check_frame_light(pin_ctrl[i].pin, i, false);
    }
}

static void set_light_gpio_input(void) {
    for (uint8_t i = 0; i < 5; i++) {
#if NFC_ENABLE
        if (i == 3) continue;
#endif
#ifdef MW_CARLINK_EC_ENABLE
        if (i == 4) continue;
#endif
        /* 极简 HAL：gpio_get 一次性解析逻辑名 -> 后端引脚号 */
        pin_ctrl[i].pin = gpio_get(pin_ctrl[i].pin_name);
        if (pin_ctrl[i].pin == GPIO_INVALID) {
            osal_log_warn("light_gpio: request pin failed\n");
            continue;
        }
        gpio_set_dir(pin_ctrl[i].pin, GPIO_DIR_INPUT);
    }
}

static void* gpio_light_thread(void* arg) {
    (void)arg;
    set_light_gpio_input();

    while (!g_gpio_light_stop) {
        osal_delay_ms(VEH_IO_CHECK_PERIOD);
        if (!g_gpio_light_stop) scan_frame_light();
    }

    osal_log_info("light_gpio: poll thread exit\n");
    return NULL;
}

int light_gpio_init(void) {
    g_gpio_light_stop = false;
    if (osal_thread_create(NULL, "gpio_light", gpio_light_thread, NULL, 2048,
                           12) != OSAL_OK) {
        osal_log_error("light_gpio: create thread failed!\n");
        return -1;
    }
    osal_log_info("light_gpio: init success!\n");
    return 0;
}

void light_gpio_sleep(void) {
    g_gpio_light_stop = true;
    osal_log_info("light_gpio: sleep (stop flag set)\n");
}

#endif
