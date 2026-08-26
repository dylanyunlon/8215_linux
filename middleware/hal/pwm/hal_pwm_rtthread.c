/** 极简 PWM HAL -- RT-Thread 后端
 * 板级回调优先（中间件不含任何厂商头文件）；无回调时兜底 rt_device
 * PWM 框架（配置结构布局同 struct rt_pwm_configuration，单位 ns）。 */
#include "hal_pwm.h"
#include <rtthread.h>
#include <rtdevice.h>

#define PWM_MAX_CH 8

static const hal_pwm_ops_t* s_board_ops = NULL;
static const char* s_dev_name = "pwm0";
static rt_device_t s_dev = RT_NULL;

/** 每通道缓存上次 period，供 hal_pwm_set_duty 只改占空比 */
static uint32_t s_period_ns[PWM_MAX_CH];

/* rt_device PWM 框架命令字（框架头未包含命令定义时兜底，与
 * RT-Thread drivers/include/pwm.h 一致） */
#ifndef PWM_CMD_ENABLE
#define PWM_CMD_ENABLE (128 + 0)
#endif
#ifndef PWM_CMD_DISABLE
#define PWM_CMD_DISABLE (128 + 1)
#endif
#ifndef PWM_CMD_SET
#define PWM_CMD_SET (128 + 2)
#endif

/** PWM 配置参数（布局同 struct rt_pwm_configuration，单位 ns） */
struct hal_pwm_cfg {
    rt_uint32_t channel;  ///< 硬件通道号（板级语义）
    rt_uint32_t period;   ///< 周期 ns
    rt_uint32_t pulse;    ///< 占空 ns
};

void hal_pwm_register_board(const hal_pwm_ops_t* ops) {
    s_board_ops = ops;
    if (ops) s_dev = RT_NULL; /* 切板级回调，释放框架设备引用 */
}

/** 板级指定 rt_device PWM 设备名（默认 "pwm0"）。仅本后端有效。 */
void hal_pwm_rtthread_set_device(const char* name) {
    if (!name || !name[0]) return;
    s_dev_name = name;
    s_dev = RT_NULL; /* 换设备名后强制下次重新 find/open */
}

static int dev_open(void) {
    if (s_dev != RT_NULL) return 0;
    s_dev = rt_device_find(s_dev_name);
    if (s_dev == RT_NULL) return -1;
    if (rt_device_open(s_dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK) {
        s_dev = RT_NULL;
        return -1;
    }
    return 0;
}

int hal_pwm_config(pwm_t ch, uint32_t duty_ns, uint32_t period_ns) {
    if (ch < 0 || ch >= PWM_MAX_CH) return -1;
    if (period_ns == 0 || duty_ns > period_ns) return -1;
    s_period_ns[ch] = period_ns;
    if (s_board_ops && s_board_ops->config)
        return s_board_ops->config(ch, duty_ns, period_ns);
    if (dev_open() != 0) return -1;

    struct hal_pwm_cfg cfg = { (rt_uint32_t)ch, period_ns, duty_ns };
    return rt_device_control(s_dev, PWM_CMD_SET, &cfg) == RT_EOK ? 0 : -1;
}

int hal_pwm_set_duty(pwm_t ch, uint32_t duty_ns) {
    if (ch < 0 || ch >= PWM_MAX_CH) return -1;
    if (s_period_ns[ch] == 0 || duty_ns > s_period_ns[ch]) return -1;
    if (s_board_ops && s_board_ops->config)
        return s_board_ops->config(ch, duty_ns, s_period_ns[ch]);
    if (dev_open() != 0) return -1;

    struct hal_pwm_cfg cfg = { (rt_uint32_t)ch, s_period_ns[ch], duty_ns };
    return rt_device_control(s_dev, PWM_CMD_SET, &cfg) == RT_EOK ? 0 : -1;
}

int hal_pwm_enable(pwm_t ch) {
    if (ch < 0 || ch >= PWM_MAX_CH) return -1;
    if (s_board_ops && s_board_ops->enable) return s_board_ops->enable(ch, 1);
    if (dev_open() != 0) return -1;

    struct hal_pwm_cfg cfg = { (rt_uint32_t)ch, 0, 0 };
    return rt_device_control(s_dev, PWM_CMD_ENABLE, &cfg) == RT_EOK ? 0 : -1;
}

int hal_pwm_disable(pwm_t ch) {
    if (ch < 0 || ch >= PWM_MAX_CH) return -1;
    if (s_board_ops && s_board_ops->enable) return s_board_ops->enable(ch, 0);
    if (dev_open() != 0) return -1;

    struct hal_pwm_cfg cfg = { (rt_uint32_t)ch, 0, 0 };
    return rt_device_control(s_dev, PWM_CMD_DISABLE, &cfg) == RT_EOK ? 0 : -1;
}
