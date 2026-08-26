/** 极简 I2C HAL -- RT-Thread 后端
 * 板级回调优先；RT_USING_I2C 启用时兜底 rt_i2c 框架
 * （rt_i2c_bus_device_find + rt_i2c_transfer，msg 字段直通），
 * 未启用框架时本后端仅板级回调（编译不破）。 */
#include "hal_i2c.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef RT_USING_I2C
#include <rtthread.h>
#include <rtdevice.h>

#ifndef RT_I2C_WR
#define RT_I2C_WR 0x0000  /* 写方向 */
#endif
#ifndef RT_I2C_RD
#define RT_I2C_RD 0x0001  /* 读方向 */
#endif
#endif /* RT_USING_I2C */

struct hal_i2c_bus {
    char name[16];
#ifdef RT_USING_I2C
    struct rt_i2c_bus_device* dev; /* 懒 find：首次 transfer 才定位 */
#endif
};

static const hal_i2c_ops_t* s_board_ops = NULL;

void hal_i2c_register_board(const hal_i2c_ops_t* ops) { s_board_ops = ops; }

const char* hal_i2c_bus_name(const hal_i2c_bus_t* bus) {
    return bus ? bus->name : NULL;
}

hal_i2c_bus_t* hal_i2c_open(const char* name) {
    if (!name || !name[0]) return NULL;
    struct hal_i2c_bus* b = (struct hal_i2c_bus*)calloc(1, sizeof(*b));
    if (!b) return NULL;
    snprintf(b->name, sizeof(b->name), "%s", name);
    return b;
}

void hal_i2c_close(hal_i2c_bus_t* bus) { free(bus); }

int hal_i2c_transfer(hal_i2c_bus_t* bus, struct hal_i2c_msg* msgs, int num) {
    if (!bus || !msgs || num <= 0 || num > HAL_I2C_MSG_MAX) return -1;
    if (s_board_ops && s_board_ops->transfer)
        return s_board_ops->transfer(bus, msgs, num);

#ifdef RT_USING_I2C
    if (bus->dev == RT_NULL) {
        bus->dev = rt_i2c_bus_device_find(bus->name);
        if (bus->dev == RT_NULL) return -1;
    }

    struct rt_i2c_msg rm[HAL_I2C_MSG_MAX];
    for (int i = 0; i < num; i++) {
        rm[i].addr = msgs[i].addr;
        rm[i].flags = msgs[i].flags; /* HAL_I2C_RD 位值同 RT_I2C_RD */
        rm[i].len = msgs[i].len;
        rm[i].buf = msgs[i].buf;
    }
    return rt_i2c_transfer(bus->dev, rm, num) == num ? 0 : -1;
#else
    return -1; /* 无框架且无板级回调 */
#endif
}
