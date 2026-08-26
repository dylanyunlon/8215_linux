/** hal_bsp 使用模板 -- 复制到 BSP 工程改填板级参数
 * （本文件不参与 lib_mw 编译，仅作参考；Makefile SRCS 未包含） */
#include "hal_bsp.h"

/* ---- 1. GPIO 逻辑名 -> 系统引脚号（Linux 后端）---- */
static const hal_bsp_gpio_map_t s_gpio_map[] = {
    {"PD.7", 271}, /* 举例：具体号查板级原理图/设备树 */
    {"PG.0", 288},
};

/* ---- 2. 按键 ADC 读取（复用特殊通道时填，如触摸屏 RTP 复用）---- */
static int board_adc_read(uint8_t channel) {
    (void)channel;
    return 0; /* 读板级 ADC 原始值返回 */
}

/* ---- 3. PWM 板级操作表（vendor 内核驱动无用户态接口时填）---- */
static int board_pwm_config(int ch, uint32_t duty_ns, uint32_t period_ns) {
    (void)ch; (void)duty_ns; (void)period_ns;
    return 0; /* 调板级 PWM 通道配置 */
}
static int board_pwm_enable(int ch, int enable) {
    (void)ch; (void)enable;
    return 0;
}
static const hal_pwm_ops_t s_pwm_ops = {
    board_pwm_config,
    board_pwm_enable,
};

/* ---- 4. I2C 板级操作表（无标准框架时填）---- */
static int board_i2c_transfer(hal_i2c_bus_t* bus, struct hal_i2c_msg* msgs,
                              int num) {
    (void)bus; (void)msgs; (void)num;
    return 0; /* 按板级 I2C 控制器实现 msg 事务 */
}
static const hal_i2c_ops_t s_i2c_ops = {
    board_i2c_transfer,
};

/* ---- 5. 板级描述与注册（BSP 启动早期调用一次）---- */
static const hal_bsp_desc_t s_board = {
    .board_name   = "8215E-demo-v2",
    .gpio_map     = s_gpio_map,
    .gpio_map_n   = (int)(sizeof(s_gpio_map) / sizeof(s_gpio_map[0])),
    .key_adc_read = board_adc_read,
    .adc_read     = board_adc_read, /* 通用 ADC(光感等)，可另给独立实现 */
    .pwm_ops      = &s_pwm_ops,
    .i2c_ops      = &s_i2c_ops,
};

void board_hal_bsp_init(void) { hal_bsp_register(&s_board); }
