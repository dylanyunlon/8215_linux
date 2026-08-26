#ifndef __USR_BACKLIGHT_H__
#define __USR_BACKLIGHT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "dev_config.h"

#if MW_BL_PWM_ENABLE

#define PWM_BACKLIGHT_PERION (1000000)  ///< pwm背光周期1MHZ

typedef enum {
    BACKLIGHT_LEVEL_AUTO = 0x00,  ///< 等级自动
    BACKLIGHT_LEVEL_1,
    BACKLIGHT_LEVEL_2,
    BACKLIGHT_LEVEL_3,
    BACKLIGHT_LEVEL_4,
    BACKLIGHT_LEVEL_5
} led_level_e;

void set_backlight_level(uint8_t level);

/**
 * @brief  按百分比设置背光亮度
 * @param  percent: 0~100（0=关闭背光输出）
 * @return none
 */
void set_backlight_percent(uint8_t percent);

void init_backlight_pwm(void);

/**
 * @brief  init auto backlight
 * @param  level: backlight level
 * @return none
 */
void init_auto_backlight(uint8_t level);

#endif

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  // __USR_BACKLIGHT_H__