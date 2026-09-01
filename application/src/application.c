#include "awtk.h"
#include "../3rd/awtk-widget-qr/src/qr_register.h"
#include "../3rd/awtk-widget-chart-view/src/chart_view_register.h"
#include "common/navigator.h"
#include "vehicle_services.h"
#if defined(LINUX) || defined(APPLE) || defined(HAS_STDIO) || defined(WINDOWS)
#include "BootAnimationDrv.h"
#include "atcbootanicom_c.h"
#endif

#ifndef APP_SYSTEM_BAR
#define APP_SYSTEM_BAR ""
#endif /*APP_SYSTEM_BAR*/

#ifndef APP_BOTTOM_SYSTEM_BAR
#define APP_BOTTOM_SYSTEM_BAR ""
#endif /*APP_BOTTOM_SYSTEM_BAR*/

#ifndef APP_START_PAGE
#define APP_START_PAGE "home_page"
#endif /*APP_START_PAGE*/

/**
 * 注册自定义控件
 */
static ret_t custom_widgets_register(void) {
  qr_register();
  chart_view_register();
  return RET_OK;
}

static ret_t ac8317_get_time_now(date_time_t* dt) {
	//todo 获取系统时间
	dt->year = 2025;
	dt->month = 3;
	dt->wday = 5;
	dt->day = 14;
	dt->hour = 14;
	dt->minute = 0;
	dt->second = 0;
	
	return RET_OK;
}

static ret_t ac8317_set_time_now(date_time_t* dt) {
	//todo 设置系统时间
	
	return RET_OK;
}

static date_time_vtable_t s_date_time_vtable_t = {
	.get_now = ac8317_get_time_now,
	.set_now = ac8317_set_time_now,
};

/**
 * 当程序初始化完成时调用，全局只触发一次。
 */
static ret_t application_on_launch(void) {
  date_time_global_init_ex(&s_date_time_vtable_t);
  window_manager_set_show_fps(window_manager(), TRUE);
  return RET_OK;
}

/**
 * 当程序退出时调用，全局只触发一次。
 */
static ret_t application_on_exit(void) {
  
  return RET_OK;
}
/**
 * 初始化程序
 */
ret_t application_init(void) {
  /* HCN 集成：引导中间件并注册车辆数据变更回调（中间件交互全部封装在 vehicle_services 内） */
  vs_init();
  /* BT/扫描/重连等一律经 vehicle_services 的 vs_bt_* 门面调用，application
   * 不直接触碰中间件。BT 初始化按阶段化启动延后（UI 首帧后按需
   * vs_bt_init），此处不做任何 BT 操作以保证仪表快速启动。 */
  vs_bt_init();
  
  custom_widgets_register();
  application_on_launch();



  // locale_info_change(locale_info(), "zh", "CN") ;
  return navigator_to(APP_START_PAGE);
}

/**
 * 退出程序
 */
ret_t application_exit(void) {
  application_on_exit();
  log_debug("application_exit\n");

  return RET_OK;
}
