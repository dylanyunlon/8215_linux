/**
*
* @file hcn_backlight.c
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2025/09/03 14:12
* @author och
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define LEVEL_1_DUTY_VALUE (500000)
#define LEVEL_2_DUTY_VALUE (400000)
#define LEVEL_3_DUTY_VALUE (600000)
#define LEVEL_4_DUTY_VALUE (200000)
#define LEVEL_5_DUTY_VALUE (100000)

#define BREATH_BACKLIGHT_PERIOD (80)

typedef struct {
    uint32_t pre_led_value;
    uint32_t cur_led_value;
    uint32_t target_led_value;
    uint32_t inc_led_value;
} breath_bl_param_t;

static uint8_t led_level = 0;
static uint8_t auto_led_level = 0;
static breath_bl_param_t auto_backlight;

static uint32_t get_dutu_cycle_value(uint8_t level) {

}

void set_backlight_level(uint8_t level) {
    
}

void init_auto_backlight(uint8_t level) {

}

void init_lcd_bl_pwm(void) {

}

void update_breath_backlight_level(uint8_t level) {
    
}

bool is_change_backlight_success(uint8_t level) {
    return 0 ;
}


