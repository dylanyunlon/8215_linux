/**
 * @file bt_cxx.h
 * @brief BT C++ 适配层对外唯一 C ABI（范式示例）
 *
 * 本头文件铁律（适用于所有 C++ 适配模块）：
 *   - 只出现 C 类型；STL/C++ 类/引用/异常禁止出现在此
 *   - 错误码一律 int 返回；状态经参数回填（调用方提供缓冲区，NUL 终止保证）
 *
 * 示例演示的完整链路（bt_device_manager 为内部实现）：
 *   C 门面/application -> 本头文件 -> C++(STL 容器管理设备表)
 *                            |
 *                            +-- 反向调用 module/ 纯 C API：
 *                                set_param      用户蓝牙开关门控
 *                                vehicle_param  行车安全（车速）门控
 *                                mw_log         统一日志
 */
#ifndef __BT_CXX_H__
#define __BT_CXX_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 错误码：C 边界只传 int，语义固定，便于门面层直接透传给 UI */
enum {
    BT_CXX_OK          = 0,  /**< 成功 */
    BT_CXX_ERR         = -1, /**< 未分类错误（含 C++ 异常兜底命中） */
    BT_CXX_ERR_ARGS    = -2, /**< 参数非法（NULL/空串/越界/缓冲区不足） */
    BT_CXX_ERR_NOTINIT = -3, /**< 尚未 init 或已 deinit */
    BT_CXX_ERR_SWITCH  = -4, /**< 用户蓝牙开关关闭（set_param: USR_PARAM_BT_SWITCH） */
    BT_CXX_ERR_DRIVING = -5, /**< 车速超过安全门限（vehicle_param: VEH_SPEED_CURRENT） */
    BT_CXX_ERR_NODEV   = -6, /**< 设备不在已配对表 */
    BT_CXX_ERR_FULL    = -7, /**< 设备表满 */
    BT_CXX_ERR_REPEAT  = -8, /**< 地址已存在（演示 unordered_map 去重语义） */
    BT_CXX_ERR_TIMEOUT = -9,  /**< 等待超时（scan_wait 等） */
};

/** 链接闭环探测（验证桩，保留）：返回版本号 >=0，异常兜底 -1 */
int bt_cxx_probe(void);

/** 初始化适配层（幂等）。内部读用户蓝牙开关决定初始状态 */
int bt_cxx_demo_init(void);

/** 注销并清空设备表（幂等） */
int bt_cxx_demo_deinit(void);

/** 添加已配对设备（addr 形如 "AA:BB:CC:DD:EE:FF"）。重复地址返回 REPEAT */
int bt_cxx_demo_add_device(const char *addr, const char *name);

/** 已配对设备数；未 init 返回 NOTINIT */
int bt_cxx_demo_get_device_count(void);

/** 按序号取设备名（name_buf 调用方提供，保证 NUL 终止） */
int bt_cxx_demo_get_device(int index, char *name_buf, int buf_len);

/** 连接设备：完整演示纯 C API 门控链（开关 -> 车速 -> 查表） */
int bt_cxx_demo_connect(const char *addr);

/**
 * 启动自动重连服务（osal::Worker + osal::Timer 组合示例）：
 * 立即试连一次；失败则每 3s 由退避定时器唤醒工作线程重试，
 * 连接判定复用 connect() 的纯 C API 门控链（开关/车速）。
 * 重复调用=重定目标地址并立即唤醒。停止用 bt_cxx_demo_stop_reconnect。
 */
int bt_cxx_demo_start_reconnect(const char *addr);

/** 停止自动重连服务（幂等）：先 join 工作线程，再析构定时器 */
int bt_cxx_demo_stop_reconnect(void);

/**
 * 启动后台设备扫描（osal::Thread 一次性线程示例）：
 * 立即返回；线程内每 300ms 模拟发现一台设备并入表，直到时长耗尽或取消。
 * 与重连服务(Worker 常驻)对比：本例是"会退出的线程"，用 join 等结果。
 * 运行中重复调用返回 BT_CXX_ERR。
 */
int bt_cxx_demo_start_scan(int duration_ms);

/**
 * 等待扫描完成（join 语义，一次性）。
 * @return >=0 发现的设备数；BT_CXX_ERR_TIMEOUT=超时未完成（可再等/取消）
 */
int bt_cxx_demo_scan_wait(int timeout_ms);

/** 取消扫描并等待线程退出（幂等）。返回发现设备数 */
int bt_cxx_demo_scan_cancel(void);

/** 自检：跑通 init/add/去重/计数/取值/重连启停全流程并打日志。
 *  返回通过项数（8=全过；扫描示例含时长等待，不进自检，单独手动调用） */
int bt_cxx_selftest(void);

#ifdef __cplusplus
}
#endif

#endif /* __BT_CXX_H__ */
