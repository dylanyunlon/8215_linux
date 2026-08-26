/** 极简 CAN HAL -- RT-Thread 后端（板级操作表，同 hal_i2c_freertos.c 模式）
 * RT-Thread 的 rt_device can 框架各版本接口差异较大（open/control/read/
 * write + 回调），由 BSP 桥接成 hal_can_ops_t 注册；未注册时收发出错。 */
#include "hal_can.h"
#include <stdio.h>
#include <stdlib.h>

struct hal_can {
    char name[16];
};

static const hal_can_ops_t* s_board_ops = NULL;

void hal_can_register_board(const hal_can_ops_t* ops) { s_board_ops = ops; }

const char* hal_can_name(const hal_can_t* can) {
    return can ? can->name : NULL;
}

hal_can_t* hal_can_open(const char* name) {
    if (!name || !name[0] || !s_board_ops) return NULL; /* 无板级实现 */
    struct hal_can* c = (struct hal_can*)calloc(1, sizeof(*c));
    if (!c) return NULL;
    snprintf(c->name, sizeof(c->name), "%s", name);
    return c;
}

void hal_can_close(hal_can_t* can) { free(can); }

int hal_can_send(hal_can_t* can, const struct hal_can_frame* frames, int num,
                 uint32_t timeout_ms) {
    if (!can || !frames || num <= 0) return -1;
    if (!s_board_ops || !s_board_ops->send) return -1;
    return s_board_ops->send(can, frames, num, timeout_ms);
}

int hal_can_recv(hal_can_t* can, struct hal_can_frame* frames, int num,
                 uint32_t timeout_ms) {
    if (!can || !frames || num <= 0) return -1;
    if (!s_board_ops || !s_board_ops->recv) return -1;
    return s_board_ops->recv(can, frames, num, timeout_ms);
}

int hal_can_set_filter(hal_can_t* can, const uint32_t* ids, int num) {
    if (!can) return -1;
    if (!s_board_ops || !s_board_ops->set_filter) return -1;
    return s_board_ops->set_filter(can, ids, num);
}
