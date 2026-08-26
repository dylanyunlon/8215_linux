/** 极简 PWM HAL -- Linux sysfs 后端（板级回调优先）
 *
 * 默认走标准 sysfs /sys/class/pwm/pwmchipN（period/duty_cycle/enable，
 * 单位 ns）。注意 AC83xx vendor PWM 驱动（drivers/pwm/atc）仅 EXPORT_SYMBOL
 * 内核态 API、无 pwmchip 注册 -- 此类板须由 BSP 在用户态实现桥接并
 * hal_pwm_register_board() 注册。
 */
#include "hal_pwm.h"
#include <stdio.h>

#define PWM_SYSFS_ROOT "/sys/class/pwm"
#define PWM_MAX_CH 8

static const hal_pwm_ops_t* s_board_ops = NULL;

/** 逻辑通道 -> pwmchipN/硬件通道 映射（默认 chip0、channel=ch） */
static struct {
    int chip;
    int channel;
} s_map[PWM_MAX_CH] = {
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7},
};

/** 每通道缓存上次 period，供 hal_pwm_set_duty 只改占空比 */
static uint32_t s_period_ns[PWM_MAX_CH];

void hal_pwm_register_board(const hal_pwm_ops_t* ops) { s_board_ops = ops; }

/** 板级指定逻辑通道对应的 pwmchip 号与硬件通道号（覆盖默认映射） */
void hal_pwm_linux_set_map(pwm_t ch, int chip, int channel) {
    if (ch < 0 || ch >= PWM_MAX_CH) return;
    s_map[ch].chip = chip;
    s_map[ch].channel = channel;
}

static int wuint(const char* path, unsigned int v) {
    FILE* f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "%u", v);
    fclose(f);
    return 0;
}

/** export 通道（已 export 时内核返回出错，验证节点存在即可） */
static int pwm_export(int chip, int channel) {
    char path[64];
    snprintf(path, sizeof(path), "%s/pwmchip%d/export", PWM_SYSFS_ROOT, chip);
    if (wuint(path, (unsigned int)channel) != 0) {
        snprintf(path, sizeof(path), "%s/pwmchip%d/pwm%d", PWM_SYSFS_ROOT, chip,
                 channel);
        FILE* f = fopen(path, "r");
        if (!f) return -1;
        fclose(f);
    }
    return 0;
}

static int pwm_wleaf(int chip, int channel, const char* leaf, unsigned int v) {
    char path[80];
    snprintf(path, sizeof(path), "%s/pwmchip%d/pwm%d/%s", PWM_SYSFS_ROOT, chip,
             channel, leaf);
    return wuint(path, v);
}

int hal_pwm_config(pwm_t ch, uint32_t duty_ns, uint32_t period_ns) {
    if (ch < 0 || ch >= PWM_MAX_CH) return -1;
    if (period_ns == 0 || duty_ns > period_ns) return -1;
    s_period_ns[ch] = period_ns;
    if (s_board_ops && s_board_ops->config)
        return s_board_ops->config(ch, duty_ns, period_ns);

    if (pwm_export(s_map[ch].chip, s_map[ch].channel) != 0) return -1;
    /* 先归零 duty 再写 period：内核要求 duty<=period，缩小周期前须先降占空比 */
    if (pwm_wleaf(s_map[ch].chip, s_map[ch].channel, "duty_cycle", 0) != 0)
        return -1;
    if (pwm_wleaf(s_map[ch].chip, s_map[ch].channel, "period", period_ns) != 0)
        return -1;
    return pwm_wleaf(s_map[ch].chip, s_map[ch].channel, "duty_cycle", duty_ns);
}

int hal_pwm_set_duty(pwm_t ch, uint32_t duty_ns) {
    if (ch < 0 || ch >= PWM_MAX_CH) return -1;
    if (s_period_ns[ch] == 0 || duty_ns > s_period_ns[ch]) return -1;
    if (s_board_ops && s_board_ops->config)
        return s_board_ops->config(ch, duty_ns, s_period_ns[ch]);
    return pwm_wleaf(s_map[ch].chip, s_map[ch].channel, "duty_cycle", duty_ns);
}

int hal_pwm_enable(pwm_t ch) {
    if (ch < 0 || ch >= PWM_MAX_CH) return -1;
    if (s_board_ops && s_board_ops->enable) return s_board_ops->enable(ch, 1);
    if (pwm_export(s_map[ch].chip, s_map[ch].channel) != 0) return -1;
    return pwm_wleaf(s_map[ch].chip, s_map[ch].channel, "enable", 1);
}

int hal_pwm_disable(pwm_t ch) {
    if (ch < 0 || ch >= PWM_MAX_CH) return -1;
    if (s_board_ops && s_board_ops->enable) return s_board_ops->enable(ch, 0);
    return pwm_wleaf(s_map[ch].chip, s_map[ch].channel, "enable", 0);
}
