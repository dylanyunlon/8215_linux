#include "dock_view.h"
#include "MVVM/viewModel/dock_vm.h"
#include "MVVM/core/binding.h"

const char* home_dock_widget_name[ICON_NUM_MAX] = {
    "icon_info" , "icon_navi" , "icon_music" , "icon_phone" , "icon_setting" , " "
} ;

static widget_t* home_dock_widget[ICON_NUM_MAX] = { NULL };

static ret_t on_dock_btn_clicked(void* ctx, event_t* e) {
    (void)e;
    dock_tab_e tab = (dock_tab_e)(intptr_t)ctx;
    dock_vm_set_current_tab(tab);

    printf("on_dock_btn_clicked tab = %d \n" , tab) ;
    return RET_OK;
}

static void on_dock_tab_emitted(void* ctx, const void* value) {
    (void)ctx;
    if (value == NULL) return;
    dock_tab_e tab = *(const dock_tab_e*)value;
    home_refresh_dock_icon((int)tab);

    set_dock_view((int)tab);
}

ret_t home_dock_view_init(widget_t* parent)
{

    if(parent == NULL) return RET_FAIL;
    for (size_t i = 0; i < ICON_NUM_MAX; i++){
        home_dock_widget[i] = widget_lookup(parent, home_dock_widget_name[i], TRUE);
    }

    if (home_dock_widget[ICON_INFO]){
        widget_set_state(home_dock_widget[ICON_INFO] , STATE_SELECTE) ;
    }
    
        for (int i = ICON_INFO; i < ICON_SETTING + 1; i++) {
        widget_t* btn = home_dock_widget[i];
        if (btn) {
            widget_on(btn, EVT_CLICK, on_dock_btn_clicked, (void*)(intptr_t)i);
            if (i == ICON_INFO) {
                MVVM_BIND(btn, dock_vm_get_emitter(), on_dock_tab_emitted);
            }
        }
    }

    return RET_OK ;
}

ret_t home_refresh_dock_icon(int index)
{
    index = tk_min(index, ICON_NUM_MAX);
    index = tk_max(index, ICON_INFO);


    for (size_t i = ICON_INFO; i < ICON_NUM_MAX; i++){
        if (home_dock_widget[i]){
            if (index == i)
                widget_set_state(home_dock_widget[i] , STATE_SELECTE);
            else
                widget_set_state(home_dock_widget[i] , STATE_NORMAL) ;
        }
        
    }
    
    return RET_OK ;
}

