#ifndef __HAL_PWM_H__
#define __HAL_PWM_H__

/**
 * @file hal_pwm.h
 * @brief 极简 PWM HAL -- 背光/呼吸灯等调光所需的唯一硬件原语
 *
 * 设计原则（同 hal_gpio.h / hal_key.h）：参数最少、返回值直接可用、
 * 单位统一纳秒（Linux sysfs 与 RT-Thread rt_pwm 框架均为 ns，直通零换算）。
 *
 *   - hal_pwm_config()    一次配置 周期+占空比（ns）
 *   - hal_pwm_set_duty()  只改占空比（渐变/呼吸灯高频调用，后端缓存周期）
 *   - hal_pwm_enable()/disable()  启停输出
 *
 * 后端策略（板级回调优先，同 hal_key.h 模式）：
 *   - Linux     : 默认 sysfs /sys/class/pwm/pwmchipN。AC83xx vendor PWM
 *                 驱动仅 EXPORT_SYMBOL、无用户态接口，此类板须由 BSP 桥接
 *                 并注册板级操作表
 *   - RT-Thread : 默认 rt_device PWM 框架（设备名默认 "pwm0"，可改）
 *   - FreeRTOS  : 无统一 PWM 框架，仅板级操作表
 *
 * 与 OSAL 解耦：本层只依赖 <stdint.h>，不包含任何 OS 头文件。
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef int pwm_t;  ///< 逻辑 PWM 通道号（板级语义）
#define PWM_INVALID (-1)

/** 板级 PWM 操作表：覆盖后端默认实现 */
typedef struct {
    /** 配置周期+占空比（ns）；返回 0 成功 */
    int (*config)(pwm_t ch, uint32_t duty_ns, uint32_t period_ns);
    int (*enable)(pwm_t ch, int enable);  ///< enable: 1=输出 0=停止
} hal_pwm_ops_t;

/** 配置周期与占空比（ns）。duty_ns 须 <= period_ns。 */
int hal_pwm_config(pwm_t ch, uint32_t duty_ns, uint32_t period_ns);

/** 只改占空比（周期取本通道上次 config 的值） */
int hal_pwm_set_duty(pwm_t ch, uint32_t duty_ns);

int hal_pwm_enable(pwm_t ch);   ///< 启动输出（须先 config）
int hal_pwm_disable(pwm_t ch);  ///< 停止输出

/** 注册板级操作表（覆盖后端默认实现；传 NULL 恢复默认） */
void hal_pwm_register_board(const hal_pwm_ops_t* ops);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_PWM_H__ */
