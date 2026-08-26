#ifndef __MSG_MANAGE_H__
#define __MSG_MANAGE_H__

/**
 * @file msg_manage.h
 * @brief 存储设备插拔消息(SD/USB) -- 轮询 /proc/mounts diff，多订阅者分发
 *
 * 机制：worker 线程每 MW_MSG_POLL_MS 读 /proc/mounts，与上次快照 diff，
 *       新增项 -> MOUNTED、消失项 -> UNMOUNTED，带 device/mount_point/fs_type/media_type。
 *       media_type 由 device 名分类：mmcblk* -> SD，sd* sr* -> USB。
 *
 * 消息来源为 mountservice 守护进程的挂载/卸载结果(/proc/mounts 反映其挂载)，
 * 故本模块只报告"已挂载/已卸载"事件，不报告物理插入但挂载失败的设备。
 *
 * 订阅：多订阅者(MW_MAX_CALLBACK_NUM)，回调在 msg_manage 的 poll 线程上下文
 *       执行，必须秒返(只读值/发消息)，禁止在其中阻塞或做重活。
 *
 * 健康状态经注册表暴露：msg_manage_init() 自挂 self_check(MW_MOD_MSG_MANAGE)，
 *       `modules` 命令可见 ok/FAULT。
 *
 * 用法：
 *   msg_manage_init();
 *   msg_manage_set_cb(my_cb);   // 多模块可各注册各的
 *   // my_cb(event, info): info->device/mount_point/fs_type/media_type
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "dev_config.h"

#if MW_MSG_MANAGE_ENABLE

typedef enum {
    MW_MSG_MEDIA_UNMOUNTED = 0,  /*< 卸载/拔出(已挂载 -> 消失) */
    MW_MSG_MEDIA_MOUNTED   = 1,  /*< 挂载/插入(新增挂载点) */
} msg_manage_media_event_e;

typedef enum {
    MW_MSG_MEDIA_UNKNOWN = 0,
    MW_MSG_MEDIA_SD      = 1,    /*< device 名以 /dev/mmcblk 开头 */
    MW_MSG_MEDIA_USB     = 2,    /*< device 名以 /dev/sd 或 /dev/sr 开头 */
} msg_manage_media_type_e;

typedef struct {
    char device[64];                /*< /dev/sda1、/dev/mmcblk0p1 */
    char mount_point[128];          /*< /media/udisk*、/media/ext_sdcard*、/mnt/* */
    char fs_type[16];               /*< vfat、exfat、ntfs、ext4 ... */
    msg_manage_media_type_e media_type;
} msg_manage_media_info_t;

typedef void (*msg_manage_cb_t)(msg_manage_media_event_e event,
                                const msg_manage_media_info_t* info);

/** 启动 poll 线程(幂等，重复调用返回 0)。失败返回 -1 */
int msg_manage_init(void);

/** 停止 poll 线程(进程退出前调用) */
int msg_manage_deinit(void);

/** 注册订阅回调(多订阅者，去重)。0=成功 1=已注册或满 -1=参数错 */
int msg_manage_set_cb(msg_manage_cb_t cb);

/** 移除订阅回调(前移收缩)。0=成功 -1=未找到 */
int msg_manage_remove_cb(msg_manage_cb_t cb);

/** 查询当前已跟踪挂载快照(拷贝到 out，线程安全)。返回写入条目数(<=cap)；-1=参数错 */
int msg_manage_get_mounts(msg_manage_media_info_t* out, int cap);

#else /* MW_MSG_MANAGE_ENABLE=0：空转内联桩，调用点无需 #if 包裹 */

/* 桩期保留类型，使签名可声明；disabled 时不会有真实事件回调 */
typedef void (*msg_manage_cb_t)(int event, const void* info);

static inline int msg_manage_init(void) { return -1; }
static inline int msg_manage_deinit(void) { return 0; }
static inline int msg_manage_set_cb(msg_manage_cb_t cb) { (void)cb; return -1; }
static inline int msg_manage_remove_cb(msg_manage_cb_t cb) { (void)cb; return -1; }
static inline int msg_manage_get_mounts(void* out, int cap) { (void)out; (void)cap; return -1; }

#endif /* MW_MSG_MANAGE_ENABLE */

#ifdef __cplusplus
}
#endif
#endif /* __MSG_MANAGE_H__ */
