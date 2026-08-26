#ifndef __HAL_CAN_H__
#define __HAL_CAN_H__

/**
 * @file hal_can.h
 * @brief 极简 CAN HAL -- 句柄 + 帧收发模型（参照 Linux SocketCAN）
 *
 * 模型（与 SocketCAN / RT-Thread can 设备同构）：
 *   - hal_can_open(name)   打开 CAN 通道拿句柄；name 为板级通道名
 *     （如 "can0"：Linux 映射 SocketCAN 网络接口，RT-Thread/FreeRTOS
 *     由板级操作表解释）。波特率/模式属链路层配置，由板级或内核侧
 *     负责（Linux 用 iproute2），本层不重复造轮子。
 *   - hal_can_send/recv()  帧数组批量收发；timeout_ms 语义同 OSAL：
 *     OSAL_WAIT_FOREVER(0xFFFFFFFF)=永久阻塞，0=不等待，其它=毫秒数。
 *     recv 返回实际收到帧数（0=超时无帧）；send 全部完成返回 0。
 *   - hal_can_set_filter() ID 白名单（11bit 标准帧精确匹配；
 *     未调用=接收所有帧）。
 *
 * 帧模型 struct hal_can_frame：id 不含标志位（标准帧 11bit /
 * 扩展帧 29bit），flags 表扩展/远程帧，dlc 0-8，data 定长 8 字节。
 *
 * 后端策略（板级回调优先，同 hal_i2c 模式）：
 *   - Linux     : SocketCAN (AF_CAN/SOCK_RAW/CAN_RAW)，socket 懒创建
 *                 自愈（can0 未 up 不阻塞 init，接口就绪后自动恢复）
 *   - RT-Thread : 板级操作表（BSP 桥接 rt_device can 框架）
 *   - FreeRTOS  : 板级操作表（BSP 桥接 xCanOpen/iCanRead 等 SDK 驱动）
 *
 * 与 OSAL 解耦：本层只依赖 <stdint.h>，不包含任何 OS 头文件。
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define HAL_CAN_DATA_LEN 8 ///< CAN 单帧最大数据长度（经典 CAN）

/** 帧标志（struct hal_can_frame::flags） */
#define HAL_CAN_FRAME_EXT 0x01 ///< 扩展帧（29bit ID）
#define HAL_CAN_FRAME_RTR 0x02 ///< 远程帧

/** 一帧：id + 标志 + 长度 + 数据（语义对齐 SocketCAN can_frame） */
struct hal_can_frame {
    uint32_t id;   ///< 帧 ID（标准帧 11bit / 扩展帧 29bit，不含标志位）
    uint8_t flags; ///< HAL_CAN_FRAME_EXT / HAL_CAN_FRAME_RTR
    uint8_t dlc;   ///< 数据长度 0-8
    uint8_t data[HAL_CAN_DATA_LEN]; ///< 数据（前 dlc 字节有效）
};

/** CAN 通道句柄（hal_can_open 获取，hal_can_close 释放；内容后端私有） */
typedef struct hal_can hal_can_t;

/** 板级 CAN 操作表（send 返回 0 成功；recv 返回帧数，超时返回 0） */
typedef struct {
    int (*send)(hal_can_t* can, const struct hal_can_frame* frames, int num,
                uint32_t timeout_ms);
    int (*recv)(hal_can_t* can, struct hal_can_frame* frames, int num,
                uint32_t timeout_ms);
    int (*set_filter)(hal_can_t* can, const uint32_t* ids, int num);
} hal_can_ops_t;

/** 打开 CAN 通道。name 如 "can0"；失败返回 NULL */
hal_can_t* hal_can_open(const char* name);

/** 关闭通道并释放句柄（传 NULL 安全） */
void hal_can_close(hal_can_t* can);

/** 取句柄对应的通道名（板级回调区分通道用） */
const char* hal_can_name(const hal_can_t* can);

/** 批量发送：num 帧一次提交，全部完成返回 0，否则 -1 */
int hal_can_send(hal_can_t* can, const struct hal_can_frame* frames, int num,
                 uint32_t timeout_ms);

/** 批量接收：最多收 num 帧填入 frames，返回实际收到帧数（0=超时无帧）。
 *  第一帧按 timeout_ms 等待，后续帧非阻塞排空同批连帧。 */
int hal_can_recv(hal_can_t* can, struct hal_can_frame* frames, int num,
                 uint32_t timeout_ms);

/** 设置接收 ID 白名单（ids 数组按 11bit 标准帧精确匹配；
 *  ids=NULL/num<=0 时不改变当前过滤） */
int hal_can_set_filter(hal_can_t* can, const uint32_t* ids, int num);

/** 注册板级操作表（覆盖后端默认实现；传 NULL 恢复默认） */
void hal_can_register_board(const hal_can_ops_t* ops);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_CAN_H__ */
