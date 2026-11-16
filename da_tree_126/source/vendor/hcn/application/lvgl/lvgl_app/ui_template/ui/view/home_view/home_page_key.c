#include "home_page_key.h"
#include "window_manager.h"
#include "ui_objects.h"
#include "mileage_calc.h"
#include "speed_view.h"
#include "vehicle_param.h"
#include "ui_feature_config.h"
#include "selfcheck.h"
#include "vehicle_data.h"
#include "ui_shared_apps.h"
void home_manager_click(const click_e click)
{
    switch (click)
    {
        case CLICK_SHORT_SET:
            window_manager_set_target(WINDOW_MENU) ;
            break;
        case CLICK_SHORT_UP:
            /* DVR 页面：UP 短按进入，BACK 返回（MODE 短按留给收音机）。 */
            window_manager_set_target(WINDOW_DVR);
            break;
        case CLICK_SHORT_DOWN:
            /* 收音机页面（骨架）：MODE 短按进入，BACK 返回。 */
            window_manager_set_target(WINDOW_RADIO);
            break;
        case CLICK_SUPER_LONG_UP:
            if(!checkout_time_out(UI_OTA_KEY_WAIT_TIMEOUT_MS))
            {
                vehicle_set_ota_page_state(1) ;
                window_manager_set_target(WINDOW_OTA) ;
            }
            else{
                lv_log("time out ！Please enter within 30 seconds\n") ;
            }
            
            break;
        case CLICK_LONG_SET:
            window_manager_set_target(WINDOW_MOBILE) ;
            break;
#ifdef LVGL_BUILD_CARPLAY
        case CLICK_LONG_UP:
            hcn_ui_shared_apps_open_carplay();
            break;
#endif
#ifdef HCN_APP_HAVE_BT
        case CLICK_LONG_DOWN:
            hcn_ui_shared_apps_open_bluetooth();
            break;
#endif
#ifdef HCN_APP_HAVE_MUSIC
        case CLICK_SHORT_BACK:
            /* 本地音乐：主界面 BACK 短按进入（虚拟按键为屏幕左侧中间）。 */
            hcn_ui_shared_apps_open_music();
            break;
#endif
        case CLICK_LONG_BACK:
            if(0 == speed_view_current() && window_manager_get() == WINDOW_MAIN)
                mileage_clear_trip();
            break;
        default:
            break;
    }
}

