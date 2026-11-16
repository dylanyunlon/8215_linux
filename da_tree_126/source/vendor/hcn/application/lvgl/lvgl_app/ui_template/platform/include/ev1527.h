/*
 * File   : ev1527.h
 * Brief  : 433.92MHz EV1527 wireless key receiver — RT-Thread version
 */
#ifndef __EV1527_H__
#define __EV1527_H__

#include <rtthread.h>
#include <stdbool.h>

/* 事件类型 */
#define EV1527_EVT_PRESS        0x01    /* 第一次按下（去抖通过）         */
#define EV1527_EVT_REPEAT       0x02    /* 持续按住时每帧上抛             */
#define EV1527_EVT_LONG_PRESS   0x03    /* 持续 >= 长按阈值，仅触发一次   */
#define EV1527_EVT_RELEASE      0x04    /* 松开                            */

/* 业务侧拿到的事件包 */
typedef struct
{
    rt_uint32_t addr;       /* 高 20 bit：芯片唯一地址 */
    rt_uint8_t  key;        /* 低  4 bit：按键编码     */
    rt_uint8_t  event;      /* EV1527_EVT_*            */
    rt_tick_t   tick;       /* 事件发生时刻            */
} ev1527_frame_t;

/* 初始化：自动注册到 INIT_APP_EXPORT */
int      ev1527_init(void);

/* 阻塞读一个事件；timeout 单位为 tick，RT_WAITING_FOREVER 永久等待 */
rt_err_t ev1527_recv(ev1527_frame_t *frame, rt_int32_t timeout);

uint8_t get_wireless_key_state(void);

uint8_t get_wireless_key_count(void);

int get_wireless_key_addr(uint8_t index, uint32_t *addr);

void start_key_learn(void);

void stop_key_learn(void);

void delete_key(uint32_t addr);

/* 433 IRQ 总开关. 关机时 (IGN_OFF / NFC_OFF 主动关机路径) 被外部调到 false,
 * 防止 sleep 后被遥控器误唤醒. WIRELESS_OFF 不调它 — 433 自己仍要继续监听. */
void ev1527_irq_enable(bool enable);

#endif /* __EV1527_H__ */