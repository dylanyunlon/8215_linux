// src/MVVM/dashboard_vm.h
//
// UI 无关的 ViewModel：
//  - 不依赖具体 UI 框架；使用标准 C 与置脏标志
//  - vm emitter 回调固定由 GUI 线程调用 dashboard_vm_flush() 派发
//    LVGL: main 循环每帧调用
//  - 返回码：0=OK，负=错误
#ifndef DASHBOARD_VM_H
#define DASHBOARD_VM_H

#include "MVVM/core/emitter.h"
#include "MVVM/model/dashboard_model.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dashboard_field_e field;
    const void*       value;
} dashboard_vm_change_t;

int               dashboard_vm_init(void);
mvvm_emitter_t*   dashboard_vm_emitter(void);

/* GUI 线程冲刷:全量 diff model 与上次已推送快照,有变化再 emit。
 * model 变更(CAN 线程)只置脏;本函数由 GUI 线程周期调用消费脏标志。 */
void              dashboard_vm_flush(void);

#ifdef __cplusplus
}
#endif
#endif
