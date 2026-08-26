/** 板级参数统一注册 -- 聚合分发到各 HAL 后端（三系统同源）
 * BSP 只需构造一份 hal_bsp_desc_t 一次注册；NULL 字段保持后端默认。 */
#include "hal_bsp.h"
#include "mw_log.h"

static const char* s_board_name = NULL;

const char* hal_bsp_board_name(void) {
    return s_board_name ? s_board_name : "unknown";
}

int hal_bsp_register(const hal_bsp_desc_t* bsp) {
    if (!bsp) return -1;

#if defined(OSAL_OS_LINUX)
    if (bsp->gpio_map && bsp->gpio_map_n > 0)
        gpio_linux_register_map(bsp->gpio_map, bsp->gpio_map_n);
#endif
    if (bsp->key_adc_read) hal_key_adc_register_board(bsp->key_adc_read);
    if (bsp->adc_read) hal_adc_register_board(bsp->adc_read);
    if (bsp->pwm_ops) hal_pwm_register_board(bsp->pwm_ops);
    if (bsp->i2c_ops) hal_i2c_register_board(bsp->i2c_ops);
    if (bsp->wdg_ops) hal_wdg_register_board(bsp->wdg_ops);
    if (bsp->can_ops) hal_can_register_board(bsp->can_ops);

    if (bsp->board_name) s_board_name = bsp->board_name;
    mw_log_info("hal_bsp registered: board=%s\n", hal_bsp_board_name());
    return 0;
}
