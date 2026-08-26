/**
 * @file mw_display_mode.c
 * @brief 显示模式/自动背光/自动大灯 -- OSAL 线程 + 纯业务状态机
 *
 * 算法与旧 FreeRTOS 版一致（6 级阈值表/5 点采样/日夜间判决/自动背光/
 * 自动大灯），平台差异全部收敛：光感原始值走 hal_adc（经 light_sensor
 * 取值器），线程/时钟/日志走 osal。停机语义同 adc_key：停标志 + done
 * 信号量优雅退出（OSAL 线程为 detached，不可 delete）。
 *
 * 已知历史遗留（按原样迁移保行为，单独立项修）：
 *   - check_sensor_level 判定条件 success_count == ARR_LEVEL-1(=4) 与
 *     采样深度 ARR_SIZE(=5) 不一致；
 *   - lg_value 为"填满即清零"缓冲而非真正滑窗。
 */
#include <stdbool.h>

#include "mw_display_mode.h"
#include "mw_light_sensor.h"
#include "set_param.h"
#include "vehicle_param.h"
#include "dev_state.h"
#include "mw_log.h"
#include "mw_modules.h"
#include "osal.h"
#if MW_BL_PWM_ENABLE
#include "usr_backlight.h"
#endif

#if MW_LIGHT_SENSOR_ENABLE

#define DISPLAY_MODE_PERIOD (50)           ///< 判定线程周期 ms
#define LIGHT_SENSOR_GET_DATA_INTERVAL (3) ///< 每 3 个周期采一点(150ms)

typedef struct {
    int arr_index;
    int sample_interval;
    uint8_t is_first_display;
    uint8_t sensor_level;
    uint8_t cur_display_mode;
} display_param_t;

static display_param_t display = {0, 0, true, 0, 0xFF};

static uint16_t lg_ref[LIGHT_SENSOR_ARR_LEVEL];
static uint16_t lg_value[LIGHT_SENSOR_ARR_SIZE];

static volatile bool s_running = false; ///< 判定线程运行标志
static osal_sem_t* s_done = NULL;       ///< 线程退出信号（sleep 同步用）

static int display_mode_self_check(void) { return s_running ? 0 : -1; }

static void light_sensor_ref_init(void) {
    lg_ref[0] = 4060;
    lg_ref[1] = 2400;
    lg_ref[2] = 1500;
    lg_ref[3] = 700;
    lg_ref[4] = 100;
    lg_ref[5] = 0;
}

#if MW_BL_PWM_ENABLE
static void check_auto_backlight_level(void) {
    static uint8_t cur_level = 0;

    if (!get_recovery_usr_param() ||
        get_check_self_state() < CHECK_SELF_STATE_SUCCESS) {
        return;
    }

    if (!get_usr_param(USR_PARAM_BRIGHTNESS_LEVEL, &cur_level)) {
        mw_log_error("Get usr param backlight failed!\n");
        return;
    }

    if (cur_level == BACKLIGHT_LEVEL_AUTO) {
        if (display.sensor_level > 1 && cur_level != BACKLIGHT_LEVEL_4) {
            set_backlight_level(BACKLIGHT_LEVEL_4);
        } else if (display.sensor_level <= 1 &&
                   cur_level != BACKLIGHT_LEVEL_1) {
            set_backlight_level(BACKLIGHT_LEVEL_1);
        }
    }
}
#endif /* MW_BL_PWM_ENABLE */

static void check_auto_headlight(void) {
    static uint8_t last_headlight = 0;
    if (display.sensor_level <= 1 && last_headlight != 1) {
        vehicle_set_data(VEH_AUTO_HEADLIGH, 1);
        last_headlight = 1;
    } else if (display.sensor_level > 1 && last_headlight != 2) {
        vehicle_set_data(VEH_AUTO_HEADLIGH, 0);
        last_headlight = 2;
    }
}

static void check_display_mode(void) {
    uint8_t display_mode = 0;

    if (!get_recovery_usr_param()) {
        return;
    }

    if (!get_usr_param(USR_PARAM_THEME, &display_mode)) {
        mw_log_error("Get usr param display mode failed!\n");
        return;
    }

    if (display_mode == AUTO_MODE) {
        int target_display = display.sensor_level > 1 ? DAY_MODE : NIGHT_MODE;
        if (display.cur_display_mode != target_display) {
            display.cur_display_mode = target_display;
            vehicle_set_data(VEH_CUR_DISPALY_MODE, display.cur_display_mode);
            mw_log_info("Current display mode: %s %d\n",
                        display.cur_display_mode == DAY_MODE ? "Day" : "Night",
                        display.cur_display_mode);
        }
    } else {
        if (display.cur_display_mode != display_mode) {
            display.cur_display_mode = display_mode;
            vehicle_set_data(VEH_CUR_DISPALY_MODE, display.cur_display_mode);
        }
    }
}

/**
 * @brief  光感分级：ACC 首次上电快判 + 阈值表连续命中判级，判级后联动三路 
 */
static void check_sensor_level(void) {
    if (is_acc_start() && display.is_first_display) {
        display.is_first_display = false;
        if (lg_value[0] <= lg_ref[1]) {
            display.sensor_level = 2;
        } else {
            display.sensor_level = 1;
        }
    } else {
        for (int i = 0; i < LIGHT_SENSOR_ARR_LEVEL - 1; i++) {
            int success_count = 0;

            for (int j = 0; j < LIGHT_SENSOR_ARR_SIZE; j++) {
                if (lg_value[j] > lg_ref[i + 1] && lg_value[j] <= lg_ref[i]) {
                    success_count++;
                } else {
                    break;
                }

                if (success_count == LIGHT_SENSOR_ARR_LEVEL - 1) {
                    display.sensor_level = i + 1;
                }
            }
        }
    }

    check_display_mode();
#if MW_BL_PWM_ENABLE
    check_auto_backlight_level();
#endif
    check_auto_headlight();
}

/**
 * @brief  周期采样入口：每 3 个周期读一次光感原始值入环，随即判级联动 
 */
static void check_light_sensor(void) {
    if ((display.sample_interval % LIGHT_SENSOR_GET_DATA_INTERVAL) == 0) {
        int raw = light_sensor_read();
        if (raw >= 0) {
            if (display.arr_index < LIGHT_SENSOR_ARR_SIZE) {
                lg_value[display.arr_index] = (uint16_t)raw;
                display.arr_index++;
            }

            if (display.arr_index >= LIGHT_SENSOR_ARR_SIZE) {
                display.arr_index = 0;
            }
        }
        check_sensor_level();
    }
    display.sample_interval++;
}

static void* display_mode_thread(void* arg) {
    (void)arg;
    light_sensor_ref_init();

    while (s_running) {
        check_light_sensor();
        osal_delay_ms(DISPLAY_MODE_PERIOD);
    }
    if (s_done) osal_sem_post(s_done);
    return NULL;
}

int display_mode_init(void) {
    if (s_running) return 0; /* 幂等 */

    (void)light_sensor_init();
    display.arr_index = 0;
    display.sample_interval = 0;
    display.is_first_display = true;
    display.sensor_level = 0;
    display.cur_display_mode = 0xFF;

    s_done = osal_sem_create(0, 1);
    if (!s_done) {
        mw_log_error("create done sem failed\n");
        return -1;
    }

    s_running = true;
    if (osal_thread_create(NULL, "display_mode", display_mode_thread, NULL, 0,
                           0) != OSAL_OK) {
        s_running = false;
        osal_sem_delete(s_done);
        s_done = NULL;
        mw_log_error("create display mode thread failed\n");
        return -1;
    }
    mw_module_register_check(MW_MOD_DISPLAY_MODE, display_mode_self_check);
    mw_log_info("display mode init success\n");
    return 0;
}

/**
 * @brief  休眠前清理: 置停标志 -> 等 done 信号量（超时 2 个周期兜底）
 *      复位首判标志（唤醒重拉起后重新快判）
 */
void display_mode_sleep(void) {
    if (!s_running) return;

    s_running = false;
    if (s_done) {
        (void)osal_sem_timedwait(s_done, 2 * DISPLAY_MODE_PERIOD + 5);
        osal_sem_delete(s_done);
        s_done = NULL;
    }

    display.is_first_display = true;
    mw_log_info("display mode sleep: judge thread stopped\n");
}

#endif
