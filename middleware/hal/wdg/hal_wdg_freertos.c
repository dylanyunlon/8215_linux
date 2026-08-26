/** 看门狗 HAL -- FreeRTOS 后端：仅板级 ops（无统一框架） */
#include "hal_wdg.h"

static const hal_wdg_ops_t* s_board_ops = NULL;

void hal_wdg_register_board(const hal_wdg_ops_t* ops) { s_board_ops = ops; }

int hal_wdg_init(uint32_t timeout_ms) {
    if (s_board_ops && s_board_ops->init)
        return s_board_ops->init(timeout_ms);
    return -1; /* 未注册板级 ops：无硬件狗 */
}

int hal_wdg_feed(void) {
    if (s_board_ops && s_board_ops->feed) return s_board_ops->feed();
    return -1;
}
