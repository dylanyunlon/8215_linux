// src/MVVM/dashboard_vm.c

#include "dashboard_vm.h"
#include "awtk.h"
#include <string.h>

static bool_t             g_t_init = FALSE;
static mvvm_emitter_t     g_emitter;
static dashboard_model_t  g_sent;

static void emit_field(dashboard_field_e field, const void* value) {
    dashboard_vm_change_t ev;
    ev.field = field;
    ev.value = value;
    mvvm_emitter_emit(&g_emitter, &ev);
}


static void vm_flush_locked(void) {
    const dashboard_model_t* cur = dashboard_model_get_data();
    if (cur == NULL) return;

    if (cur->speed != g_sent.speed) {
        g_sent.speed = cur->speed;
        emit_field(DASHBOARD_FIELD_SPEED, &cur->speed);
    }
    if (cur->rpm != g_sent.rpm) {
        g_sent.rpm = cur->rpm;
        emit_field(DASHBOARD_FIELD_RPM, &cur->rpm);
    }
    if (cur->gear != g_sent.gear) {
        g_sent.gear = cur->gear;
        emit_field(DASHBOARD_FIELD_GEAR, &cur->gear);
    }
    if (cur->power != g_sent.power) {
        g_sent.power = cur->power;
        emit_field(DASHBOARD_FIELD_POWER, &cur->power);
    }
    if (cur->battery != g_sent.battery) {
        g_sent.battery = cur->battery;
        emit_field(DASHBOARD_FIELD_BATTERY, &cur->battery);
    }
    if (memcmp(cur->signals, g_sent.signals, sizeof(cur->signals)) != 0) {
        memcpy(g_sent.signals, cur->signals, sizeof(cur->signals));
        emit_field(DASHBOARD_FIELD_SIGNALS, cur->signals);
    }
}


static ret_t dashboard_vm_idle_flush(const idle_info_t* info) {
    (void)info;
    if (g_t_init) vm_flush_locked();
    return RET_REMOVE;
}

static void on_model_changed(void* ctx, const void* value) {
    (void)ctx;
    (void)value;
    idle_queue(dashboard_vm_idle_flush, NULL);
}

ret_t dashboard_vm_init(void) {
    if (g_t_init) return RET_OK;

    mvvm_emitter_init(&g_emitter, "dashboard.vm");

    memset(&g_sent, 0, sizeof(g_sent));
    g_sent.gear = DASHBOARD_GEAR_INVALID;

    dashboard_model_init();
    mvvm_emitter_on(dashboard_model_get_emitter(),
                    on_model_changed, NULL);

    g_t_init = TRUE;

    // 初始冲刷:把当前 model 值同步给 UI(若 model 尚无数据则为零值,无副作用)。
    vm_flush_locked();

    return RET_OK;
}

// 供 GUI 线程主动冲刷(如页面初始化后、进入下一页重新显示时),线程安全地 await 到本人
// 线程再调;与 idle 链等效,仅用于需要"立刻刷新"的非数据驱动场景。
void dashboard_vm_flush(void) {
    if (g_t_init) vm_flush_locked();
}

mvvm_emitter_t* dashboard_vm_emitter(void) {
    return &g_emitter;
}
