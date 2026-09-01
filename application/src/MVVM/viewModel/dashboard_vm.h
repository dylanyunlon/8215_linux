// src/MVVM/dashboard_vm.h
#ifndef DASHBOARD_VM_H
#define DASHBOARD_VM_H

#include "awtk.h"
#include "MVVM/core/emitter.h"
#include "MVVM/model/dashboard_model.h"

BEGIN_C_DECLS

typedef struct {
    dashboard_field_e field;
    const void*       value;
} dashboard_vm_change_t;

ret_t             dashboard_vm_init(void);
mvvm_emitter_t*   dashboard_vm_emitter(void);

/* GUI 线程冲刷:全量 diff model 与上次已推送快照,有变化再 emit。 */
void              dashboard_vm_flush(void);

END_C_DECLS
#endif