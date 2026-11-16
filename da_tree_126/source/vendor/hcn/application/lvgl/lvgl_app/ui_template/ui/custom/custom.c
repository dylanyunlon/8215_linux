#include "custom.h"
#include "lvgl.h"
#include "home_view_interface.h"
#include "vehicle_data.h"
#include "home_view_interface.h"
#include "button_manager.h"
#include "logic.h"
#include "window_manager.h"
#include "hcn_boot_debug.h"   /* boot target switches */
#include "selfcheck.h"        /* CHECK_STATE_FINISHED */
#include "dev_state.h"        /* ANIMATION_STATE_END */
#include "vehicle_services.h" /* vs_boot_anim_set() */

#define TOUCH_SCREEN

#if defined(TOUCH_SCREEN)
typedef struct
{
    lv_obj_t *btn_up  ;
    lv_obj_t *btn_down;
    lv_obj_t *btn_set ;
    lv_obj_t *btn_back;
}virtual_button_t;
static virtual_button_t vir_buttons = {0};


void on_btn_clicked(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_user_data(e);
    if (LV_EVENT_CLICKED == lv_event_get_code(e) )
    {
        if (obj == vir_buttons.btn_up)
        {
            button_manager_set_click(CLICK_SHORT_UP);
        }
        else if(obj == vir_buttons.btn_down)
        {
            button_manager_set_click(CLICK_SHORT_DOWN);
        }
        else if(obj == vir_buttons.btn_set)
        {
            button_manager_set_click(CLICK_SHORT_SET);
        }
        else if(obj == vir_buttons.btn_back)
        {
            button_manager_set_click(CLICK_SHORT_BACK );
        }
    }else if (LV_EVENT_LONG_PRESSED == lv_event_get_code(e))
    {
        lv_log("buttons long press!");
        if(obj == vir_buttons.btn_set)
            button_manager_set_click(CLICK_LONG_SET);
        else if(obj == vir_buttons.btn_up)
            button_manager_set_click(CLICK_LONG_UP);
        else if(obj == vir_buttons.btn_down)
            button_manager_set_click(CLICK_LONG_DOWN);
    }

}

void virtual_button_init()
{
    vir_buttons.btn_up   = lv_button_create(lv_layer_top()) ;
    vir_buttons.btn_down = lv_button_create(lv_layer_top()) ;
    vir_buttons.btn_set  = lv_button_create(lv_layer_top()) ;
    vir_buttons.btn_back = lv_button_create(lv_layer_top()) ;

    lv_obj_align(vir_buttons.btn_up , LV_ALIGN_TOP_MID ,0,0);
    lv_obj_align(vir_buttons.btn_down , LV_ALIGN_BOTTOM_MID ,0,0);
    lv_obj_align(vir_buttons.btn_set , LV_ALIGN_RIGHT_MID ,0,0);
    lv_obj_align(vir_buttons.btn_back , LV_ALIGN_LEFT_MID ,0,0);

    lv_obj_add_event_cb(vir_buttons.btn_up , on_btn_clicked ,LV_EVENT_ALL ,vir_buttons.btn_up );
    lv_obj_add_event_cb(vir_buttons.btn_down , on_btn_clicked ,LV_EVENT_ALL ,vir_buttons.btn_down );
    lv_obj_add_event_cb(vir_buttons.btn_set , on_btn_clicked ,LV_EVENT_ALL ,vir_buttons.btn_set );
    lv_obj_add_event_cb(vir_buttons.btn_back , on_btn_clicked ,LV_EVENT_ALL ,vir_buttons.btn_back );
}
#endif

void custom_init()
{
    #if !defined(AIC_STARTUP_UI_SHOW)
#if HCN_BOOT_SKIP_BOOT_ANIM
    /* Skip boot animation: force animation state to END so selfcheck can advance
     * and button_manager_refresh() remains active. */
    vs_boot_anim_set(ANIMATION_STATE_END);
#else
    //播放自定义开机动画
    extern void boot_animation_init() ;
    boot_animation_init() ;
#endif
    #endif

    /* Add your codes here */
    home_logic_init();

#if HCN_BOOT_SKIP_SELFCHECK
    /* Skip selfcheck: set FINISHED immediately; otherwise button_manager_refresh()
     * stays blocked for the 2.5 s check and input does not work. */
    checkself_set_state(CHECK_STATE_FINISHED);

    /* Normal selfcheck completion also moves the cartoon to READY and initializes
     * animation; skipping the check requires the same transition. */
    animation_set_state(CARTOON_STATE_READY);
#endif

    #if defined(TOUCH_SCREEN)
    virtual_button_init();
    #endif
}
