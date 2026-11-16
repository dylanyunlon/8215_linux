// dashboard_view.c — MVVM LVGL View（HCN_APP_FULL 模式）
//
// 车辆数据面板：车速/转速/档位 label，经 dashboard_vm 订阅刷新。
// SquareLine 生成的 ui_*.c 只做布局，业务刷新集中在本文件。

#ifdef HCN_APP_FULL

#include "ui.h"
#include "MVVM/viewModel/dashboard_vm.h"
#include <stdio.h>
#include <syslog.h>

static lv_obj_t *s_speed_label = NULL;
static lv_obj_t *s_rpm_label = NULL;
static lv_obj_t *s_gear_label = NULL;

static const char *gear_to_str(int gear)
{
    switch (gear) {
    case DASHBOARD_GEAR_PARK:    return "P";
    case DASHBOARD_GEAR_NEUTRAL: return "N";
    case DASHBOARD_GEAR_DRIVE:   return "D";
    case DASHBOARD_GEAR_REVERSE: return "R";
    default:                     return "-";
    }
}

/* vm emitter 回调：GUI 线程（main 循环 dashboard_vm_flush 派发）执行，可安全操作 lvgl */
static void on_dashboard_vm_change(void *ctx, const void *value)
{
    (void)ctx;
    const dashboard_vm_change_t *ev = (const dashboard_vm_change_t *)value;
    const dashboard_model_t *data = dashboard_model_get_data();

    switch (ev->field) {
    case DASHBOARD_FIELD_SPEED:
        if (s_speed_label) {
            lv_label_set_text_fmt(s_speed_label, "SPD %d km/h", (int)data->speed);
        }
        break;
    case DASHBOARD_FIELD_RPM:
        if (s_rpm_label) {
            lv_label_set_text_fmt(s_rpm_label, "RPM %d", (int)data->rpm);
        }
        break;
    case DASHBOARD_FIELD_GEAR:
        if (s_gear_label) {
            lv_label_set_text_fmt(s_gear_label, "GEAR %s", gear_to_str((int)data->gear));
        }
        break;
    default:
        break;
    }
}

void dashboard_view_attach(lv_obj_t *parent)
{
    if (parent == NULL || s_speed_label != NULL) {
        return; /* 幂等 */
    }

    /* VM 装配：model -> vm -> view；dashboard_vm_init() 内部初始化 model。 */
    dashboard_vm_init();
    mvvm_emitter_on(dashboard_vm_emitter(), on_dashboard_vm_change, NULL);

    /* 面板容器：挂 Home 屏顶部中间，不侵入既有布局 */
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_size(panel, 320, 90);
    lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_bg_opa(panel, LV_OPA_70, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    s_speed_label = lv_label_create(panel);
    lv_obj_set_style_text_font(s_speed_label, ui_font_chinese_16_get(), 0);
    lv_label_set_text(s_speed_label, "SPD -- km/h");

    s_rpm_label = lv_label_create(panel);
    lv_obj_set_style_text_font(s_rpm_label, ui_font_chinese_16_get(), 0);
    lv_label_set_text(s_rpm_label, "RPM --");

    s_gear_label = lv_label_create(panel);
    lv_obj_set_style_text_font(s_gear_label, ui_font_chinese_16_get(), 0);
    lv_label_set_text(s_gear_label, "GEAR -");

    syslog(LOG_INFO, "[hcn-app] dashboard view attached to home screen");

    /* attach 后立刻冲刷一次：把 model 当前值同步到面板 */
    dashboard_vm_flush();
}

#endif /* HCN_APP_FULL */
