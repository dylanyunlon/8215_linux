/*
 * ui_backend.h —— 平台后端的启动钩子（pc / device 各实现一份）
 *
 * port/hcn_application_entry.c 在 ui_init() 之后调用 ui_backend_init()，把"这个后端
 * 需要做的额外初始化"交给后端自己。
 *
 * **两个后端的第一行都是 `vs_init()`** —— 那是 vehicle_services 唯一的引导入口：
 *   pc     -> platform/pc/mock_vehicle_services.c（懒启动的 mock，恒返回 0），
 *             之后起假车辆数据发生器 + 假按键注入（platform/pc/ui_backend.c）
 *   device -> hcn/vehicle_services/vehicle_services.c（内部拉起真中间件），
 *             device 侧不再做别的事（platform/device/ui_backend.c）
 *
 * 这样入口文件不 include 任何 mock_* / 中间件头，两个后端共用同一条启动路径，
 * 业务代码换后端零改动。
 */
#ifndef UI_BACKEND_H
#define UI_BACKEND_H

#ifdef __cplusplus
extern "C" {
#endif

/* 幂等。必须在 lv_init() 之后、LVGL 线程里调用（两个后端都用 lv_timer）。 */
void ui_backend_init(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_BACKEND_H */
