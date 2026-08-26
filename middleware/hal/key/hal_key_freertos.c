/** 极简按键采集 HAL -- FreeRTOS 后端（板级回调，同 hal_gpio_freertos.c 模式）
 * FreeRTOS 无统一 ADC 框架，由 BSP 注册读取回调；未注册时返回出错。 */
#include "hal_key.h"

static hal_key_adc_read_fn s_board_fn = NULL;

void hal_key_adc_register_board(hal_key_adc_read_fn fn) { s_board_fn = fn; }

int hal_key_adc_read(uint8_t channel) {
    if (!s_board_fn) return -1;
    return s_board_fn(channel);
}
