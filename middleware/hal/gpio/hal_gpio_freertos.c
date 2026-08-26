/** 极简 GPIO HAL -- FreeRTOS 后端（板级驱动回调） */
#include "hal_gpio.h"
#include <stdint.h>

/**
 * FreeRTOS 无统一 GPIO 框架，故由 BSP 注册一组回调。
 * 引脚号编码为 (port<<16)|pin，port/pin 各 16 位。
 */
struct gpio_board_ops {
    int (*resolve)(const char* name, uint32_t* port, uint32_t* pin);
    int (*set_dir)(uint32_t port, uint32_t pin, gpio_dir_t dir);
    int (*read)(uint32_t port, uint32_t pin); /* 0/1; <0 出错 */
    int (*write)(uint32_t port, uint32_t pin, int value);
    int (*set_pull)(uint32_t port, uint32_t pin, gpio_pull_t pull);
};

static const struct gpio_board_ops* s_ops;

/** BSP 在启动时注册板级 GPIO 驱动 */
void gpio_freertos_register_board(const struct gpio_board_ops* ops) {
    s_ops = ops;
}

gpio_t gpio_get(const char* name) {
    if (!s_ops || !s_ops->resolve || !name) return GPIO_INVALID;
    uint32_t port, pin;
    if (s_ops->resolve(name, &port, &pin)) return GPIO_INVALID;
    return (gpio_t)((port << 16) | (pin & 0xFFFF));
}

static void decode(gpio_t g, uint32_t* port, uint32_t* pin) {
    *port = (uint32_t)g >> 16;
    *pin = (uint32_t)g & 0xFFFF;
}

int gpio_set_dir(gpio_t pin, gpio_dir_t dir) {
    if (!s_ops || !s_ops->set_dir || pin < 0) return -1;
    uint32_t p, n;
    decode(pin, &p, &n);
    return s_ops->set_dir(p, n, dir);
}

int gpio_read(gpio_t pin) {
    if (!s_ops || !s_ops->read || pin < 0) return -1;
    uint32_t p, n;
    decode(pin, &p, &n);
    return s_ops->read(p, n);
}

int gpio_write(gpio_t pin, int value) {
    if (!s_ops || !s_ops->write || pin < 0) return -1;
    uint32_t p, n;
    decode(pin, &p, &n);
    return s_ops->write(p, n, value);
}

int gpio_toggle(gpio_t pin) {
    int v = gpio_read(pin);
    if (v < 0) return -1;
    return gpio_write(pin, !v);
}

int gpio_set_pull(gpio_t pin, gpio_pull_t pull) {
    if (!s_ops || !s_ops->set_pull || pin < 0) return -1;
    uint32_t p, n;
    decode(pin, &p, &n);
    return s_ops->set_pull(p, n, pull);
}
