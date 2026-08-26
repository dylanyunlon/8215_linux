#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

/**
 * hal_gpio.h -- 极简 GPIO HAL
 *
 * 设计原则：参数最少、返回值直接可用。
 *   - gpio_get(name)  一次性把"逻辑引脚名"解析成后端引脚号，后续操作只传该号
 *   - gpio_read()      直接返回 0/1（<0 表示出错），无需输出参数
 *   - 引脚名（如 "PD.7"）跨系统统一：RT-Thread 原生支持；Linux/FreeRTOS
 * 由板级表解析
 *
 * 与 OSAL 解耦：本层只依赖 <stdint.h>，不包含任何 OS 头文件，
 * 因此 gpio_light.c 等应用模块仅包含 osal.h + hal_gpio.h 即可三系统同源。
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef int gpio_t; 
#define GPIO_INVALID (-1)

typedef enum {
    GPIO_DIR_INPUT = 0,
    GPIO_DIR_OUTPUT,
} gpio_dir_t;

typedef enum {
    GPIO_PULL_NONE = 0,
    GPIO_PULL_UP,
    GPIO_PULL_DOWN,
} gpio_pull_t;

/**
 * 解析逻辑引脚名为后端引脚号。
 * @param name  逻辑名，如 "PD.7"
 * @return      引脚号；失败返回 GPIO_INVALID
 */
gpio_t gpio_get(const char* name);

int gpio_set_dir(gpio_t pin, gpio_dir_t dir);
int gpio_read(gpio_t pin); /* 返回 0/1；<0 出错 */
int gpio_write(gpio_t pin, int value);
int gpio_toggle(gpio_t pin);
int gpio_set_pull(gpio_t pin, gpio_pull_t pull);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_GPIO_H__ */
