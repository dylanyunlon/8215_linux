/** 极简 GPIO HAL -- RT-Thread rt_pin 后端 */
#include "hal_gpio.h"
#include <rtthread.h>
#include <rtdevice.h>

/** RT-Thread 原生支持 "PD.7" 这类引脚名解析，无需板级映射表 */
gpio_t gpio_get(const char* name) {
    if (!name) return GPIO_INVALID;
    rt_base_t pin = rt_pin_get(name);
    return (pin < 0) ? GPIO_INVALID : (gpio_t)pin;
}

int gpio_set_dir(gpio_t pin, gpio_dir_t dir) {
    if (pin < 0) return -1;
    rt_pin_mode(pin, dir == GPIO_DIR_OUTPUT ? PIN_MODE_OUTPUT : PIN_MODE_INPUT);
    return 0;
}

int gpio_read(gpio_t pin) {
    if (pin < 0) return -1;
    return rt_pin_read(pin) ? 1 : 0;
}

int gpio_write(gpio_t pin, int value) {
    if (pin < 0) return -1;
    rt_pin_write(pin, value ? PIN_HIGH : PIN_LOW);
    return 0;
}

int gpio_toggle(gpio_t pin) {
    if (pin < 0) return -1;
    rt_pin_write(pin, rt_pin_read(pin) ? PIN_LOW : PIN_HIGH);
    return 0;
}

int gpio_set_pull(gpio_t pin, gpio_pull_t pull) {
    if (pin < 0) return -1;
    rt_pin_mode_t m;
    switch (pull) {
        case GPIO_PULL_UP:
            m = PIN_MODE_INPUT_PULLUP;
            break;
        case GPIO_PULL_DOWN:
            m = PIN_MODE_INPUT_PULLDOWN;
            break;
        default:
            m = PIN_MODE_INPUT;
            break;
    }
    rt_pin_mode(pin, m);
    return 0;
}
