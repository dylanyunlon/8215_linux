/**
 * @file osal_cxx.h
 * @brief OSAL 的 C++ 封装（RAII / std::function / 模板 POD 队列）
 *
 * 定位与边界（重要）：
 *   - 仅支持 Linux 后端（MW_OS=linux）。RTOS 后端（freertos/rtthread）
 *     继续使用纯 C 的 osal.h，本文件在非 Linux 后端直接 #error 拦截。
 *   - 只依赖 osal.h，属于 OSAL 的薄包装层；纯 C 代码对本文件零感知。
 *   - header-only：无需改动 Makefile，C++ 模块 #include "osal_cxx.h" 即用。
 *   - 所有包装不抛异常，错误保持 OSAL 的 int 语义（OSAL_OK/ERR/TIMEOUT）。
 *
 * 相比直接用 C API 的增益：
 *   - Mutex/Semaphore/Timer/MessageQueue 构造即创建、析构即销毁（RAII）
 *   - MutexGuard：异常安全的临界区（等价 std::lock_guard）
 *   - Thread/Timer 接 std::function，消灭 C 回调传 this 的入口样板（用户不再写 trampoline，见 Thread 注释）
 *   - MessageQueue<T> 编译期断言 T 必须 POD -- 定长拷贝语义的纪律硬约束
 */
#ifndef __OSAL_CXX_H__
#define __OSAL_CXX_H__

#include "osal.h"  /* 引入后端宏与全部 C 原语 */

#if !OSAL_BACKEND_LINUX
#error "osal_cxx.h 仅支持 MW_OS=linux 后端；RTOS 后端请使用纯 C 的 osal.h"
#endif

#include <atomic>
#include <functional>
#include <memory>
#include <queue>
#include <type_traits>
#include <utility>

namespace osal {

/* ==================== 自由函数直通 ==================== */

inline void delay_ms(uint32_t ms) { osal_delay_ms(ms); }
inline void delay_us(uint32_t us) { osal_delay_us(us); }
inline uint64_t tick_ms(void) { return osal_tick_ms(); }

/* ==================== 互斥锁 ==================== */

/** 互斥锁 RAII 持有：构造 init、析构 destroy；Linux 下零开销直通 pthread。
 *  须声明在 MutexGuard 之前（后者引用本类型）。 */
class Mutex {
public:
    explicit Mutex(bool recursive = false) { osal_mutex_init(&m_, recursive); }
    ~Mutex() { osal_mutex_destroy(&m_); }
    Mutex(const Mutex &) = delete;
    Mutex &operator=(const Mutex &) = delete;

    void lock() { osal_mutex_lock(&m_); }
    void unlock() { osal_mutex_unlock(&m_); }
    osal_mutex_t *raw() { return &m_; }

private:
    osal_mutex_t m_;
};

/** RAII 守卫：构造加锁、析构解锁（含异常路径），等价 std::lock_guard。
 *  双构造：osal::Mutex 对象（推荐）；或裸 osal_mutex_t（需与 C 代码
 *  共享同一把锁时用，锁的 init/destroy 仍归 C 侧管）。 */
class MutexGuard {
public:
    explicit MutexGuard(Mutex &m) : m_(m.raw()) { osal_mutex_lock(m_); }
    explicit MutexGuard(osal_mutex_t &m) : m_(&m) { osal_mutex_lock(m_); }
    ~MutexGuard() { osal_mutex_unlock(m_); }
    MutexGuard(const MutexGuard &) = delete;
    MutexGuard &operator=(const MutexGuard &) = delete;

private:
    osal_mutex_t *m_;
};

/* ==================== 计数信号量 ==================== */

/** RAII 信号量：析构自动 delete；wait 失败（句柄无效）返回 false 不抛 */
class Semaphore {
public:
    explicit Semaphore(uint32_t init_count = 0,
                       uint32_t max_count = 0xFFFFFFFFu)
        : s_(osal_sem_create(init_count, max_count)) {}
    ~Semaphore() {
        if (s_ != nullptr) {
            osal_sem_delete(s_);
        }
    }
    Semaphore(const Semaphore &) = delete;
    Semaphore &operator=(const Semaphore &) = delete;

    bool valid() const { return s_ != nullptr; }
    bool wait() { return osal_sem_wait(s_) == OSAL_OK; }
    bool timed_wait(uint32_t ms) { return osal_sem_timedwait(s_, ms) == OSAL_OK; }
    bool post() { return osal_sem_post(s_) == OSAL_OK; }

private:
    osal_sem_t *s_;
};

/* ==================== 线程 ==================== */

/**
 * RAII 线程：以 std::function 为线程主函数（thread_main），免写 C 样板。
 *
 * ================== 线程入口的四层调用链（看懂这段=看懂全部）==================
 *
 *   osal::Thread t("name", [this]{ my_func(); });
 *                        └── 你传入的 lambda（thread_main）
 *
 *   [新线程] pthread 启动
 *      -> ① c_entry(arg)            C 入口：pthread 只认 C 函数指针，
 *                                    成员函数/带捕获 lambda 都不能直接给它，
 *                                    所以由它接 C 边界、从 arg 取回 this
 *      -> ② self->main_()           std::function 间接调用（存在对象里）
 *      -> ③ lambda 的 operator()    即你写的 [this]{...}，等价于
 *                                    this->my_func()
 *      -> ④ my_func()               真正的业务函数，跑在新线程中
 *
 *   ①②层是封装内部，用户不可见；你只需要关心 ③④：写一个 lambda，
 *   在里面调用想在线程里跑的函数。lambda 捕获 this 时，owner 的存活期
 *   必须覆盖线程运行期（见生命周期契约）。
 * =============================================================================
 *
 * 安全设计（三重）：
 *   1. 异常兜底 -- 线程入口 try/catch 全收。异常若穿越 pthread 的 C 入口
 *      会 std::terminate 崩掉整个进程，这里是硬边界。
 *   2. 句柄零初始化 -- 创建失败时 valid() 返回 false（不读垃圾值）。
 *   3. join() -- done 信号量等待线程函数返回（一次性；对齐 common/
 *      task_server 的 done-sem 模式）。常驻线程勿调用（永久阻塞）。
 *
 * 生命周期契约（不可违反）：
 *   - Thread 对象必须活得比线程函数久（main_ 存于对象内，对象先亡即悬垂）。
 *     可退出线程的正确收尾：join() 返回后再析构 Thread；
 *     常驻线程随进程退出即可。
 *   - 底层 pthread 已 detach（见 osal_linux.c），无 joinable 资源滞留。
 *   - name 限 15 字节内（Linux pthread_setname_np 限制）。
 */
class Thread {
public:
    /** @param thread_main 线程主函数：线程启动即调用，返回即线程结束
     *  （返回会触发 done_.post()，使 join() 返回） */
    Thread(const char *name, std::function<void()> thread_main,
           uint32_t stack_size = 0, uint32_t prio = 0)
        : main_(std::move(thread_main)), done_(0, 1), t_{} {
        if (osal_thread_create(&t_, name, &Thread::c_entry, this,
                               stack_size, prio) != OSAL_OK) {
            t_.handle = nullptr; /* 失败兜底：valid() 可靠返回 false */
        }
    }

    ~Thread() = default; /* 不等待线程（常驻线程会永久阻塞）；见生命周期契约 */

    Thread(const Thread &) = delete;
    Thread &operator=(const Thread &) = delete;

    bool valid() const { return t_.handle != nullptr; }

    /**
     * 等待线程函数返回（一次性，第二次调用立即返回）。
     * 前提：valid()==true 且线程为可退出型；超时返回 false。
     */
    bool join(uint32_t timeout_ms = OSAL_WAIT_FOREVER) {
        return done_.timed_wait(timeout_ms);
    }

private:
    /* ① C 入口：pthread 调用的就是它（见类注释调用链图）。
     * 静态成员函数无 this，通过 arg 传回对象指针 -- 这是 C 线程 API
     * 与 C++ 成员函数之间唯一可行的桥。 */
    static void *c_entry(void *arg) {
        Thread *self = static_cast<Thread *>(arg);
        try {
            self->main_();
        } catch (...) {
            /* 边界纪律：异常禁止穿越线程入口（否则 std::terminate 崩进程）*/
            osal_log_error("osal::Thread entry exception swallowed\n");
        }
        self->done_.post();
        return nullptr;
    }

    std::function<void()> main_;  /* ② 线程主函数（构造时由 lambda 而来） */
    Semaphore done_;              /* join 用：主函数返回后 post 一次 */
    osal_thread_t t_{};
};

/* ==================== 软件定时器 ==================== */

/**
 * RAII 定时器：std::function 回调。
 *
 * 安全设计：
 *   1. 异常兜底 -- 回调 try/catch 全收。守护线程被所有定时器共享，
 *      异常穿越会 std::terminate 崩进程并停摆全部定时器。
 *   2. 析构安全 -- Linux 后端 osal_timer_delete 会等在途回调返回后才
 *      释放（见 osal.h 契约），因此 ~Timer 销毁 cb_ 时不可能还有回调
 *      在执行，无 use-after-free。
 *
 * 使用契约：
 *   - 回调在共享守护线程执行，须快速返回：正确姿势是回调内只
 *     MessageQueue::send 事件，重活留给工作线程。
 *   - 禁止在回调内销毁本定时器（Linux 后端 delete 等待自身 -> 死锁）。
 *   - 成员声明建议：owner 类中把 Timer 声明为最后一个成员 --
 *     析构最先执行，此刻回调可能访问的其他成员仍全部存活。
 */
class Timer {
public:
    Timer(std::function<void()> cb, bool auto_reload)
        : cb_(std::move(cb)),
          t_(osal_timer_create(&Timer::c_entry, this, auto_reload)) {}

    ~Timer() {
        /* delete 返回 = 在途回调已结束且后续不再触发（Linux 后端契约），
           此后销毁 cb_ 安全 */
        if (t_ != nullptr) {
            osal_timer_delete(t_);
        }
    }

    Timer(const Timer &) = delete;
    Timer &operator=(const Timer &) = delete;

    bool start(uint32_t period_ms) { return osal_timer_start(t_, period_ms) == OSAL_OK; }
    bool stop() { return osal_timer_stop(t_) == OSAL_OK; }
    bool reset() { return osal_timer_reset(t_) == OSAL_OK; }
    bool change_period(uint32_t period_ms) {
        return osal_timer_change_period(t_, period_ms) == OSAL_OK;
    }

private:
    /* C 入口（同 Thread::c_entry 的桥接职责）：定时器守护线程调用，
     * 从 arg 取回 Timer 对象再调 cb_。注意快速返回契约。 */
    static void c_entry(void *arg) {
        Timer *self = static_cast<Timer *>(arg);
        try {
            self->cb_();
        } catch (...) {
            /* 共享守护线程的硬边界：异常吞掉并记日志，绝不穿越 */
            osal_log_error("osal::Timer callback exception swallowed\n");
        }
    }

    std::function<void()> cb_;
    osal_timer_t *t_;
};

/* ==================== POD 消息队列 ==================== */

/**
 * RAII 消息队列（模板）。
 *
 * 编译期硬约束：T 必须是 trivially copyable（POD）。
 * OSAL MQ 为定长 memcpy 语义，不会调用拷贝构造 --
 * 含 std::string/指针成员的对象入队即腐坏，此处直接编译报错拦住。
 * 需要 C++ 对象队列时，请用 Mutex + Semaphore 自建（仅 Linux 模块）。
 */
template <typename T>
class MessageQueue {
    static_assert(std::is_trivially_copyable<T>::value,
                  "osal::MessageQueue<T>: T must be POD (OSAL MQ is "
                  "fixed-size memcpy semantics)");
    static_assert(sizeof(T) > 0, "empty message type");

public:
    explicit MessageQueue(uint32_t depth) : mq_(osal_mq_create(sizeof(T), depth)) {}
    ~MessageQueue() {
        if (mq_ != nullptr) {
            osal_mq_delete(mq_);
        }
    }
    MessageQueue(const MessageQueue &) = delete;
    MessageQueue &operator=(const MessageQueue &) = delete;

    bool valid() const { return mq_ != nullptr; }

    /** 入队（默认永久阻塞）。timeout: OSAL_WAIT_FOREVER / 0=不等待 / 毫秒 */
    bool send(const T &msg, uint32_t timeout_ms = OSAL_WAIT_FOREVER) {
        return osal_mq_send(mq_, &msg, timeout_ms) == OSAL_OK;
    }

    /** 出队（默认永久阻塞）。超时返回 false */
    bool recv(T &out, uint32_t timeout_ms = OSAL_WAIT_FOREVER) {
        return osal_mq_recv(mq_, &out, timeout_ms) == OSAL_OK;
    }

    uint32_t count() { return osal_mq_count(mq_); }

private:
    osal_mq_t *mq_;
};

/* ==================== 可停止工作线程 ==================== */

/**
 * Worker -- 便携可停止工作线程。
 *
 * 解决的样板：手写工作线程需要 stop 原子标志 + 唤醒信号量 + 循环退出
 * 检查 + join 限时等待 + 成员逆序声明纪律，约 20 行/处且极易写错。
 * Worker 把以上全部收进框架，用户只提供"单步处理"函数 step。
 *
 * 两种驱动模式（wake_period_ms）：
 *   0  纯事件驱动 -- 仅 kick() 触发一次 step（事件合并：多次 kick
 *                    pending 为一次）
 *   >0 周期+事件  -- 最长间隔该毫秒数调用一次 step（周期巡检场景），
 *                    kick() 可提前唤醒
 *
 * 安全设计：
 *   - step 单步异常被捕获并记日志，工作线程继续运行（单条坏事件不杀线程）
 *   - stop() 幂等：置标志 + kick 唤醒 + join 限时；析构自动 stop(3s) 兜底
 *   - 可重启：stop 后可再次 start
 *   - join 超时返回 false：线程仍可能在运行，此后 Worker/其 owner 绝不可
 *     析构（悬垂），须排查阻塞点
 *
 * 使用契约：
 *   - owner 类中建议把 Worker 声明为最后一个成员（析构最先执行，
 *     此刻 step 可能访问的其他成员仍存活）
 *   - step 内不要写 while/退出逻辑（框架负责循环与退出），只处理"一步"
 *
 * 用法：
 *   osal::Worker worker_{"bt_state"};
 *   worker_.start([this] { process_one_event(); });     // 纯事件驱动
 *   worker_.start([this] { poll_status(); }, 1000);     // 1s 周期巡检
 *   void on_packet() { queue_.push(...); worker_.kick(); }
 */
class Worker {
public:
    explicit Worker(const char *name) : name_(name) {}

    ~Worker() { stop(); /* 兜底：忘记显式 stop 时限时等待 */ }

    Worker(const Worker &) = delete;
    Worker &operator=(const Worker &) = delete;

    /**
     * 启动并循环调用 step 直到 stop。
     * @param step           单步处理函数（无参数；需要的事件由 kick 语义携带）
     * @param wake_period_ms 0=纯事件驱动(仅 kick)；>0=最长等待该毫秒数
     * @return false=已在运行或线程创建失败
     */
    bool start(std::function<void()> step, uint32_t wake_period_ms = 0,
               uint32_t stack_size = 0, uint32_t prio = 0) {
        if (started_.exchange(true)) {
            return false; /* 已在运行 */
        }
        stop_flag_ = false;
        /* Thread 主函数（lambda）= 循环调度器 run()；step 在其中被调用 */
        thread_.reset(new Thread(
            name_, [this, step, wake_period_ms] { run(step, wake_period_ms); },
            stack_size, prio));
        if (thread_ == nullptr || !thread_->valid()) {
            started_ = false;
            thread_.reset();
            return false;
        }
        return true;
    }

    /** 停止并限时等待线程退出（幂等）。false=超时未退出（见类注释风险） */
    bool stop(uint32_t timeout_ms = 3000) {
        if (!started_.exchange(false)) {
            return true; /* 未运行或已停止 */
        }
        stop_flag_ = true;
        wake_.post(); /* 唤醒阻塞中的 run */
        const bool exited = thread_ ? thread_->join(timeout_ms) : true;
        if (!exited) {
            osal_log_warn("osal::Worker[%s] not exited in %u ms\n", name_,
                          (unsigned)timeout_ms);
        }
        thread_.reset();
        return exited;
    }

    /** 外部事件唤醒：触发一次 step（pending 语义，多次 kick 合并一次） */
    void kick() { wake_.post(); }

    bool running() const { return started_; }

private:
    void run(const std::function<void()> &step, uint32_t period_ms) {
        while (!stop_flag_) {
            if (period_ms == 0) {
                if (!wake_.wait()) {
                    continue; /* 纯事件驱动：无事件不干活 */
                }
            } else {
                wake_.timed_wait(period_ms); /* 周期+事件：超时也走一轮 */
            }
            if (stop_flag_) {
                break; /* 唤醒来自 stop：直接退出 */
            }
            try {
                step();
            } catch (...) {
                /* 单步异常不杀工作线程：记日志继续（异常穿越会崩进程，
                   见 Thread 的边界纪律） */
                osal_log_error("osal::Worker[%s] step exception swallowed\n",
                               name_);
            }
        }
    }

    const char *name_;
    std::atomic<bool> stop_flag_{false};
    std::atomic<bool> started_{false};
    Semaphore wake_{0, 1};
    std::unique_ptr<Thread> thread_; /* 最后声明：析构最先执行 */
};

/* ==================== 串行任务队列 ==================== */

/**
 * TaskQueue -- 串行任务队列（闭包版生产者-消费者）。
 *
 * 解决的问题：任意线程投递闭包（"要做什么"本身），单一工作线程按 FIFO
 * 顺序串行执行。是 vendor SDK 集成的标配件：
 *   - vendor API 通常要求全部调用在同一线程 -> 所有调用 post() 排队
 *   - vendor 回调发生在它自己的线程 -> 把后续工作 post() 回本队列
 *
 * 与兄弟原语的分工：
 *   MessageQueue<T> : POD 事件流（定长拷贝），开销最小 -> 优先用它
 *   Worker          : 固定 step 的循环 -> 动作单一时用它
 *   TaskQueue       : 动作可变（闭包）且须串行 -> 本类
 *
 * 停止语义（stop）：
 *   drain=true（默认）：拒绝新任务 -> 已入队任务全部执行完 -> 退出。
 *     FIFO + 与 post 共锁保证：凡 post 返回 true 的任务必然被执行。
 *   drain=false：立即丢弃未执行任务并退出。
 *
 * 用法：
 *   osal::TaskQueue vendor_q_{"bt_vendor"};
 *   vendor_q_.start();
 *   vendor_q_.post([this] { vendor_connect(addr); });        // 任意线程
 *   static void on_cb(void* ctx, evt_t* e) {                 // vendor 线程
 *       self->vendor_q_.post([=] { self->handle(e); });      // 调回主队列
 *   }
 *
 * 注意：队列无上界（内存受 post 频率与执行速度差支配），持续积压说明
 * 设计有问题，用 size() 观测。
 */
class TaskQueue {
public:
    explicit TaskQueue(const char *name) : worker_(name) {}

    ~TaskQueue() { stop(); /* 兜底：默认排空语义限时 3s */ }

    TaskQueue(const TaskQueue &) = delete;
    TaskQueue &operator=(const TaskQueue &) = delete;

    /** 启动（stop 后可重启；重启会清空上次残留） */
    bool start() {
        {
            MutexGuard lk(mtx_);
            stopping_ = false;
            std::queue<std::function<void()>> empty;
            tasks_.swap(empty); /* 丢弃未 drain 停止的残留 */
        }
        return worker_.start([this] { step(); });
    }

    /**
     * 投递一个任务（任意线程调用）。
     * @return false=已停止（任务未入队）或 task 为空
     * @note 返回 true 即保证该任务会在 stop(drain=true) 前被执行
     */
    bool post(std::function<void()> task) {
        if (!task) {
            return false;
        }
        {
            MutexGuard lk(mtx_);
            if (stopping_) {
                return false; /* 与结束标记同锁：杜绝"排在标记后"的漏执行 */
            }
            tasks_.push(std::move(task));
        }
        worker_.kick();
        return true;
    }

    /** 当前积压任务数（含未开始执行的；观测用，瞬时不精确） */
    std::size_t size() {
        MutexGuard lk(mtx_);
        return tasks_.size();
    }

    bool running() const { return worker_.running(); }

    /**
     * 停止（幂等）。
     * @param drain true=执行完已入队任务再退出（默认）；false=丢弃
     * @return false=超时未退出（某任务卡死；此后本对象绝不可析构）
     */
    bool stop(uint32_t timeout_ms = 3000, bool drain = true) {
        if (!worker_.running()) {
            return true;
        }
        if (!drain) {
            return worker_.stop(timeout_ms); /* 丢弃：Worker 直接停 */
        }
        /* 排空：尾部放结束标记（FIFO 保证其前任务全部执行），
         * 标记自身 post drained_ 信号量，stop 在此等待 */
        {
            MutexGuard lk(mtx_);
            stopping_ = true;
            tasks_.push([this] { drained_.post(); });
        }
        worker_.kick();
        if (!drained_.timed_wait(timeout_ms)) {
            osal_log_warn("TaskQueue drain timeout, hard stop\n");
            return worker_.stop(timeout_ms); /* 某任务卡死：退化为硬停 */
        }
        return worker_.stop(timeout_ms); /* 队列已空：立即返回 */
    }

private:
    void step() {
        std::queue<std::function<void()>> batch;
        {
            MutexGuard lk(mtx_);
            tasks_.swap(batch); /* 锁内只做 swap，执行在锁外 */
        }
        while (!batch.empty()) {
            std::function<void()> task = std::move(batch.front());
            batch.pop();
            try {
                task();
            } catch (...) {
                /* 单任务异常不杀队列：记日志继续执行后续任务 */
                osal_log_error("osal::TaskQueue task exception swallowed\n");
            }
        }
    }

    bool stopping_ = false;              /* 与 tasks_ 同锁（mtx_）保护 */
    Semaphore drained_{0, 1};            /* 排空标记的完成信号 */
    Mutex mtx_;
    std::queue<std::function<void()>> tasks_;
    Worker worker_;                      /* 最后声明：析构最先执行 */
};

}  // namespace osal

#endif /* __OSAL_CXX_H__ */
