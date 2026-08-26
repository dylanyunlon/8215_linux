/**
 * @file adc_key.c
 * @brief ADC/GPIO 按键扫描 -- 纯 OSAL+HAL，三系统同源
 *
 * 算法与原 RT-Thread 版一致：
 *   10ms 采样 -> 5 点中值滤波 -> 电压窗口匹配 ->
 *   短按/长按(2.5s)/超长按(8s, 仅上键) 状态机 -> key_common 分发事件。
 * 平台差异全部收敛：ADC 原始值走 hal_key.h（hal/key/key_<os>.c 后端），
 * 线程/时钟/日志走 osal.h；GPIO 按键模式（MW_KEY_USE_GPIO=1）复用
 * hal_gpio.h。自检门槛（dev_state）保持不变。
 * adc_key_sleep 由"删线程"改为"停标志 + done 信号量"优雅退出
 * （OSAL 线程为 detached，不可 delete）。
 */

#include <stdlib.h>
#include <string.h>

#include "adc_key.h"
#include "dev_state.h"
#include "key_common.h"
#include "mw_log.h"
#include "hal.h"

#if MW_ADC_KEY_ENABLE

#define UP_KEY_AD (3700)
#define DOWN_KEY_AD (2776)
#define ENTER_KEY_AD (1788)
#define BACK_KEY_AD (736)

// #define KEY_FILTER_DATA
#define KEYAD_NUM (4)
#define LONG_PRESS_MS (2500)        ///< 长按时间，单位ms
#define SUPER_LONG_PRESS_MS (8000)  ///< 超长按时间，单位ms（仅上键）
#define ADC_SAMPLE_PERIOD_MS (10)
#define ADC_SAMPLE_COUNT (5)          ///< 采集次数，总消抖时间30ms
#define ADC_RELEASE_THRESHOLD (4000)  ///< 大于此值认为释放

#ifdef KEY_FILTER_DATA
#define KEY_ERROR_BY_NEIGHBOR_RANGE (20)
#define KEYAD_RANGE_VALUE (600)
#else
#define KEYAD_RANGE_VALUE (120)
#endif

static uint16_t keyadc_value[KEYAD_NUM] = {UP_KEY_AD, DOWN_KEY_AD, ENTER_KEY_AD,
                                           BACK_KEY_AD};

static uint8_t key_map[KEYAD_NUM] = {
    UP_KEY_PRESS,
    DOWN_KEY_PRESS,
    ENTER_KEY_PRESS,
    BACK_KEY_PRESS,
};

#if MW_KEY_USE_GPIO
/** GPIO 模式引脚逻辑名表（hal_gpio 逻辑名，板级在 dev_config.h 覆盖） */
static const char* s_key_gpio_names[KEYAD_NUM] = {
    MW_KEY_GPIO_UP,
    MW_KEY_GPIO_DOWN,
    MW_KEY_GPIO_ENTER,
    MW_KEY_GPIO_BACK,
};
static gpio_t s_key_gpio_pin[KEYAD_NUM]; /* 解析后的 GPIO 引脚号 */
#endif

static key_state_t s_key_state = KEY_STATE_IDLE;
static int8_t s_pressed_key_idx = -1;  /* 当前按下的按键索引 */
static uint64_t s_press_start_ms = 0;  /* 按键按下开始时刻 */
static bool s_long_sent = false;       /* 长按事件是否已发送 */
static bool s_super_long_sent = false; /* 超长按事件是否已发送 */

static uint16_t s_adc_sample_buf[ADC_SAMPLE_COUNT] = {0};
static uint8_t s_sample_cnt = 0;

static volatile bool s_running = false; /* 扫描线程运行标志 */
static osal_sem_t* s_done = NULL;       /* 线程退出信号（sleep 同步用）*/

#if !MW_KEY_USE_GPIO
/** 读一次 ADC 原始值（平台差异已收敛到 hal_key 后端）*/
static uint16_t adc_read_raw(void) {
    int v = hal_key_adc_read(MW_ADC_KEY_CHANNEL);
    return (v < 0) ? 0 : (uint16_t)v;
}
#endif

static void push_key_to_avg_pool(uint16_t ad_value) {
    if (s_sample_cnt < ADC_SAMPLE_COUNT) {
        s_adc_sample_buf[s_sample_cnt] = ad_value;
        s_sample_cnt++;
    }
}

static bool avg_pool_is_full(void) { return s_sample_cnt == ADC_SAMPLE_COUNT; }

static void reset_key_avg_pool(void) { s_sample_cnt = 0; }

static uint16_t get_key_avg_value(void) {
#ifndef KEY_FILTER_DATA
    uint16_t buf[ADC_SAMPLE_COUNT];
    for (int i = 0; i < s_sample_cnt; i++) {
        buf[i] = s_adc_sample_buf[i];
    }

    ///< 冒泡排序（升序）
    for (int i = 0; i < s_sample_cnt - 1; i++) {
        for (int j = i + 1; j < s_sample_cnt; j++) {
            if (buf[i] > buf[j]) {
                uint16_t tmp = buf[i];
                buf[i] = buf[j];
                buf[j] = tmp;
            }
        }
    }

    ///< 取中位数（偶数个时取中间两个的平均值）
    if (s_sample_cnt % 2 == 1) {
        return buf[s_sample_cnt / 2];
    } else {
        return (buf[s_sample_cnt / 2 - 1] + buf[s_sample_cnt / 2]) / 2;
    }

#else
    uint16_t ulSum;
    uint16_t i;

    bool is_first = true;
    uint16_t count = 0;
    ulSum = 0;

    for (i = 0; i < s_sample_cnt - 1; i++) {
        if (abs(s_adc_sample_buf[i] - s_adc_sample_buf[i + 1]) <
            KEY_ERROR_BY_NEIGHBOR_RANGE) {
            if (is_first) {
                is_first = false;
                ulSum += s_adc_sample_buf[i];
                count++;
            }
            ulSum += s_adc_sample_buf[i + 1];
            count++;
        } else {
            is_first = true;
        }
    }

    if (count == 0) return 0;

    return (ulSum / count);
#endif
}

static int8_t check_key(uint16_t sample_value) {
#ifndef KEY_FILTER_DATA
    if (sample_value > ADC_RELEASE_THRESHOLD) {
        return -1;
    }

    int8_t i = -1;

    for (i = 0; i < (sizeof(keyadc_value) / sizeof(keyadc_value[0])); i++) {
        if ((sample_value >= keyadc_value[i] - KEYAD_RANGE_VALUE) &&
            (sample_value <= keyadc_value[i] + KEYAD_RANGE_VALUE)) {
            return i;
        }
    }

    return -1;
#else
    if (sample_value > ADC_RELEASE_THRESHOLD) {
        return -1;
    }

    int8_t i;
    int8_t id = 0;
    uint16_t range = abs(sample_value - keyadc_value[0]);

    for (i = 1; i < KEYAD_NUM; i++) {
        if (abs(sample_value - keyadc_value[i]) < range) {
            range = abs(sample_value - keyadc_value[i]);
            id = i;
        }
    }

    if (range < KEYAD_RANGE_VALUE) {
        return id;
    }

    return -1;
#endif
}

/**
 * @brief 发送按键事件（短按或长按）
 */
static void adc_send_event(int8_t idx, bool is_long) {
    if (idx < 0 || idx >= KEYAD_NUM) return;
    uint8_t key_code = key_map[idx];
    uint8_t key_event = 0;

    switch (key_code) {
        case UP_KEY_PRESS:
            key_event = is_long ? UP_KEY_LONG_PR : UP_KEY_SHORT_PR;
            mw_log_info("%s\n",
                        is_long ? "up key long press" : "up key short press");
            send_key_event(key_event);
            break;

        case DOWN_KEY_PRESS:
            key_event = is_long ? MODE_KEY_LONG_PR : MODE_KEY_SHORT_PR;
            mw_log_info("%s\n", is_long ? "down key long press"
                                        : "down key short press");
            send_key_event(key_event);
            break;

        case ENTER_KEY_PRESS:
            key_event = is_long ? SET_KEY_LONG_PR : SET_KEY_SHORT_PR;
            mw_log_info("%s\n", is_long ? "enter key long press"
                                        : "enter key short press");
            send_key_event(key_event);
            break;

        case BACK_KEY_PRESS:
            key_event = is_long ? BACK_KEY_LONG_PR : BACK_KEY_SHORT_PR;
            mw_log_info("%s\n", is_long ? "back key long press"
                                        : "back key short press");
            send_key_event(key_event);
            break;

        default:
            break;
    }
}

/**
 * @brief 处理一次平均后的按键判决结果
 * @param cur_key 当前判决的按键索引（-1表示无按键）
 */
static void process_key_decision(int8_t cur_key) {
    uint64_t now_ms = osal_tick_ms();

    switch (s_key_state) {
        case KEY_STATE_IDLE:
            if (cur_key != -1) {
                ///< 检测到有效按键，进入按下状态
                s_key_state = KEY_STATE_PRESSED;
                s_pressed_key_idx = cur_key;
                s_press_start_ms = now_ms;
                s_long_sent = false;
                s_super_long_sent = false;
            }
            break;

        case KEY_STATE_PRESSED:
            if (cur_key != s_pressed_key_idx) {
                ///< 按键已释放或变为其他按键：发送短按（如果未长按
                if (!s_long_sent) {
                    adc_send_event(s_pressed_key_idx, false);
                }
                s_key_state = KEY_STATE_IDLE;
                s_pressed_key_idx = -1;
                s_long_sent = false;
                s_super_long_sent = false;
            } else {
                ///< 按键仍按下，检查长按
                if (!s_long_sent) {
                    uint64_t duration_ms = now_ms - s_press_start_ms;
                    if (duration_ms >= LONG_PRESS_MS) {
                        adc_send_event(s_pressed_key_idx, true);
                        s_long_sent = true;
                        s_key_state = KEY_STATE_LONG_PRESSED;
                    }
                }
            }
            break;

        case KEY_STATE_LONG_PRESSED:
            if (cur_key != s_pressed_key_idx) {
                s_key_state = KEY_STATE_IDLE;
                s_pressed_key_idx = -1;
                s_long_sent = false;
                s_super_long_sent = false;
            } else {
                ///< 上键继续按住，检查超长按
                if (key_map[s_pressed_key_idx] == UP_KEY_PRESS &&
                    !s_super_long_sent) {
                    uint64_t duration_ms = now_ms - s_press_start_ms;
                    if (duration_ms >= SUPER_LONG_PRESS_MS) {
                        key_event_e evt = UP_KEY_SUPER_LONG_PR;
                        mw_log_info("up key super long press\n");
                        send_key_event(evt);
                        s_super_long_sent = true;
                        s_key_state = KEY_STATE_SUPER_LONG_PRESSED;
                    }
                }
            }
            break;

        case KEY_STATE_SUPER_LONG_PRESSED:
            if (cur_key != s_pressed_key_idx) {
                s_key_state = KEY_STATE_IDLE;
                s_pressed_key_idx = -1;
                s_long_sent = false;
                s_super_long_sent = false;
            }
            break;

        default:
            break;
    }
}

static void adc_key_scan(void) {
    if (get_check_self_state() < CHECK_SELF_STATE_SUCCESS) {
        return;
    }

#if MW_KEY_USE_GPIO
    ///< GPIO 模式：逐键读电平，首个按下的键作为判决结果（单键语义）
    int8_t cur_key = -1;
    for (int i = 0; i < KEYAD_NUM; i++) {
        int lv = gpio_read(s_key_gpio_pin[i]);
        if (lv == MW_KEY_GPIO_ACTIVE_LEVEL) {
            cur_key = (int8_t)i;
            break;
        }
    }
    process_key_decision(cur_key);
#else
    uint16_t raw = adc_read_raw();
    push_key_to_avg_pool(raw);
    if (avg_pool_is_full()) {
        uint16_t avg_ad_value = get_key_avg_value();
        int8_t key_code = check_key(avg_ad_value);
        process_key_decision(key_code);
        reset_key_avg_pool();
    }
#endif
}

static void* adc_key_thread(void* arg) {
    (void)arg;
    while (s_running) {
        osal_delay_ms(ADC_SAMPLE_PERIOD_MS);
        if (!s_running) break;
        adc_key_scan();
    }
    if (s_done) osal_sem_post(s_done);
    return NULL;
}

/** 复位状态机与滤波池，防止 sleep/init 循环间残留 */
static void reset_key_state(void) {
    s_key_state = KEY_STATE_IDLE;
    s_pressed_key_idx = -1;
    s_long_sent = false;
    s_super_long_sent = false;
    s_sample_cnt = 0;
}

int adc_key_init(void) {
    if (s_running) return 0; /* 幂等 */

#if MW_KEY_USE_GPIO
    for (int i = 0; i < KEYAD_NUM; i++) {
        s_key_gpio_pin[i] = gpio_get(s_key_gpio_names[i]);
        if (s_key_gpio_pin[i] == GPIO_INVALID) {
            mw_log_error("key gpio %s invalid\n", s_key_gpio_names[i]);
            return -1;
        }
        gpio_set_dir(s_key_gpio_pin[i], GPIO_DIR_INPUT);
        gpio_set_pull(s_key_gpio_pin[i],
                      MW_KEY_GPIO_ACTIVE_LEVEL ? GPIO_PULL_DOWN : GPIO_PULL_UP);
    }
#endif

    reset_key_state();

    s_done = osal_sem_create(0, 1);
    if (!s_done) {
        mw_log_error("create done sem failed\n");
        return -1;
    }

    s_running = true;
    if (osal_thread_create(NULL, "adc_key", adc_key_thread, NULL, 0, 0) !=
        OSAL_OK) {
        s_running = false;
        osal_sem_delete(s_done);
        s_done = NULL;
        mw_log_error("create adc key thread failed\n");
        return -1;
    }

    mw_log_info("adc key init success\n");
    return 0;
}

/** 休眠前清理: 置停标志 -> 等 done 信号量（超时 2 个采样周期兜底）->
 * 复位状态机。唤醒路径不变（IGN_ON 走 cpu reset，起来后 init 重拉起）。 */
void adc_key_sleep(void) {
    if (!s_running) return;

    s_running = false;
    if (s_done) {
        (void)osal_sem_timedwait(s_done, 2 * ADC_SAMPLE_PERIOD_MS + 5);
        osal_sem_delete(s_done);
        s_done = NULL;
    }

    reset_key_state();
    mw_log_info("adc key sleep: scan thread stopped\n");
}

#endif
