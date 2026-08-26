/**
 * @file bt_device_manager.cpp
 * @brief BT 适配层内部实现：STL 容器管理 + module/ 纯 C API 反向调用
 */
#include "bt_device_manager.h"

#include "mw_log.h"        /* common/log 纯 C API */
#include "set_param.h"     /* module/set_param 纯 C API（用户参数） */
#include "vehicle_param.h" /* module/vehicle_param 纯 C API（车辆数据） */

BtDeviceManager &BtDeviceManager::instance() {
    /* 函数内 static：首次调用构造，规避静态初始化顺序问题（fiasco） */
    static BtDeviceManager s_inst;
    return s_inst;
}

/* ==================== 纯 C API 反向调用封装 ==================== */

bool BtDeviceManager::is_user_bt_switch_on() {
    /* C API 调用纪律示范：
     * 1) 先查就绪状态（usr_param_init 是否已跑过，mw_init 链路内）
     * 2) 取值失败（返回 false）必须有明确默认行为，不静默            */
    if (!get_recovery_usr_param()) {
        mw_log_warn("usr param not ready, bt switch treat as off\n");
        return false;
    }
    uint8_t on = 0;
    if (!get_usr_param(USR_PARAM_BT_SWITCH, &on)) {
        mw_log_warn("get_usr_param(BT_SWITCH) failed, treat as off\n");
        return false;
    }
    return on != 0;
}

int32_t BtDeviceManager::vehicle_speed_kmh() {
    /* vehicle_get_data 无失败码（int32_t 直返），异常值防御性钳位 */
    int32_t v = vehicle_get_data(VEH_SPEED_CURRENT);
    if (v < 0 || v > 300) {
        mw_log_warn("abnormal speed %d, clamp to 0\n", (int)v);
        return 0;
    }
    return v;
}

/* ==================== 设备表管理（STL 容器） ==================== */

int BtDeviceManager::init() {
    osal::MutexGuard lk(mutex_);
    if (inited_) {
        return kOk;  /* 幂等 */
    }
    devices_.clear();
    by_addr_.clear();
    by_addr_.reserve(kMaxDevices * 2);  /* 预留桶，避免小表反复 rehash */
    inited_ = true;
    mw_log_info("bt cxx adapter init ok (user switch=%d)\n",
                is_user_bt_switch_on() ? 1 : 0);
    return kOk;
}

void BtDeviceManager::deinit() {
    stop_reconnect(); /* 先停工作线程与定时器，再清表（成员表可能被 step 访问） */
    scan_cancel();    /* 再收一次性扫描线程（join 后才可安全清表） */
    osal::MutexGuard lk(mutex_);
    devices_.clear();
    by_addr_.clear();
    inited_ = false;
}

int BtDeviceManager::add_device(const std::string &addr, const std::string &name) {
    if (addr.empty() || name.empty()) {
        return kErrArgs;
    }
    osal::MutexGuard lk(mutex_);
    if (!inited_) {
        return kErrNotInit;
    }
    if (by_addr_.count(addr) > 0) {          /* unordered_map O(1) 查重 */
        return kErrRepeat;
    }
    if (static_cast<int>(devices_.size()) >= kMaxDevices) {
        return kErrFull;
    }
    by_addr_[addr] = devices_.size();        /* 记录下标映射 */
    devices_.push_back({addr, name});        /* vector 保序 */
    mw_log_info("bt dev added [%s] \"%s\" (%d/%d)\n",
                addr.c_str(), name.c_str(),
                static_cast<int>(devices_.size()), kMaxDevices);
    return kOk;
}

int BtDeviceManager::device_count() {
    osal::MutexGuard lk(mutex_);
    if (!inited_) {
        return kErrNotInit;
    }
    return static_cast<int>(devices_.size());
}

int BtDeviceManager::get_device(int index, std::string &out_name) {
    osal::MutexGuard lk(mutex_);
    if (!inited_) {
        return kErrNotInit;
    }
    if (index < 0 || index >= static_cast<int>(devices_.size())) {
        return kErrArgs;
    }
    out_name = devices_[static_cast<std::size_t>(index)].name;
    return kOk;
}

/* ==================== 后台扫描（一次性 Thread + join） ==================== */

int BtDeviceManager::start_scan(int duration_ms) {
    if (duration_ms <= 0) {
        return kErrArgs;
    }
    {
        osal::MutexGuard lk(mutex_);
        if (!inited_) {
            return kErrNotInit;
        }
    }
    if (scanning_.exchange(true)) {
        return kErr; /* 已在扫描 */
    }
    scan_stop_ = false;
    scan_found_ = 0;
    /* 一次性线程：std::function 入口免 trampoline；栈/优先级用默认 */
    scan_thread_.reset(new osal::Thread(
        "bt_scan", [this, duration_ms] { scan_loop(duration_ms); }));
    if (!scan_thread_->valid()) {
        scanning_ = false;
        scan_thread_.reset();
        return kErr;
    }
    mw_log_info("scan started (%d ms)\n", (int)duration_ms);
    return kOk; /* 立即返回，不阻塞调用方 */
}

int BtDeviceManager::scan_wait(int timeout_ms) {
    if (scan_thread_ == nullptr) {
        return kErrNotInit; /* 未启动或已收尾 */
    }
    /* join 一次性语义：等线程函数返回。超时返回 kErrTimeout，可重试 */
    if (!scan_thread_->join(timeout_ms)) {
        return kErrTimeout;
    }
    const int found = scan_found_.load();
    scan_thread_.reset(); /* join 成功后释放句柄（此后线程确定已退出） */
    return found;
}

int BtDeviceManager::scan_cancel() {
    scan_stop_ = true;          /* 先置取消：loop 的 sleep/周期检查点退出 */
    if (scan_thread_ == nullptr) {
        return scan_found_.load();
    }
    const bool exited = scan_thread_->join(2000);
    const int found = scan_found_.load();
    scan_thread_.reset();
    if (!exited) {
        mw_log_warn("scan thread not exited in 2s\n");
        return kErr; /* 线程仍活：此后本对象绝不可析构 */
    }
    return found;
}

void BtDeviceManager::scan_loop(int duration_ms) {
    const uint64_t deadline = osal::tick_ms() + (uint64_t)duration_ms;
    int idx = 0;
    while (!scan_stop_ && osal::tick_ms() < deadline) {
        osal::delay_ms(300); /* 模拟 inquiry 周期（真实实现=vendor 扫描调用） */
        if (scan_stop_) {
            break;
        }
        char addr[32], name[48];
        snprintf(addr, sizeof(addr), "AA:BB:CC:DD:EE:%02X",
                 (unsigned)(0x10 + idx));
        snprintf(name, sizeof(name), "ScanDev-%d", idx);
        if (add_device(addr, name) == kOk) { /* 复用容器管理：入表+去重 */
            scan_found_++;
            idx++;
        }
    }
    scanning_ = false;
    mw_log_info("scan finished, found=%d\n", scan_found_.load());
}

/* ==================== 自动重连服务（Worker + Timer） ==================== */

int BtDeviceManager::start_reconnect(const std::string &addr) {
    if (addr.empty()) {
        return kErrArgs;
    }
    {
        osal::MutexGuard lk(mutex_);
        if (!inited_) {
            return kErrNotInit;
        }
    }
    if (reconnect_active_.exchange(true)) {
        /* 已在运行：重定目标地址并立即唤醒（幂等重入语义） */
        {
            osal::MutexGuard lk(mutex_);
            reconnect_addr_ = addr;
        }
        worker_.kick();
        return kOk;
    }
    /* Timer：单次到期，回调只 kick 工作线程（守护线程快速返回契约） */
    retry_timer_.reset(new osal::Timer([this] { worker_.kick(); }, false));
    /* Worker：纯事件驱动，每 kick 执行一次 reconnect_step */
    if (!worker_.start([this] { reconnect_step(); })) {
        retry_timer_.reset();
        reconnect_active_ = false;
        return kErr;
    }
    worker_.kick(); /* 立即首次尝试 */
    mw_log_info("reconnect service started for %s\n", addr.c_str());
    return kOk;
}

void BtDeviceManager::stop_reconnect() {
    if (!reconnect_active_.exchange(false)) {
        return; /* 未运行，幂等 */
    }
    /* 顺序是关键：先 stop/join 工作线程（此后 step 绝不再执行、
     * 不再触碰 retry_timer_），再析构定时器（~Timer 自身还会等待
     * in-flight 回调返回，双重保险） */
    worker_.stop();
    retry_timer_.reset();
    mw_log_info("reconnect service stopped\n");
}

void BtDeviceManager::reconnect_step() {
    if (!reconnect_active_) {
        return; /* 已请求停止：不再布防 */
    }
    std::string addr;
    {
        osal::MutexGuard lk(mutex_);
        addr = reconnect_addr_;
    }
    /* 复用 connect()：完整走 set_param 开关 -> vehicle_param 车速 -> 查表 */
    int rc = connect(addr);
    mw_log_info("reconnect attempt [%s] rc=%d\n", addr.c_str(), rc);
    if (rc == kOk) {
        return; /* 连上：不再布防定时器，等下次 start */
    }
    if (!reconnect_active_) {
        return;
    }
    /* 失败：3s 后定时器 kick 重试（单次定时器，每次失败重新布防） */
    if (retry_timer_ != nullptr) {
        retry_timer_->start(3000);
    }
}

int BtDeviceManager::connect(const std::string &addr) {
    if (addr.empty()) {
        return kErrArgs;
    }
    /* 门控1：用户蓝牙开关（set_param 纯 C API）-- 关闭则拒绝，错误码直达 UI */
    if (!is_user_bt_switch_on()) {
        mw_log_warn("connect %s rejected: user bt switch off\n", addr.c_str());
        return kErrSwitch;
    }
    /* 门控2：行车安全（vehicle_param 纯 C API）-- 超速禁连 */
    int32_t spd = vehicle_speed_kmh();
    if (spd > kSpeedGateKmh) {
        mw_log_warn("connect %s rejected: driving %d km/h over gate %d\n",
                    addr.c_str(), static_cast<int>(spd), kSpeedGateKmh);
        return kErrDriving;
    }
    osal::MutexGuard lk(mutex_);
    if (!inited_) {
        return kErrNotInit;
    }
    std::unordered_map<std::string, std::size_t>::const_iterator it =
        by_addr_.find(addr);                 /* unordered_map O(1) 查找 */
    if (it == by_addr_.end()) {
        return kErrNoDev;
    }
    mw_log_info("bt connected [%s] \"%s\"\n",
                addr.c_str(), devices_[it->second].name.c_str());
    return kOk;
}
