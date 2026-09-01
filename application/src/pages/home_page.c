#include "awtk.h"
#include "../common/navigator.h"
#include "../logic/hcn_logic.h"
#include "../view/view_manager.h"
#include "MVVM/viewModel/dashboard_vm.h"
#include "MVVM/viewModel/time_vm.h"
#include "MVVM/viewModel/settings_vm.h"
#include "MVVM/model/dock_model.h"
#include "MVVM/model/dashboard_model.h"

static ret_t on_icon_info_click(void* ctx, event_t* e) {
  pointer_event_t* evt = pointer_event_cast(e);
  // TODO: 在此添加控件事件处理程序代码
  printf("on_icon_info_click x = %d , y = %d \n" , evt->x , evt->y) ;
  return RET_OK;
}

/**
 * 初始化窗口的子控件
 */


static ret_t visit_init_child(void* ctx, const void* iter) {
  widget_t* widget = WIDGET(iter);
  // (void)ctx;
  widget_t* win = (widget_t*)ctx;
  const char* name = widget->name;

  // 初始化指定名称的控件（设置属性或注册事件），请保证控件名称在窗口上唯一
  if (name != NULL && *name != '\0') {
    if (tk_str_eq(name, "icon_info")) {
      widget_on(widget, EVT_CLICK, on_icon_info_click, win);
    }

  }

  return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t home_page_init(widget_t* win, void* ctx) {
  (void)ctx;
  return_value_if_fail(win != NULL, RET_BAD_PARAMS);

  widget_foreach(win, visit_init_child, win);

  demonstration_stop() ;

  dashboard_model_init();
  dashboard_vm_init();
  time_vm_init();
  settings_vm_init();
  dock_vm_init();

  home_view_init(win) ;
  set_view_init(win) ;


  return RET_OK;
}






