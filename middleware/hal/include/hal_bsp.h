#ifndef __HAL_BSP_H__
#define __HAL_BSP_H__

/**
 * @file hal_bsp.h
 * @brief 板级参数统一注册 -- BSP 一次调用，分发到各 HAL 后端
 *
 * 背景：各 HAL 的板级注册入口分散（GPIO 映射表、按键 ADC 回调、
 * PWM/I2C 操作表），BSP 需逐个找头文件与函数名。本层聚合为一份
 * hal_bsp_desc_t + 一次 hal_bsp_register()，NULL 字段=保持后端默认。
 *
 * 用法（BSP 启动早期、mw_init 之前调用一次）：
 *   static const hal_bsp_gpio_map_t s_gpio[] = {{"PD.7", 271}};
 *   static const hal_bsp_desc_t s_bsp = {
 *       .board_name   = "8215E-demo-v2",
 *       .gpio_map     = s_gpio,
 *       .gpio_map_n   = 1,
 *       .key_adc_read = board_adc_read,  // 可选
 *       .pwm_ops      = &board_pwm_ops,  // 可选
 *       .i2c_ops      = &board_i2c_ops,  // 可选
 *   };
 *   hal_bsp_register(&s_bsp);
 *
 * 完整示例见 hal/bsp/hal_bsp_example.c（模板，不参与编译）。
 * 新增 HAL 的板级参数：desc 加字段 + 本层聚合一行即可。
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "hal_key.h"
#include "hal_adc.h"
#include "hal_pwm.h"
#include "hal_i2c.h"
#include "hal_can.h"
#include "hal_wdg.h"

/** GPIO 板级映射：逻辑名 -> 系统引脚号（Linux 后端为全局 GPIO 号） */
typedef struct {
    const char* name;  ///< 逻辑名，如 "PD.7"
    int gpio;          ///< 系统引脚号
} hal_bsp_gpio_map_t;

/** 板级描述：一次注册所有板级参数（NULL/0 字段=保持后端默认） */
typedef struct {
    const char* board_name;             ///< 板/机型名（诊断/版本输出用）
    const hal_bsp_gpio_map_t* gpio_map; ///< GPIO 映射表（Linux 后端）
    int gpio_map_n;                     ///< 映射表长度
    hal_key_adc_read_fn key_adc_read;   ///< 按键 ADC 读取回调
    hal_adc_read_fn adc_read;           ///< 通用 ADC 读取回调(光感等)
    const hal_pwm_ops_t* pwm_ops;       ///< PWM 板级操作表
    const hal_i2c_ops_t* i2c_ops;       ///< I2C 板级操作表
    const hal_wdg_ops_t* wdg_ops;       ///< 看门狗板级操作表
    const hal_can_ops_t* can_ops;       ///< CAN 板级操作表
} hal_bsp_desc_t;

/** 统一注册（可重复调用，后注册覆盖）。bsp=NULL 返回 -1。 */
int hal_bsp_register(const hal_bsp_desc_t* bsp);

/** 已注册的板/机型名（未注册返回 "unknown"） */
const char* hal_bsp_board_name(void);

/** hal_gpio_linux.c 板级映射注册（hal_bsp_register 内部聚合调用，
 *  亦可单独使用）。仅 Linux 后端提供定义；不依赖 OS 宏的包含顺序，
 *  头文件被单独包含时声明始终可见。 */
void gpio_linux_register_map(const hal_bsp_gpio_map_t* map, int n);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_BSP_H__ */
