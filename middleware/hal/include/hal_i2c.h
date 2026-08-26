#ifndef __HAL_I2C_H__
#define __HAL_I2C_H__

/**
 * @file hal_i2c.h
 * @brief 极简 I2C HAL -- 句柄 + msg 事务模型（参照 Linux i2c-core）
 *
 * 模型（与 Linux i2c-core / RT-Thread rt_i2c / i2c_ref.c 同构）：
 *   - hal_i2c_open(name)   打开总线拿句柄；name 为板级总线名
 *     （如 "i2c0"：Linux 映射 /dev/i2c-0，RT-Thread 按设备名 find，
 *     FreeRTOS 由板级回调解释）。存在性在首次 transfer 校验。
 *   - hal_i2c_transfer()   msg 数组事务原语：一个 msg = 一段
 *     addr+方向+长度+缓冲；多 msg 一次提交，msg 间自动 repeated-start
 *   - 寄存器便捷入口在头文件内联组 msg，日常读写寄存器一行搞定
 *
 * 地址约定：addr 为 7bit 从机地址（不含 R/W 位）。
 *
 * 后端策略（板级回调优先，同 hal_key/hal_pwm 模式）：
 *   - Linux     : /dev/i2c-N + I2C_RDWR ioctl（当前 ac83xx defconfig
 *                 未启 I2C，此类板须 BSP 桥接并注册板级操作表）
 *   - RT-Thread : RT_USING_I2C 时兜底 rt_i2c 框架，否则仅板级回调
 *   - FreeRTOS  : 仅板级操作表
 *
 * 与 OSAL 解耦：本层只依赖 <stdint.h>，不包含任何 OS 头文件。
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define HAL_I2C_RD 0x0001    ///< 读方向（位值同 I2C_M_RD / RT_I2C_RD）
#define HAL_I2C_MSG_MAX 8    ///< 单次事务最大 msg 数（后端栈数组上限）
#define HAL_I2C_REG_MAX 64   ///< write_regs 单次最大数据长度（组包上限）

/** 一个 msg = 一段传输：addr + 方向 + 长度 + 缓冲（布局同 i2c_ref.c） */
struct hal_i2c_msg {
    uint16_t addr;  ///< 7bit 从机地址
    uint16_t flags; ///< 方向等标志（HAL_I2C_RD）
    uint16_t len;   ///< 数据长度
    uint8_t* buf;   ///< 数据缓冲
};

/** 总线句柄（hal_i2c_open 获取，hal_i2c_close 释放；内容后端私有） */
typedef struct hal_i2c_bus hal_i2c_bus_t;

/** 板级 I2C 操作表：实现 msg 数组事务（返回 0 成功） */
typedef struct {
    int (*transfer)(hal_i2c_bus_t* bus, struct hal_i2c_msg* msgs, int num);
} hal_i2c_ops_t;

/** 打开总线。name 如 "i2c0"（Linux 也接受 "i2c-1"/"1"/"/dev/i2c-1"）；
 *  失败返回 NULL。 */
hal_i2c_bus_t* hal_i2c_open(const char* name);

/** 关闭总线并释放句柄（传 NULL 安全） */
void hal_i2c_close(hal_i2c_bus_t* bus);

/** msg 数组事务原语：num 个 msg 一次提交，msg 间 repeated-start */
int hal_i2c_transfer(hal_i2c_bus_t* bus, struct hal_i2c_msg* msgs, int num);

/** 取句柄对应的总线名（板级回调区分总线用） */
const char* hal_i2c_bus_name(const hal_i2c_bus_t* bus);

/** 注册板级操作表（覆盖后端默认实现；传 NULL 恢复默认） */
void hal_i2c_register_board(const hal_i2c_ops_t* ops);

/* ---------- 寄存器便捷入口（内联组 msg，收口到 transfer） ---------- */

/** 写单寄存器 */
static inline int hal_i2c_write_reg(hal_i2c_bus_t* bus, uint8_t addr,
                                    uint8_t reg, uint8_t val) {
    uint8_t data[2] = {reg, val};
    struct hal_i2c_msg m = {addr, 0, 2, data};
    return hal_i2c_transfer(bus, &m, 1);
}

/** 读单寄存器（写寄存器号 -> repeated-start -> 读 1 字节） */
static inline int hal_i2c_read_reg(hal_i2c_bus_t* bus, uint8_t addr,
                                   uint8_t reg, uint8_t* val) {
    struct hal_i2c_msg m[2] = {
        {addr, 0, 1, &reg},
        {addr, HAL_I2C_RD, 1, val},
    };
    return hal_i2c_transfer(bus, m, 2);
}

/** 连续读寄存器（reg 起 len 字节） */
static inline int hal_i2c_read_regs(hal_i2c_bus_t* bus, uint8_t addr,
                                    uint8_t reg, uint8_t* buf, int len) {
    struct hal_i2c_msg m[2] = {
        {addr, 0, 1, &reg},
        {addr, HAL_I2C_RD, (uint16_t)len, buf},
    };
    return hal_i2c_transfer(bus, m, 2);
}

/** 连续写寄存器（reg 起 len 字节；I2C 写需 reg+数据连续，内部栈组包，
 *  len 须 <= HAL_I2C_REG_MAX） */
static inline int hal_i2c_write_regs(hal_i2c_bus_t* bus, uint8_t addr,
                                     uint8_t reg, const uint8_t* buf,
                                     int len) {
    if (len <= 0 || len > HAL_I2C_REG_MAX) return -1;
    uint8_t tmp[HAL_I2C_REG_MAX + 1];
    tmp[0] = reg;
    for (int i = 0; i < len; i++) tmp[i + 1] = buf[i];
    struct hal_i2c_msg m = {addr, 0, (uint16_t)(len + 1), tmp};
    return hal_i2c_transfer(bus, &m, 1);
}

#ifdef __cplusplus
}
#endif
#endif /* __HAL_I2C_H__ */
