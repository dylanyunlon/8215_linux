/**
 * @file bt_device_manager.h
 * @brief BT 适配层内部实现（仅 C++ 层使用，禁止被对外头/应用包含）
 *
 * STL 使用边界：std::vector / std::unordered_map / std::string / std::mutex
 * 只允许出现在本层与 .cpp 内；一切对外的形态都经 bt_cxx.cpp 收敛为 C ABI。
 */
#ifndef __BT_DEVICE_MANAGER_H__
#define __BT_DEVICE_MANAGER_H__

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "osal_cxx.h"  /* OSAL C++ 封装（仅 Linux 后端）：Mutex/MutexGuard */

class BtDeviceManager {
public:
    /** 单例：函数内 static（bt_device_manager.cpp），无跨编译单元初始化顺序问题 */
    static BtDeviceManager &instance();

    /* 错误码与 bt_cxx.h enum 逐值对应（内部复用同值，wrapper 直接透传） */
    enum : int {
        kOk       = 0,
        kErr      = -1,
        kErrArgs  = -2,
        kErrNotInit = -3,
        kErrSwitch  = -4,
        kErrDriving = -5,
        kErrNoDev   = -6,
        kErrFull    = -7,
        kErrRepeat  = -8,
        kErrTimeout = -9,
    };

    int  init();
    void deinit();
    int  start_reconnect(const std::string &addr);
    void stop_reconnect();
    int  start_scan(int duration_ms);
    int  scan_wait(int timeout_ms);
    int  scan_cancel();
    int  add_device(const std::string &addr, const std::string &name);
    int  device_count();
    int  get_device(int index, std::string &out_name);
    int  connect(const std::string &addr);

private:
    BtDeviceManager() = default;
    ~BtDeviceManager() = default;
    BtDeviceManager(const BtDeviceManager &) = delete;
    BtDeviceManager &operator=(const BtDeviceManager &) = delete;

    /* ---------- 反向调用 module/ 纯 C API（本示例核心演示点） ----------
     * C++ 调 C 天然安全（头文件均带 extern "C" guard），纪律在失败路径：
     * 就绪检查 -> 取值 -> 校验 -> 明确默认行为，四步缺一不可。        */
    bool    is_user_bt_switch_on();  /* set_param: 用户蓝牙开关 */
    int32_t vehicle_speed_kmh();     /* vehicle_param: 当前车速 */

    struct Device {
        std::string addr;  /**< 设备地址（主键） */
        std::string name;  /**< 显示名 */
    };

    static const int kMaxDevices    = 8;   /* 设备表上限（演示 kErrFull 语义） */
    static const int kSpeedGateKmh  = 80;  /* 行车安全门限：超速禁连 */

    osal::Mutex mutex_;                              /* C++ 侧线程安全（OSAL RAII 封装） */
    std::vector<Device> devices_;                    /* STL 容器1：有序设备表 */
    std::unordered_map<std::string, std::size_t> by_addr_;  /* 容器2：地址 O(1) 查重/查找 */
    bool inited_ = false;

    /* ---- 自动重连服务：osal::Worker + osal::Timer 组合（并发原语用法示例）----
     * 模型：Timer 回调(守护线程,只 kick,微秒级) -> Worker step(工作线程,重活)
     * 声明顺序约定：worker_ 必须最后声明（逆序析构最先执行，届时
     * step 可能访问的成员 -- reconnect_addr_/retry_timer_ -- 仍存活）。 */
    void reconnect_step();                       /* Worker 单步：试连+失败布防 */
    void scan_loop(int duration_ms);             /* Thread 一次性：扫描入库 */
    std::string reconnect_addr_;                 /* 受 mutex_ 保护 */
    std::atomic<bool> reconnect_active_{false};  /* 服务运行标志 */
    std::unique_ptr<osal::Timer> retry_timer_;         /* 3s 退避定时器（失败才布防） */
    /* ---- 一次性线程示例（osal::Thread + join）：后台扫描 ----
     * 与 Worker 的分工：Thread=会退出的一次性任务（join 等结果），
     *                 Worker=常驻循环（stop+join 收尾）。 */
    std::atomic<bool> scanning_{false};        /* 扫描进行中（start/loop 维护） */
    std::atomic<bool> scan_stop_{false};       /* 取消请求 */
    std::atomic<int>  scan_found_{0};          /* 已发现数（线程写，wait 读） */
    std::unique_ptr<osal::Thread> scan_thread_; /* 声明于 worker_ 之前 */
    osal::Worker worker_{"bt_reconn"};         /* 事件驱动工作线程（最后声明） */
};

#endif /* __BT_DEVICE_MANAGER_H__ */
