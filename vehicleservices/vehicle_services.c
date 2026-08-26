#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "vehicle_services.h"
#include <mw/vehicle_param.h>
#include <mw/mw_init.h>
#include <mw/msg_manage.h>
#include <mw/bt_cxx.h> /* BT 域：底层为 lib_mw C++ 适配层（对本文件仍为纯 C ABI） */

/* 门面错误码与底层错误码数值对齐的编译期保证（错位=门面语义翻译失效） */
_Static_assert(VS_BT_ERR_ARGS == BT_CXX_ERR_ARGS, "vs_bt errcode drift");
_Static_assert(VS_BT_ERR_NOTINIT == BT_CXX_ERR_NOTINIT, "vs_bt errcode drift");
_Static_assert(VS_BT_ERR_SWITCH == BT_CXX_ERR_SWITCH, "vs_bt errcode drift");
_Static_assert(VS_BT_ERR_DRIVING == BT_CXX_ERR_DRIVING, "vs_bt errcode drift");
_Static_assert(VS_BT_ERR_TIMEOUT == BT_CXX_ERR_TIMEOUT, "vs_bt errcode drift");


#if MW_MSG_MANAGE_ENABLE
_Static_assert(VS_STORAGE_UNMOUNTED == MW_MSG_MEDIA_UNMOUNTED, "vs_storage event drift");
_Static_assert(VS_STORAGE_MOUNTED   == MW_MSG_MEDIA_MOUNTED,   "vs_storage event drift");
_Static_assert(VS_STORAGE_MEDIA_UNKNOWN == MW_MSG_MEDIA_UNKNOWN, "vs_storage media drift");
_Static_assert(VS_STORAGE_MEDIA_SD      == MW_MSG_MEDIA_SD,      "vs_storage media drift");
_Static_assert(VS_STORAGE_MEDIA_USB     == MW_MSG_MEDIA_USB,     "vs_storage media drift");

/* msg_manage poll 线程上下文回调：转发给 UI(锁less 单指针，同 s_ui_cb 范式) */
static void on_msg_media(msg_manage_media_event_e event,
                         const msg_manage_media_info_t* info) {
    /* msg_manage poll 线程上执行：只打印不阻塞，保持回调快速返回 */
    printf("[vs] storage %s %s: %s -> %s (%s)\n",
           info->media_type == MW_MSG_MEDIA_SD ? "SD" :
           info->media_type == MW_MSG_MEDIA_USB ? "USB" : "UNKNOWN",
           event == MW_MSG_MEDIA_MOUNTED ? " mounted" : " unmounted",
           info->device, info->mount_point, info->fs_type);
}
#endif

/* 中间件引导标志：防止 mw_init 被重复拉起（保持 vs_init 幂等） */
static bool s_mw_inited = false;

/* 中间件契约：本回调在 vehicle_set_data 调用者线程上下文（如 CAN 接收线程）
   执行，须快速返回。此处获取值，不刷新ui动作。 */
static void on_vehicle_change(veh_data_e id, int32_t value) {
   switch (id) {
    case VEH_SPEED_CURRENT:
        /* code */
        printf("current speed:%d\n", value);
        break;
   
    default:
        break;
   }
}

int vs_init(void) {
    if (!s_mw_inited) {
        if (mw_init() != 0) {
            return -1;
        }
        s_mw_inited = true;
    }
    vehicle_param_set_change_cb(on_vehicle_change);
#if MW_MSG_MANAGE_ENABLE
    msg_manage_set_cb(on_msg_media);
#endif
    printf("vehicle services init success!\n");
    return 0;
}

void vs_deinit(void) {
    vehicle_param_set_change_cb(NULL);
#if MW_MSG_MANAGE_ENABLE
    msg_manage_remove_cb(on_msg_media);
#endif
}

#if MW_MSG_MANAGE_ENABLE
int vs_storage_get_mounts(vs_storage_info_t* out, int cap) {
    if (!out || cap <= 0) return -1;
    msg_manage_media_info_t tmp[MW_MSG_MAX_DEVS];
    int want = (cap > MW_MSG_MAX_DEVS) ? MW_MSG_MAX_DEVS : cap;
    int n = msg_manage_get_mounts(tmp, want);
    if (n < 0) return -1;
    for (int i = 0; i < n; i++) {
        vs_storage_info_t* e = &out[i];
        memset(e, 0, sizeof(*e));
        strncpy(e->device, tmp[i].device, sizeof(e->device) - 1);
        strncpy(e->mount_point, tmp[i].mount_point, sizeof(e->mount_point) - 1);
        strncpy(e->fs_type, tmp[i].fs_type, sizeof(e->fs_type) - 1);
        e->media_type = (int)tmp[i].media_type;
    }
    return n;
}
#else
int vs_storage_get_mounts(vs_storage_info_t* out, int cap) { (void)out; (void)cap; return -1; }
#endif

int32_t vs_get(int id) {
    return vehicle_get_data((veh_data_e)id);
}

void vs_set(int id, int32_t value) {
    vehicle_set_data((veh_data_e)id, value);
}

/* ==================== BT 域门面（薄透传：签名稳定，底层可演进） ====================
 * 当前映射到 bt_cxx 演示实现；后续真实 BT（vendor .so dlopen）仍由
 * vehicle_services 之下的适配层承接，本组门面签名不变，application 无感。 */

int vs_bt_init(void) { return bt_cxx_demo_init(); }

int vs_bt_scan_start(int duration_ms) { return bt_cxx_demo_start_scan(duration_ms); }

int vs_bt_scan_wait(int timeout_ms) { return bt_cxx_demo_scan_wait(timeout_ms); }

int vs_bt_scan_cancel(void) { return bt_cxx_demo_scan_cancel(); }

int vs_bt_reconnect_start(const char *addr) { return bt_cxx_demo_start_reconnect(addr); }

int vs_bt_reconnect_stop(void) { return bt_cxx_demo_stop_reconnect(); }

int vs_bt_selftest(void) { return bt_cxx_selftest(); }
