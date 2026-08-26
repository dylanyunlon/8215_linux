/**
 * @file mw_can_rx.c
 * @brief CAN 接收模块 -- 报文解析 + 超时清零 + 通信状态判定
 *
 * 改造说明（原 FreeRTOS 版 -> OSAL + HAL 三系统同源，业务逻辑不变）：
 *   - xCanOpen/vCanInit/iCanRead  -> hal_can_open / hal_can_recv
 *   - xTaskCreate/vTaskDelay      -> osal_thread_create / osal_delay_ms
 *   - xTaskGetTickCount           -> osal_tick_ms（ms 单调时钟）
 *   - hal_gpio_set_output(CAN_STB_GPIO) -> gpio_write(gpio_get(逻辑名))
 *   - CanMsg                      -> struct hal_can_frame
 *   - hcn_log                     -> mw_log
 *   - 0x776~0x779 里程/清 EEPROM 报文依赖原工程 storage_param2 与
 *     uart 下发命令模块（未迁入），由 MW_CAN_MILE_MSG_ENABLE 裁剪，
 *     默认关闭；报文定义与解析逻辑保持原样，迁入后打开即用。
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "mw_log.h"
#include "osal.h"
#include "hal.h"
#include "dev_config.h"
#include "mw_can.h"
#include "mw_can_tx.h"
#include "vehicle_param.h"
#include "dev_state.h"
#if MW_CAN_MILE_MSG_ENABLE
#include "set_param.h"
#endif

#define CAN_RX_TIMEOUT_INTERVAL_MS (1000)  ///< 单帧超时清零阈值
#define CAN_RX_COMM_LOST_MS (3000)         ///< 通信丢失判定阈值
#define CAN_RX_BATCH (8)                   ///< 单次批量收帧上限
#define CAN_RX_POLL_MS (10)                ///< 无帧时轮询等待
#define GET_DATA_BITS(data, offset, mask) ((data >> offset) & mask)

/** 各周期报文最近接收时刻（ms）；超时未刷新则清零对应车辆数据 */
typedef struct {
    uint32_t ecu_110_rx_ms;
    uint32_t ecu_111_rx_ms;
    uint32_t ecu_112_rx_ms;
    uint32_t ecu_120_rx_ms;
    uint32_t ecu_122_rx_ms;
    uint32_t ecu_12b_rx_ms;
} can_rx_timeout_t;

static bool is_can_com = false;
static can_rx_timeout_t rx_timeout;

/** 溢出安全的时间比较：now - last > interval（uint32 回绕安全） */
static bool time_elapsed(uint32_t now, uint32_t last, uint32_t interval) {
    return (uint32_t)(now - last) > interval;
}

static void timeout_clear_can_data(void) {
    uint32_t now = (uint32_t)osal_tick_ms();

    if (time_elapsed(now, rx_timeout.ecu_110_rx_ms,
                     CAN_RX_TIMEOUT_INTERVAL_MS)) {
        if (vehicle_get_data(VEH_SPEED_ENGINE) > 0) {
            vehicle_set_data(VEH_SPEED_ENGINE, 0);
        }
    }

    if (time_elapsed(now, rx_timeout.ecu_111_rx_ms,
                     CAN_RX_TIMEOUT_INTERVAL_MS)) {
        if (vehicle_get_data(VEH_DRIVE_MODE) > 0) {
            vehicle_set_data(VEH_DRIVE_MODE, 0);
        }

        if (vehicle_get_data(VEH_LIGHT_ENGINE_FAULT) > 0) {
            vehicle_set_data(VEH_LIGHT_ENGINE_FAULT, 0);
        }
    }

    if (time_elapsed(now, rx_timeout.ecu_112_rx_ms,
                     CAN_RX_TIMEOUT_INTERVAL_MS)) {
        if (vehicle_get_data(VEH_LIGHT_BRAKE) > 0) {
            vehicle_set_data(VEH_LIGHT_BRAKE, 0);
        }

        if (vehicle_get_data(VEH_LIGHT_READY) > 0) {
            vehicle_set_data(VEH_LIGHT_READY, 0);
        }

        if (vehicle_get_data(VEH_LIGHT_GPS) > 0) {
            vehicle_set_data(VEH_LIGHT_GPS, 0);
        }

        if (vehicle_get_data(VEH_TRAM_POWR) > 0) {
            vehicle_set_data(VEH_TRAM_POWR, 0);
        }

        if (vehicle_get_data(VEH_TRAM_REMAIN_BATTARY) > 0) {
            vehicle_set_data(VEH_TRAM_REMAIN_BATTARY, 0);
        }

        if (vehicle_get_data(VEH_GEAR_POSITION) > 0) {
            vehicle_set_data(VEH_GEAR_POSITION, 0);
        }
    }

    if (time_elapsed(now, rx_timeout.ecu_120_rx_ms,
                     CAN_RX_TIMEOUT_INTERVAL_MS)) {
        if (vehicle_get_data(VEH_TCS_WARNING) > 0) {
            vehicle_set_data(VEH_TCS_WARNING, 0);
        }
    }

    if (time_elapsed(now, rx_timeout.ecu_122_rx_ms,
                     CAN_RX_TIMEOUT_INTERVAL_MS)) {
        if (vehicle_get_data(VEH_SPEED_CURRENT) > 0) {
            vehicle_set_data(VEH_SPEED_CURRENT, 0);
        }
    }

    if (time_elapsed(now, rx_timeout.ecu_12b_rx_ms,
                     CAN_RX_TIMEOUT_INTERVAL_MS)) {
        /* ABS 灯与其它信号相反：0x12b 超时要点亮（无通信默认告警） */
        if (vehicle_get_data(VEH_LIGHT_ABS) == 0) {
            vehicle_set_data(VEH_LIGHT_ABS, 1);
        }
    }
}

static int parse_can_msg_110(const uint8_t* buf, uint8_t size) {
    if (get_check_self_state() < CHECK_SELF_STATE_START) {
        return 0;
    }

    if ((buf == NULL) || (size < 8)) {
        mw_log_error("can_ecu_110 dlc[%d] or msg buff err!\r\n", size);
        return -1;
    }

    rx_timeout.ecu_110_rx_ms = (uint32_t)osal_tick_ms();

    int data = ((buf[2] << 8) + (buf[3]));
    if (data == 0xffff) {
        data = 0;
    }

    float engine_speed_tmp = data * 0.25;
    data = (int)engine_speed_tmp;
    if (data > 0 && data <= 1700) {
        data = 1700;
    }

    vehicle_set_data(VEH_SPEED_ENGINE, data);

    return 0;
}

static int parse_can_msg_111(const uint8_t* buf, uint8_t size) {
    if (get_check_self_state() < CHECK_SELF_STATE_START) {
        return 0;
    }

    if ((buf == NULL) || (size < 8)) {
        mw_log_error("can_ecu_111 dlc[%d] or msg buff err!\r\n", size);
        return -1;
    }

    rx_timeout.ecu_111_rx_ms = (uint32_t)osal_tick_ms();

    int data = GET_DATA_BITS(buf[0], 1, 0x01);
    vehicle_set_data(VEH_LIGHT_ENGINE_FAULT, data);

    data = GET_DATA_BITS(buf[0], 2, 0x07);
    vehicle_set_data(VEH_DRIVE_MODE, data);

    return 0;
}

static int parse_can_msg_112(const uint8_t* buf, uint8_t size) {
    if (get_check_self_state() < CHECK_SELF_STATE_START) {
        return 0;
    }

    if ((buf == NULL) || (size < 8)) {
        mw_log_error("can_ecu_112 dlc[%d] or msg buff err!\r\n", size);
        return -1;
    }

    rx_timeout.ecu_112_rx_ms = (uint32_t)osal_tick_ms();

    int data = GET_DATA_BITS(buf[0], 0, 0x01);
    vehicle_set_data(VEH_LIGHT_BRAKE, data);

    data = GET_DATA_BITS(buf[0], 1, 0x1);
    vehicle_set_data(VEH_LIGHT_READY, data);

    data = GET_DATA_BITS(buf[0], 2, 0x1);
    vehicle_set_data(VEH_LIGHT_GPS, data);

    data = GET_DATA_BITS(buf[2], 0, 0x07);
    vehicle_set_data(VEH_GEAR_POSITION, data);

    float power = 0;
    data = ((buf[5] << 8) + buf[6]);
    if (data == 0xFFFF) {
        data = 0;
    }

    power = data * 0.01;
    data = (int)power;
    if (data > 100) {
        data = 100;
    }
    vehicle_set_data(VEH_TRAM_POWR, data);

    data = buf[7];
    if (data == 0xFF) {
        data = 0;
    }
    vehicle_set_data(VEH_TRAM_REMAIN_BATTARY, data);

    return 0;
}

static int parse_can_msg_120(const uint8_t* buf, uint8_t size) {
    if (get_check_self_state() < CHECK_SELF_STATE_START) {
        return 0;
    }

    if ((buf == NULL) || (size < 8)) {
        mw_log_error("can_ecu_120 dlc[%d] or msg buff err!\r\n", size);
        return -1;
    }

    rx_timeout.ecu_120_rx_ms = (uint32_t)osal_tick_ms();

    int data = GET_DATA_BITS(buf[3], 2, 0x01);
    vehicle_set_data(VEH_TCS_WARNING, data);

    return 0;
}

static int parse_can_msg_122(const uint8_t* buf, uint8_t size) {
    if (get_check_self_state() < CHECK_SELF_STATE_START) {
        return 0;
    }

    if ((buf == NULL) || (size < 8)) {
        mw_log_error("can_ecu_122 dlc[%d] or msg buff err!\r\n", size);
        return -1;
    }

    rx_timeout.ecu_122_rx_ms = (uint32_t)osal_tick_ms();
    int data = 0;
    float speed_tmp = 0;

    data = ((buf[2] << 8) + buf[3]);
    if (data == 0xFFFF) {
        data = 0;
    }

    speed_tmp = data * 0.01;
    data = (int)speed_tmp;
    if (data > 199) {
        data = 199;
    }

    vehicle_set_data(VEH_SPEED_CURRENT, data);

    return 0;
}

static int parse_can_msg_12b(const uint8_t* buf, uint8_t size) {
    if (get_check_self_state() < CHECK_SELF_STATE_START) {
        return 0;
    }

    if ((buf == NULL) || (size < 8)) {
        mw_log_error("can_ecu_12b dlc[%d] or msg buff err!\r\n", size);
        return -1;
    }

    rx_timeout.ecu_12b_rx_ms = (uint32_t)osal_tick_ms();

    int data = GET_DATA_BITS(buf[5], 0, 0x01);
    vehicle_set_data(VEH_LIGHT_ABS, data);

    return 0;
}

#if MW_CAN_MILE_MSG_ENABLE
/* ---------- 里程/清 EEPROM 报文（0x776~0x779，原工程模块依赖） ---------- */

#define CRC8_MAGIC_NUM (0x1D)

/** J1850 CRC8（多项式 0x1D，初值/终值 0xFF，校验前 7 字节） */
static uint8_t can_j1850_crc8(const uint8_t* data, uint8_t length) {
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ CRC8_MAGIC_NUM;
            } else {
                crc <<= 1;
            }
        }
        crc &= 0xFF;
    }
    crc ^= 0xFF;
    return crc;
}

/** 三帧一致确认状态（每个报文 ID 独立一份，防交叉复位） */
typedef struct {
    uint8_t cnt;
    uint8_t buf[8];
} can_msg_confirm_t;

/** 连续 3 帧完全相同返回 true（去抖，防单帧误码）；状态不匹配则重计 */
static bool can_msg_triple_confirm(can_msg_confirm_t* st,
                                   const uint8_t* buf) {
    if (st->cnt == 0) {
        memcpy(st->buf, buf, 8);
        st->cnt = 1;
    } else if (memcmp(st->buf, buf, 8) == 0) {
        st->cnt++;
    } else {
        memcpy(st->buf, buf, 8);
        st->cnt = 1;
    }

    if (st->cnt >= 3) {
        st->cnt = 0;
        return true;
    }
    return false;
}

/** 帧头公共校验：自检状态 + 长度 + CRC8（失败打日志并返回 -1） */
static int mile_msg_check(const uint8_t* buf, uint8_t size, uint32_t id) {
    if (get_check_self_state() < CHECK_SELF_STATE_SUCCESS) {
        return -1;
    }

    if ((buf == NULL) || (size < 8)) {
        mw_log_error("can_ecu_%x dlc[%d] or msg buff err!\r\n", id, size);
        return -1;
    }

    uint8_t crc = can_j1850_crc8(buf, 7);
    if (crc != buf[7]) {
        mw_log_error("can_ecu_%x checksum err: 0x%x != 0x%x!\r\n", id, crc,
                     buf[7]);
        return -1;
    }
    return 0;
}

static int parse_can_msg_776(const uint8_t* buf, uint8_t size) {
    static can_msg_confirm_t confirm = {0};

    if (mile_msg_check(buf, size, 0x776) != 0) {
        return -1;
    }
    if (!can_msg_triple_confirm(&confirm, buf)) {
        return 0;
    }

    uint32_t trip_a;
    uint32_t tmp = 0;

    if (!get_hcn_mile_param(HCN_MILE_PARAM_TRIP_A, &tmp)) {
        mw_log_error("Get HCN_MILE_PARAM_TRIP_A error!\r\n");
        return -1;
    }

    trip_a = (((uint32_t)buf[0]) << 24) | (((uint32_t)buf[1]) << 16) |
             (((uint32_t)buf[2]) << 8) | (((uint32_t)buf[3]) << 0);
    if (trip_a != tmp) {
        set_hcn_mile_param(HCN_MILE_PARAM_TRIP_A, &trip_a);
        vehicle_set_data(VEH_MILEAGE_CHANGE_MSG, 1);
    }

    return 0;
}

static int parse_can_msg_777(const uint8_t* buf, uint8_t size) {
    static can_msg_confirm_t confirm = {0};

    if (mile_msg_check(buf, size, 0x777) != 0) {
        return -1;
    }
    if (!can_msg_triple_confirm(&confirm, buf)) {
        return 0;
    }

    uint32_t trip_b;
    uint32_t tmp = 0;

    if (!get_hcn_mile_param(HCN_MILE_PARAM_TRIP_B, &tmp)) {
        mw_log_error("Get HCN_MILE_PARAM_TRIP_B error!\r\n");
        return -1;
    }

    trip_b = (((uint32_t)buf[0]) << 24) | (((uint32_t)buf[1]) << 16) |
             (((uint32_t)buf[2]) << 8) | (((uint32_t)buf[3]) << 0);
    if (trip_b != tmp) {
        set_hcn_mile_param(HCN_MILE_PARAM_TRIP_B, &trip_b);
        vehicle_set_data(VEH_MILEAGE_CHANGE_MSG, 2);
    }

    return 0;
}

static int parse_can_msg_778(const uint8_t* buf, uint8_t size) {
    static can_msg_confirm_t confirm = {0};

    if (mile_msg_check(buf, size, 0x778) != 0) {
        return -1;
    }
    if (!can_msg_triple_confirm(&confirm, buf)) {
        return 0;
    }

    uint32_t odo;
    uint32_t tmp = 0;

    if (!get_hcn_mile_param(HCN_MILE_PARAM_ODO, &tmp)) {
        mw_log_error("Get HCN_MILE_PARAM_ODO error!\r\n");
        return -1;
    }

    odo = (((uint32_t)buf[0]) << 24) | (((uint32_t)buf[1]) << 16) |
          (((uint32_t)buf[2]) << 8) | (((uint32_t)buf[3]) << 0);
    if (odo != tmp) {
        set_hcn_mile_param(HCN_MILE_PARAM_ODO, &odo);
        vehicle_set_data(VEH_MILEAGE_CHANGE_MSG, 3);
    }

    return 0;
}

static int parse_can_msg_779(const uint8_t* buf, uint8_t size) {
    static can_msg_confirm_t confirm = {0};

    if (mile_msg_check(buf, size, 0x779) != 0) {
        return -1;
    }

    /* 魔数命中即清保养存储并命令 MCU 复位（系统随后重启）。
     * 注: 原版此处确认计数不复位, 每帧重复触发; 现复用三帧确认,
     * 每 3 帧重触发一次, 语义等价(靠 1s 延时 + MCU 复位收敛)。 */
    if (can_msg_triple_confirm(&confirm, buf)) {
        if ((buf[0] == 0xa1) && (buf[1] == 0xb1) && (buf[2] == 0xc1) &&
            (buf[3] == 0xd1) && (buf[4] == 0xe1) && (buf[5] == 0xf1) &&
            (buf[6] == 0xee)) {
            extern void send_mcu_clear_eeprom(void);
            clean_eeprom_operate();
            osal_delay_ms(1000);
            send_mcu_clear_eeprom();
            mw_log_info("clear eeprom, os will reboot...\r\n");
        }
    }

    return 0;
}
#endif /* MW_CAN_MILE_MSG_ENABLE */

bool can_get_communication_status(void) { return is_can_com; }

static void can_recv_msg_process(const struct hal_can_frame* frame) {
    if (!frame) {
        return;
    }

    mw_log_debug("can recv id:0x%x dlc:%d\n", frame->id, frame->dlc);

    switch (frame->id) {
        case 0x110:
            parse_can_msg_110(frame->data, frame->dlc);
            break;

        case 0x111:
            parse_can_msg_111(frame->data, frame->dlc);
            break;

        case 0x112:
            parse_can_msg_112(frame->data, frame->dlc);
            break;

        case 0x120:
            parse_can_msg_120(frame->data, frame->dlc);
            break;

        case 0x122:
            parse_can_msg_122(frame->data, frame->dlc);
            break;

        case 0x12b:
            parse_can_msg_12b(frame->data, frame->dlc);
            break;

#if MW_CAN_MILE_MSG_ENABLE
        case 0x776:
            parse_can_msg_776(frame->data, frame->dlc);
            break;

        case 0x777:
            parse_can_msg_777(frame->data, frame->dlc);
            break;

        case 0x778:
            parse_can_msg_778(frame->data, frame->dlc);
            break;

        case 0x779:
            parse_can_msg_779(frame->data, frame->dlc);
            break;
#endif

        default:
            break;
    }
}

static void* can_rx_thread(void* arg) {
    hal_can_t* can = (hal_can_t*)arg;
    struct hal_can_frame frames[CAN_RX_BATCH];
    uint64_t last_rx_ms = 0;

    for (;;) {
        int recv_len = hal_can_recv(can, frames, CAN_RX_BATCH, CAN_RX_POLL_MS);
        if (recv_len > 0) {
            last_rx_ms = osal_tick_ms();
            for (int i = 0; i < recv_len; i++) {
                can_recv_msg_process(&frames[i]);
            }
        }

        timeout_clear_can_data();

        is_can_com = (osal_tick_ms() - last_rx_ms) <= CAN_RX_COMM_LOST_MS;
    }
    return NULL;
}

int can_module_init(void) {
    hal_can_t* can = hal_can_open(MW_CAN_DEV_NAME);
    if (!can) {
        mw_log_error("open can %s failed!\n", MW_CAN_DEV_NAME);
        return -1;
    }

    /* 收发器 STB 引脚拉低退出待机（原版 hal_gpio_set_output 语义） */
#if MW_CAN_STB_ENABLE
    gpio_t stb = gpio_get(MW_CAN_STB_PIN);
    if (stb == GPIO_INVALID) {
        mw_log_warn("can stb pin %s not found\n", MW_CAN_STB_PIN);
    } else {
        gpio_write(stb, 0);
    }
#endif

    osal_delay_ms(2);

    /* ABS 灯无通信默认点亮；0x12b 收到后熄灭，超时兜底再点亮 */
    vehicle_set_data(VEH_LIGHT_ABS, 1);

    if (osal_thread_create(NULL, "can_rx", can_rx_thread, can, 0, 0) !=
        OSAL_OK) {
        mw_log_error("create can rx task fail.\n");
        return -1;
    }

    if (mw_can_tx_start(can) != 0) {
        mw_log_error("can tx start fail.\n");
        return -1;
    }

    mw_log_info("can module init ok! (dev:%s baud:%d)\n", MW_CAN_DEV_NAME,
                MW_CAN_BAUD);

    return 0;
}
