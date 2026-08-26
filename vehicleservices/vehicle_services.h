#ifndef __VEHICLE_SERVICES_H__
#define __VEHICLE_SERVICES_H__

/**
 * @file vehicle_services.h
 * @brief 车机服务层 —— 应用侧唯一的中间件门面（facade）
 *
 * 本层是 application 与中间件(lib_mw)之间唯一的交互边界：application
 * 不直接引用任何 <mw/xxx.h>，中间件引导、数据订阅、读写全部封装在此，
 * 将中间件数据交互隔离在 vehicle_services 内部。
 *
 * 职责：
 *   - 引导中间件：vs_init() 内部调用 mw_init()，拉起 osal/定时器/条件模块
 *     （如 CAN 收发线程、看门狗等），应用无需关心；
 *   - 数据订阅/映射：注册 vehicle_param 变更回调并转发给 UI 订阅者；
 *   - 稳定访问接口：vs_get/vs_set 封装车辆数据读写，UI 只依赖本层。
 *
 * 用法（application_init 第一行调用，仅此一处）：
 *   vs_init();                       // 引导中间件 + 注册 vehicle_param 变更回调
 *   int32_t spd = vs_get(VEH_SPEED_CURRENT);   // 读当前值
 *   vs_set(VEH_DRIVE_MODE, 1);       // 写回
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** UI 侧变更回调。注意：在 vehicle_param 回调线程上下文执行，须快速返回，
 *  只做存值/置脏，禁止 UI 操作、阻塞或注销本回调。 */
typedef void (*vs_change_cb_t)(int id, int32_t value);

/** 初始化：引导中间件(mw_init，仅首次)并注册 vehicle_param 变更回调。
 *  幂等。返回 0=成功，-1=中间件启动失败。 */
int vs_init(void);

/** 注销：释放回调（可重复调用）。 */
void vs_deinit(void);

/** 读当前车辆数据（等价 vehicle_get_data）。 */
int32_t vs_get(int id);

/** 写车辆数据（等价 vehicle_set_data，值变化时触发 change 派发）。 */
void vs_set(int id, int32_t value);

/* ==================== BT 域门面（封装 lib_mw 的 C++ 适配层） ====================
 * 边界原则：application 不 include 任何 <mw/xxx.h>，蓝牙相关调用一律经
 * 本层 vs_bt_* 完成。错误码在本层自治定义（与底层适配层数值对齐，
 * .c 内有 _Static_assert 校验），不向 application 泄漏中间件头。
 * 阶段化启动：vs_init() 不做任何 BT 初始化；UI 首帧后按需调 vs_bt_init。
 * ============================================================================= */

/** BT 域错误码（门面层自治；语义与底层适配层 1:1 对齐） */
enum {
    VS_BT_OK          = 0,   /**< 成功 */
    VS_BT_ERR         = -1,  /**< 未分类错误 */
    VS_BT_ERR_ARGS    = -2,  /**< 参数非法 */
    VS_BT_ERR_NOTINIT = -3,  /**< 未 vs_bt_init */
    VS_BT_ERR_SWITCH  = -4,  /**< 用户蓝牙开关关闭 */
    VS_BT_ERR_DRIVING = -5,  /**< 车速超安全门限（行车限制） */
    VS_BT_ERR_NODEV   = -6,  /**< 设备不存在 */
    VS_BT_ERR_FULL    = -7,  /**< 设备表满 */
    VS_BT_ERR_REPEAT  = -8,  /**< 地址已存在 */
    VS_BT_ERR_TIMEOUT = -9,  /**< 等待超时（scan_wait 等） */
};

/** BT 服务初始化（幂等）。启动关键路径勿调；UI 首帧后按需调用 */
int vs_bt_init(void);

/** 启动后台扫描（duration_ms 内每 300ms 模拟发现一台设备）。立即返回 */
int vs_bt_scan_start(int duration_ms);

/** 等待扫描完成（一次性）。返回 >=0 发现设备数；VS_BT_ERR_TIMEOUT=超时可重试 */
int vs_bt_scan_wait(int timeout_ms);

/** 取消扫描并等待线程退出（幂等）。返回已发现设备数 */
int vs_bt_scan_cancel(void);

/** 启动自动重连服务（幂等重入=重定目标地址并立即唤醒） */
int vs_bt_reconnect_start(const char *addr);

/** 停止自动重连服务（幂等） */
int vs_bt_reconnect_stop(void);

/** BT 适配层自检（8 项；含线程/定时器启停，调试用，勿在启动路径调用） */
int vs_bt_selftest(void);

/* ==================== 存储插拔门面(封装 lib_mw 的 msg_manage) ====================
 * 边界原则同 vs_bt_*：application 只见 vs_storage_*，不 include <mw/msg_manage.h>。
 * 回调在 msg_manage 的 poll 线程上下文执行，必须秒返(只读值/发消息)，
 * 禁止在其中阻塞或做重活。event 取 VS_STORAGE_*，media_type 取 VS_STORAGE_MEDIA_*。
 * ============================================================================= */
enum {
    VS_STORAGE_UNMOUNTED = 0,
    VS_STORAGE_MOUNTED   = 1,
};
enum {
    VS_STORAGE_MEDIA_UNKNOWN = 0,
    VS_STORAGE_MEDIA_SD      = 1,
    VS_STORAGE_MEDIA_USB     = 2,
};
typedef void (*vs_storage_cb_t)(int event,
                                const char* device,
                                const char* mount_point,
                                const char* fs_type,
                                int media_type);

/* 当前挂载快照(纯数据，不依赖 <mw/msg_manage.h>，供 UI 直接用) */
typedef struct {
    char device[64];
    char mount_point[128];
    char fs_type[16];
    int  media_type;   /* VS_STORAGE_MEDIA_* */
} vs_storage_info_t;
/** 查询当前挂载快照(拷贝到 out)。返回条目数(<=cap)；-1=参数错 */
int vs_storage_get_mounts(vs_storage_info_t* out, int cap);

#ifdef __cplusplus
}
#endif
#endif /* __VEHICLE_SERVICES_H__ */
