#ifndef __MW_EASY_NAVI_H__
#define __MW_EASY_NAVI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "dev_config.h"

// 导航类型
typedef enum {
    NAV_TYPE_WALK = 0,          // 步行
    NAV_TYPE_BIKE = 1,          // 骑自行车
    NAV_TYPE_EBIKE = 2,         // 骑电动车
    NAV_TYPE_UNKNOWN = 0xFF
} navigation_type_t;

// 红绿灯状态
typedef enum {
    TRAFFIC_LIGHT_INVALID = 0,  // 无效
    TRAFFIC_LIGHT_OFF = 11,     // 灭灯
    TRAFFIC_LIGHT_RED = 21,     // 红灯
    TRAFFIC_LIGHT_YELLOW = 22,  // 黄灯
    TRAFFIC_LIGHT_GREEN = 23    // 绿灯
} traffic_light_state_t;

// GPS信号状态
typedef enum {
    GPS_SIGNAL_NORMAL = 0,      // 信号正常
    GPS_SIGNAL_WEAK = 1         // 信号弱
} gps_signal_t;

// 偏航状态
typedef enum {
    DEVIATION_NONE = 0,         // 正常，没有偏航
    DEVIATION_IN_PROGRESS = 1,  // 正在偏航
    DEVIATION_CANCELLED = 2,    // 偏航失败或主动取消
    DEVIATION_REPLAN_SUCCESS = 3 // 重新规划成功
} deviation_state_t;

typedef enum {
    USR_NAVI_ICON_NONE                          = 0,               ///< 收到此值，不显示导航图标
    USR_NAVI_ICON_DEFAULT                       = 1,               ///< 自车.请忽略这个元素，从左转图标开始
    USR_NAVI_ICON_LEFT                          = 2,               ///< 左转
    USR_NAVI_ICON_RIGHT                         = 3,               ///< 右转
    USR_NAVI_ICON_LEFT_FRONT                    = 4,               ///< 左前方
    USR_NAVI_ICON_RIGHT_FRONT                   = 5,               ///< 右前方
    USR_NAVI_ICON_LEFT_BACK                     = 6,               ///< 左后方
    USR_NAVI_ICON_RIGHT_BACK                    = 7,               ///< 右后方
    USR_NAVI_ICON_LEFT_TURN_AROUND              = 8,               ///< 左转掉头
    USR_NAVI_ICON_STRAIGHT                      = 9,               ///< 直行
    USR_NAVI_ICON_ARRIVED_WAYPOINT              = 10,              ///< 到达途经点
    USR_NAVI_ICON_ENTER_ROUNDABOUT              = 11,              ///< 进入环岛
    USR_NAVI_ICON_OUT_ROUNDABOUT                = 12,              ///< 驶出环岛
    USR_NAVI_ICON_ARRIVED_SERVICE_AREA          = 13,              ///< 到达服务区
    USR_NAVI_ICON_ARRIVED_TOLLGATE              = 14,              ///< 到达收费站
    USR_NAVI_ICON_ARRIVED_DESTINATION           = 15,              ///< 到达目的地
    USR_NAVI_ICON_ARRIVED_TUNNEL                = 16,              ///< 到达隧道
    USR_NAVI_ICON_CROSSWALK                     = 17,              ///< 通过人行横道
    USR_NAVI_ICON_OVERPASS                      = 18,              ///< 通过过街天桥
    USR_NAVI_ICON_UNDERPASS                     = 19,              ///< 通过地下通道
    USR_NAVI_ICON_SQUARE                        = 20,              ///< 通过广场
    USR_NAVI_ICON_PARK                          = 21,              ///< 通过公园
    USR_NAVI_ICON_STAIRCASE                     = 22,              ///< 通过扶梯
    USR_NAVI_ICON_LIFT                          = 23,              ///< 通过直梯
    USR_NAVI_ICON_CABLEWAY                      = 24,              ///< 通过索道
    USR_NAVI_ICON_SKY_CHANNEL                   = 25,              ///< 通过空中通道
    USR_NAVI_ICON_CHANNEL                       = 26,              ///< 通过通道、建筑物穿越通道
    USR_NAVI_ICON_WALK_ROAD                     = 27,              ///< 通过行人道路
    USR_NAVI_ICON_CRUISE_ROUTE                  = 28,              ///< 通过游船路线
    USR_NAVI_ICON_SIGHTSEEING_BUSLINE           = 29,              ///< 通过观光车路线
    USR_NAVI_ICON_SLIDEWAY                      = 30,              ///< 通过滑道
    USR_NAVI_ICON_LADDER                        = 31,              ///< 通过阶梯
    USR_NAVI_ICON_MERGE_LEFT                    = 51,              ///< 靠左行驶
    USR_NAVI_ICON_MERGE_RIGHT                   = 52,              ///< 靠右行驶
    USR_NAVI_ICON_SLOW                          = 53,              ///< 减速慢行
    USR_NAVI_ICON_ENTRY_RING_LEFT               = 54,              ///< 标准小环岛 绕环岛左转，右侧通行地区的逆时针环岛
    USR_NAVI_ICON_ENTRY_RING_RIGHT              = 55,              ///< 标准小环岛 绕环岛右转，右侧通行地区的逆时针环岛
    USR_NAVI_ICON_ENTRY_RING_CONTINUE           = 56,              ///< 标准小环岛 绕环岛直行，右侧通行地区的逆时针环岛
    USR_NAVI_ICON_ENTRY_RING_UTURN              = 57,              ///< 标准小环岛 绕环岛调头，右侧通行地区的逆时针环岛
    USR_NAVI_ICON_ENTRY_LEFT_RING               = 58,              ///< 进入环岛图标，左侧通行地区的顺时针环岛
    USR_NAVI_ICON_LEAVE_LEFT_RING               = 59,              ///< 驶出环岛图标，左侧通行地区的顺时针环岛
    USR_NAVI_ICON_UTURN_RIGHT                   = 60,              ///< 右转掉头图标，左侧通行地区的掉头
    USR_NAVI_ICON_SPECIAL_CONTINUE              = 61,              ///< 顺行图标(和直行有区别，顺行图标带有虚线)
    USR_NAVI_ICON_ENTRY_LEFT_RING_LEFT          = 62,              ///< 标准小环岛 绕环岛左转，左侧通行地区的顺时针环岛
    USR_NAVI_ICON_ENTRY_LEFT_RING_RIGHT         = 63,              ///< 标准小环岛 绕环岛右转，左侧通行地区的顺时针环岛
    USR_NAVI_ICON_ENTRY_LEFT_RING_CONTINUE      = 64,              ///< 标准小环岛 绕环岛直行，左侧通行地区的顺时针环岛
    USR_NAVI_ICON_ENTRY_LEFT_RING_UTURN         = 65,              ///< 标准小环岛 绕环岛调头，左侧通行地区的顺时针环岛
    USR_NAVI_ICON_SLOPE                         = 66,              ///< 通过斜坡图标
    USR_NAVI_ICON_BRIDGE                        = 67,              ///< 通过桥图标
    USR_NAVI_ICON_FERRYBOAT                     = 68,              ///< 通过渡轮图标
    USR_NAVI_ICON_SUBWAY                        = 69,              ///< 通过地铁图标
    USR_NAVI_ICON_ENTER_BUILDING                = 70,              ///< 进入建筑物图标
    USR_NAVI_ICON_LEAVE_BUILDING                = 71,              ///< 离开建筑物图标
    USR_NAVI_ICON_BY_ELEVATOR                   = 72,              ///< 电梯换层图标
    USR_NAVI_ICON_BY_STAIR                      = 73,              ///< 楼梯换层图标
    USR_NAVI_ICON_ESCALATOR                     = 74,              ///< 扶梯换层图标
    USR_NAVI_ICON_LOW_TRAFFIC_CROSS             = 75,              ///< 非导航段通过红绿灯路口图标
    USR_NAVI_ICON_LOW_CROSS                     = 76,              ///< 非导航段通过普通路口图标
    USR_NAVI_ICON_ROTARY_SHARP_LEFT             = 77,              ///< 环岛左后转，右侧通行地区的逆时针环岛
    USR_NAVI_ICON_ROTARY_SHARP_RIGHT            = 78,              ///< 环岛后右转，右侧通行地区的逆时针环岛
    USR_NAVI_ICON_ROTARY_SLIGHT_LEFT            = 79,              ///< 环岛左前转，右侧通行地区的逆时针环岛
    USR_NAVI_ICON_ROTARY_SLIGHT_RIGHT           = 80,              ///< 环岛右前转，右侧通行地区的逆时针环岛
    USR_NAVI_ICON_MAX
} usrNaviIcon;

// 解析后的导航数据结构
typedef struct {
    // 导航信息
    navigation_type_t nav_type;         // 导航类型
    traffic_light_state_t light_state;  // 红绿灯状态
    uint16_t countdown_seconds;         // 红绿灯倒计时(秒) 0=无效
    uint16_t distance_to_light;         // 到红绿灯距离(米) 0=无效
    uint8_t naviIcon;                    ///< 简易导航图标 see usrNaviIcon define
    gps_signal_t gps_signal;            // GPS信号状态
    deviation_state_t deviation;        // 偏航状态
    uint16_t remaining_time_minutes;    // 剩余时间(分钟) 0=无效
    int32_t roadRemainingDistance;          ///< 到下一路口距离(米) 0=无效
    int32_t destinationRemainingDistance;   ///< 剩余里程(米) 0=无效
} easy_navi_data_t;

const easy_navi_data_t *get_easy_navi_info(void);
void set_easy_navi_info(const easy_navi_data_t *data);

#if MW_EASY_NAVI_ENABLE

/* AT指令设置前缀 */
#define AT_CMD_NAME_SET         "AT+NAME="        // 设置设备名称
#define AT_CMD_MAC_SET          "AT+MAC="         // 设置MAC地址
#define AT_CMD_ADVINT_SET       "AT+ADVINT="      // 设置广播间隔
#define AT_CMD_ADV_SET          "AT+ADV="         // 设置广播状态
#define AT_CMD_LINK_SET         "AT+LINK="        // 设置连接
#define AT_CMD_SCANTIME_SET     "AT+SCAN="        // 设置扫描时间
#define AT_CMD_CONN_SET         "AT+CONN="        // 设置连接设备
#define AT_CMD_DISCONN_SET      "AT+DISCONN="     // 设置断开连接
#define AT_CMD_FLASH_SET        "AT+FLASH="       // 设置Flash
#define AT_CMD_SEND_SET         "AT+SEND="        // 设置发送数据
#define AT_CMD_AUTO_SET         "AT+AUTO+++="      // 设置自动+++
#define AT_CMD_POWER_SET        "AT+POWER="       // 设置电源
#define AT_CMD_SLEEP_SET        "AT+SLEEP="       // 设置睡眠
#define AT_CMD_INDUCTOR_SET     "AT+INDUCTOR="    // 设置无感解锁
#define AT_CMD_DIST_SET         "AT+DIST="        // 设置感应范围
#define AT_CMD_MAP_SET          "AT+MAP="         // 设置简易导航

/* AT指令查询前缀 */
#define AT_CMD_NAME_QUERY       "AT+NAME?"        // 查询设备名称
#define AT_CMD_MAC_QUERY        "AT+MAC?"         // 查询MAC地址
#define AT_CMD_ADVINT_QUERY     "AT+ADVINT?"      // 查询广播间隔
#define AT_CMD_ADV_QUERY        "AT+ADV?"         // 查询广播状态
#define AT_CMD_LINK_QUERY       "AT+LINK?"        // 查询连接列表
#define AT_CMD_POWER_QUERY      "AT+POWER?"       // 查询电源状态
#define AT_CMD_RSSI_QUERY       "AT+RSSI?"        // 查询RSSI
#define AT_CMD_MAP_QUERY        "AT+MAP?"         // 查询简易导航状态
#define AT_CMD_AUTO_PLUS        "AT+AUTO+++?"      // 查询透传
#define AT_CMD_INDUCTOR_QUERY     "AT+INDUCTOR?"    // 查询无感解锁
#define AT_CMD_DIST_QUERY         "AT+DIST?"        // 查询感应范围

/* AT指令操作前缀（无参数） */
#define AT_CMD_RESET            "AT+RESET?"         // 模块复位
#define AT_CMD_SCAN_START       "AT+SCAN"          // 开始扫描
#define AT_CMD_PLUS             "AT++++"              // 进入透传

#define AT_CMD_MAX_LEN          32

// 精确导航图标
typedef enum {
    PRECISE_NAVI_INVALID = 0,                    // 无效
    PRECISE_NAVI_START = 1,                      // 起点
    PRECISE_NAVI_STRAIGHT = 2,                   // 直行
    PRECISE_NAVI_RIGHT_FRONT = 3,                // 右前方转弯
    PRECISE_NAVI_RIGHT = 4,                      // 右转
    PRECISE_NAVI_RIGHT_BACK = 5,                 // 右后方转弯
    PRECISE_NAVI_LEFT_BACK = 6,                  // 左后方转弯
    PRECISE_NAVI_LEFT = 7,                       // 左转
    PRECISE_NAVI_LEFT_FRONT = 8,                 // 左前方转弯
    PRECISE_NAVI_RIGHT_FRONT_STRAIGHT = 9,       // 右前方直行
    PRECISE_NAVI_LEFT_FRONT_STRAIGHT = 10,       // 左前方直行
    PRECISE_NAVI_LEFT_CROSS_STRAIGHT = 11,       // 左转过马路继续向前
    PRECISE_NAVI_RIGHT_CROSS_STRAIGHT = 12,      // 右转过马路继续向前
    PRECISE_NAVI_LEFT_CROSS_BACK = 13,           // 左转过马路往回走
    PRECISE_NAVI_RIGHT_CROSS_BACK = 14,          // 右转过马路往回走
    PRECISE_NAVI_LEFT_DIAGONAL_RIGHT = 15,       // 向左到路口斜对面，向右转弯
    PRECISE_NAVI_LEFT_DIAGONAL_RIGHT_FRONT = 16, // 向左到路口斜对面，向右前方转弯
    PRECISE_NAVI_LEFT_DIAGONAL_STRAIGHT = 17,    // 向左到路口斜对面，继续向前
    PRECISE_NAVI_LEFT_DIAGONAL_LEFT_FRONT = 18,  // 向左到路口斜对面，向左前方转弯
    PRECISE_NAVI_LEFT_DIAGONAL_LEFT = 19,        // 向左到路口斜对面，向左转弯
    PRECISE_NAVI_LEFT_DIAGONAL_LEFT_BACK = 20,   // 向左到路口斜对面，向左后方转弯
    PRECISE_NAVI_RIGHT_DIAGONAL_LEFT = 21,       // 向右到路口斜对面，向左转弯
    PRECISE_NAVI_RIGHT_DIAGONAL_LEFT_FRONT = 22, // 向右到路口斜对面，向左前方转弯
    PRECISE_NAVI_RIGHT_DIAGONAL_STRAIGHT = 23,   // 向右到路口斜对面，继续向前
    PRECISE_NAVI_RIGHT_DIAGONAL_RIGHT_FRONT = 24,// 向右到路口斜对面，向右前方转弯
    PRECISE_NAVI_RIGHT_DIAGONAL_RIGHT = 25,      // 向右到路口斜对面，向右转弯
    PRECISE_NAVI_RIGHT_DIAGONAL_RIGHT_BACK = 26, // 向右到路口斜对面，向右后方转弯
    PRECISE_NAVI_CROSS_LEFT = 27,                // 过马路左转
    PRECISE_NAVI_CROSS_RIGHT = 28,               // 过马路右转
    PRECISE_NAVI_ENTER_LEFT_ROAD = 29,           // 进入左侧道路
    PRECISE_NAVI_ENTER_RIGHT_ROAD = 30,          // 进入右侧道路
    PRECISE_NAVI_ENTER_LEFT_ROAD_BACK = 31,      // 进入左侧道路往回走
    PRECISE_NAVI_ENTER_RIGHT_ROAD_BACK = 32,     // 进入右侧道路往回走
    PRECISE_NAVI_DESTINATION = 33,               // 目的地
    PRECISE_NAVI_OVERBRIDGE = 34,                // 天桥
    PRECISE_NAVI_UNDERPASS = 35,                  // 地下通道

    PRECISE_NAVI_MAX
} precise_navi_icon_t;

// 完整协议帧结构（B0~B25，共26字节）
typedef struct {
    uint8_t head1;              // B0: 识别头1 (0x57)
    uint8_t head2;              // B1: 识别头2 (0x4B)
    uint8_t length;             // B2: 数据长度 (0x14)
    uint8_t fixed1;             // B3: 固定字段 (0x80)
    uint8_t fixed2;             // B4: 固定字段 (0xFF)
    uint8_t nav_type;           // B5: 导航类型
    uint8_t traffic_light_state;// B6: 红绿灯状态
    uint8_t countdown_low;      // B7: 红绿灯倒计时低字节
    uint8_t countdown_high;     // B8: 红绿灯倒计时高字节
    uint8_t distance_low;       // B9: 到红绿灯距离低字节
    uint8_t distance_high;      // B10: 到红绿灯距离高字节
    uint8_t simple_icon;        // B11: 简易导航图标
    uint8_t precise_icon;       // B12: 精确导航图标
    uint8_t next_dist_low;      // B13: 到下一路口距离低字节
    uint8_t next_dist_high;     // B14: 到下一路口距离高字节
    uint8_t gps_signal;         // B15: GPS信号
    uint8_t deviation;          // B16: 偏航信息
    uint8_t remain_time_low;    // B17: 剩余时间低字节
    uint8_t remain_time_high;   // B18: 剩余时间高字节
    uint8_t remain_dist_0;      // B19: 剩余里程字节0
    uint8_t remain_dist_1;      // B20: 剩余里程字节1
    uint8_t remain_dist_2;      // B21: 剩余里程字节2
    uint8_t remain_dist_3;      // B22: 剩余里程字节3
    uint8_t checksum;           // B23: 异或校验
    uint8_t tail1;              // B24: 结束符 '\r' (0x0D)
    uint8_t tail2;              // B25: 结束符 '\n' (0x0A)
} rt_easy_navi_msg;

int mw_easy_navi_init(void);
const char *get_easy_navi_bt_name();

#endif

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __MW_EASY_NAVI_H__