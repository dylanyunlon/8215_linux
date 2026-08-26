#include <stdlib.h>
#include "hal_pwm.h"
#include "mw_log.h"
#include "usr_backlight.h"

#if MW_BL_PWM_ENABLE

#define LEVEL_1_DUTY_VALUE (600000)
#define LEVEL_2_DUTY_VALUE (400000)
#define LEVEL_3_DUTY_VALUE (200000)
#define LEVEL_4_DUTY_VALUE (100000)
#define LEVEL_5_DUTY_VALUE (10000)
#define BREATH_BACKLIGHT_PERIOD (80)

typedef struct {
    uint32_t pre_led_value;
    uint32_t cur_led_value;
    uint32_t target_led_value;
    uint32_t inc_led_value;
} breath_bl_param_t;

static uint8_t led_level = 0;
static uint8_t auto_led_level = 0;
static uint8_t led_percent = 0; ///< 当前亮度百分比(0~100)
static breath_bl_param_t auto_backlight;

static uint32_t get_duty_cycle_value(uint8_t level) {
    uint32_t value = LEVEL_1_DUTY_VALUE;
    switch (level) {
        case 1:
            value = LEVEL_1_DUTY_VALUE;
            break;
        case 2:
        case 3:
            value = LEVEL_3_DUTY_VALUE;
            break;
        case 4:
        case 5:
            value = LEVEL_5_DUTY_VALUE;
            break;
        default:
            break;
    }
    return value;
}

void set_backlight_level(uint8_t level) {
    if (level != led_level) {
        if (level >= BACKLIGHT_LEVEL_1 && level <= BACKLIGHT_LEVEL_5) {
            uint32_t level_value = LEVEL_3_DUTY_VALUE;
            switch (level) {
                case 1:
                    level_value = LEVEL_1_DUTY_VALUE;
                    break;
                case 2:
                    level_value = LEVEL_2_DUTY_VALUE;
                    break;
                case 3:
                    level_value = LEVEL_3_DUTY_VALUE;
                    break;
                case 4:
                    level_value = LEVEL_4_DUTY_VALUE;
                    break;
                case 5:
                    level_value = LEVEL_5_DUTY_VALUE;
                    break;
                default:
                    break;
            }

            hal_pwm_config(MW_LCD_PWM_CH, level_value, PWM_BACKLIGHT_PERION);
            hal_pwm_enable(MW_LCD_PWM_CH);
            auto_backlight.target_led_value = level_value;
            auto_backlight.cur_led_value = level_value;
            mw_log_info("Set pwm value:%lu\n", level_value);
            led_level = level;
        }
    }
}

void set_backlight_percent(uint8_t percent) {
    if (percent > 100) {
        percent = 100;
    }
    if (percent == led_percent) {
        return;
    }
    if (percent == 0) {
        hal_pwm_disable(MW_LCD_PWM_CH);
    } else {
        uint32_t duty =
            (uint32_t)percent * PWM_BACKLIGHT_PERION / 100;

        hal_pwm_config(MW_LCD_PWM_CH, duty, PWM_BACKLIGHT_PERION);
        hal_pwm_enable(MW_LCD_PWM_CH);
        auto_backlight.target_led_value = duty;
        auto_backlight.cur_led_value = duty;
    }
    led_percent = percent;
    mw_log_info("Set backlight percent:%u", percent);
}

void init_auto_backlight(uint8_t level) {
    if (auto_led_level != level) {
        uint32_t level_value = get_duty_cycle_value(level);
        hal_pwm_config(MW_LCD_PWM_CH, level_value, PWM_BACKLIGHT_PERION);
        hal_pwm_enable(MW_LCD_PWM_CH);
        auto_backlight.target_led_value = level_value;
        auto_backlight.cur_led_value = level_value;
        auto_led_level = level;
    }
}

void init_backlight_pwm(void) {
    hal_pwm_config(MW_LCD_PWM_CH, PWM_BACKLIGHT_PERION, PWM_BACKLIGHT_PERION);
    hal_pwm_enable(MW_LCD_PWM_CH);
    mw_log_info("init pwm value:1000000\r\n");
}
#endif


