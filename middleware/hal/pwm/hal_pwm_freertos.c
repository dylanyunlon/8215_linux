/** 极简 PWM HAL -- FreeRTOS 后端（板级操作表，同 hal_key_freertos.c 模式）
 * FreeRTOS 无统一 PWM 框架，由 BSP 注册操作表；未注册时返回出错。 */
#include "hal_pwm.h"

#define PWM_MAX_CH 8

static const hal_pwm_ops_t* s_board_ops = NULL;

/** 每通道缓存上次 period，供 hal_pwm_set_duty 只改占空比 */
static uint32_t s_period_ns[PWM_MAX_CH];

void hal_pwm_register_board(const hal_pwm_ops_t* ops) { s_board_ops = ops; }

int hal_pwm_config(pwm_t ch, uint32_t duty_ns, uint32_t period_ns) {
    if (ch < 0 || ch >= PWM_MAX_CH) return -1;
    if (period_ns == 0 || duty_ns > period_ns) return -1;
    s_period_ns[ch] = period_ns;
    if (!s_board_ops || !s_board_ops->config) return -1;
    return s_board_ops->config(ch, duty_ns, period_ns);
}

int hal_pwm_set_duty(pwm_t ch, uint32_t duty_ns) {
    if (ch < 0 || ch >= PWM_MAX_CH) return -1;
    if (s_period_ns[ch] == 0 || duty_ns > s_period_ns[ch]) return -1;
    if (!s_board_ops || !s_board_ops->config) return -1;
    return s_board_ops->config(ch, duty_ns, s_period_ns[ch]);
}

int hal_pwm_enable(pwm_t ch) {
    if (ch < 0 || ch >= PWM_MAX_CH) return -1;
    if (!s_board_ops || !s_board_ops->enable) return -1;
    return s_board_ops->enable(ch, 1);
}

int hal_pwm_disable(pwm_t ch) {
    if (ch < 0 || ch >= PWM_MAX_CH) return -1;
    if (!s_board_ops || !s_board_ops->enable) return -1;
    return s_board_ops->enable(ch, 0);
}
