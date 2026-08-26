/** 极简 I2C HAL -- FreeRTOS 后端（板级操作表，同 hal_key_freertos.c 模式）
 * FreeRTOS 无统一 I2C 框架，由 BSP 注册操作表；未注册时返回出错。 */
#include "hal_i2c.h"
#include <stdio.h>
#include <stdlib.h>

struct hal_i2c_bus {
    char name[16];
};

static const hal_i2c_ops_t* s_board_ops = NULL;

void hal_i2c_register_board(const hal_i2c_ops_t* ops) { s_board_ops = ops; }

const char* hal_i2c_bus_name(const hal_i2c_bus_t* bus) {
    return bus ? bus->name : NULL;
}

hal_i2c_bus_t* hal_i2c_open(const char* name) {
    if (!name || !name[0] || !s_board_ops) return NULL; /* 无板级实现 */
    struct hal_i2c_bus* b = (struct hal_i2c_bus*)calloc(1, sizeof(*b));
    if (!b) return NULL;
    snprintf(b->name, sizeof(b->name), "%s", name);
    return b;
}

void hal_i2c_close(hal_i2c_bus_t* bus) { free(bus); }

int hal_i2c_transfer(hal_i2c_bus_t* bus, struct hal_i2c_msg* msgs, int num) {
    if (!bus || !msgs || num <= 0 || num > HAL_I2C_MSG_MAX) return -1;
    if (!s_board_ops || !s_board_ops->transfer) return -1;
    return s_board_ops->transfer(bus, msgs, num);
}
