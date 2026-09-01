#include "power_view.h"
#include "MVVM/viewModel/dashboard_vm.h"
#include "MVVM/core/binding.h"
const char* home_power_widget_name[POWER_NUM_MAX] = {
    "power_value" , "power_progress" 
} ;


static widget_t* home_power_widget[POWER_NUM_MAX] = { NULL };;


static ret_t on_power_bar_changed(void* ctx, event_t* e) {
    (void)e;
    widget_t* w = (widget_t*)ctx;
    if (w == NULL) return RET_FAIL; 

    image_value_set_value(home_power_widget[POWER_VALUE] , ((slider_t*)(w))->value) ;
}

static void on_dashboard_vm_change(void* ctx, const void* value) {
    widget_t* w = (widget_t*)ctx;
    (void)w;
    if (value == NULL) return;
    const dashboard_vm_change_t* ev = (const dashboard_vm_change_t*)value;

    switch (ev->field) {
    case DASHBOARD_FIELD_POWER:
        home_refresh_power(*(const uint32_t*)ev->value);
        printf("on_dashboard_vm_change power = %d \n" , *(const uint32_t*)ev->value) ;
        break;
    default:
        break;  // power/battery/signals 不在本视图处理
    }
}

ret_t home_power_view_init(widget_t* parent)
{
    if(parent == NULL) return RET_FAIL;
    for (size_t i = 0; i < POWER_NUM_MAX; i++){
        home_power_widget[i] = widget_lookup(parent, home_power_widget_name[i], TRUE);
    }


    widget_on(home_power_widget[POWER_BAR] , EVT_VALUE_CHANGED , on_power_bar_changed , home_power_widget[POWER_BAR]) ;
    // widget_t* anchor = home_power_widget[POWER_VALUE];    //模拟滚动条手动刷新数据 无须绑定底层数据
    // if (anchor) {
    //     MVVM_BIND(anchor, dashboard_vm_emitter(), on_dashboard_vm_change);
    // }
    return RET_OK ;
}


ret_t home_refresh_power(int power) 
{
    power = tk_min(power , POWER_MAX) ;

    float step = (float)100.0f / POWER_MAX ;

    if(home_power_widget[POWER_VALUE]){
        image_value_set_value(home_power_widget[POWER_VALUE] , power) ;
    }

    if(home_power_widget[POWER_BAR]){
        slider_set_value(home_power_widget[POWER_BAR] , (power * step)) ;
    }
    
    return RET_OK ;
}