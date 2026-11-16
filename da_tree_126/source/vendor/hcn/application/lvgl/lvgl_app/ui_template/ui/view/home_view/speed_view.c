#include "speed_view.h"
#include "lvgl.h"
#include "aic_ui.h"
#include "vehicle_argument.h"
#include "animation_ctrl.h"
#include "selfcheck.h"

bool g_stop_state = true ;
static bool last_speed_zero = true;

void home_refresh_speed(uint32_t speed)
{
    static int lastSpeed = -1 ;
    if (lastSpeed == speed)
        return ;

    lastSpeed = speed ;

    if (checkself_get_state() == CHECK_STATE_FINISHED)
    {
      if(speed != 0 && g_stop_state)
        home_refresh_stop_state(false);

      
      if (speed == 0 && !last_speed_zero) 
      {
        animation_set_state(CARTOON_STATE_READY);
        last_speed_zero = true;
      } 
      else if (speed != 0 && last_speed_zero) 
      {
        animation_set_state(CARTOON_STATE_RUNNING);
        last_speed_zero = false;
      }

    }

    home_page_t *page = home_page_get(&ui_manager) ;

    VIEW_MEMBER_VALID(page, speed_lab)

    if (MPH == vehicle_get_param_unit())
        speed  *= KM_CONVERT_MILE ;

    lv_label_set_text_fmt(speed_lab, "%ld" , speed) ;

    return ;
}

void home_refresh_stop_state(bool istop)
{
    home_page_t *page = home_page_get(&ui_manager);
    VIEW_MEMBER_VALID(page, stop_image)
    VIEW_MEMBER_VALID(page, speed_lab)
    VIEW_MEMBER_VALID(page, speed_bg)
    VIEW_MEMBER_VALID(page, speed_unit)
    if(istop)
    {
        lv_obj_remove_flag(stop_image , LV_OBJ_FLAG_HIDDEN) ;
        lv_obj_add_flag(speed_lab ,LV_OBJ_FLAG_HIDDEN) ;
        lv_obj_add_flag(speed_unit , LV_OBJ_FLAG_HIDDEN) ;
        lv_image_set_src(speed_bg ,LVGL_IMAGE_PATH(bg_white.png) ) ;

        // animation_set_state(CARTOON_STATE_READY) ;
    }
    else
    {
        lv_obj_remove_flag(speed_lab , LV_OBJ_FLAG_HIDDEN) ;
        lv_obj_remove_flag(speed_unit , LV_OBJ_FLAG_HIDDEN) ;
        lv_obj_add_flag(stop_image ,LV_OBJ_FLAG_HIDDEN) ;
        lv_image_set_src(speed_bg ,LVGL_IMAGE_PATH(bg_golden.png) ) ;

        // animation_set_state(CARTOON_STATE_RUNNING) ;
    }

    g_stop_state = istop ;
    return ;
}

void home_refresh_unit(unit_e unit)
{
   char format_buff[128] = { 0 };
   lv_snprintf(format_buff , sizeof(format_buff) -1, "%s", (unit == KM_H) ? "unit_km_h" : "unit_mph");

   home_page_t *page = home_page_get(&ui_manager);
   VIEW_MEMBER_VALID(page, speed_unit)  //to do 切图

   lv_label_set_text_fmt(speed_unit , "%s" , unit == KM_H ? "km/h" : "mph") ;

   return  ;
}


typedef struct {
  int previous;
  int current;
  int data;
  int accumulated;
} data_accumulator_t;

static data_accumulator_t acc = {
    .current     = 0,
    .accumulated = 1,
    .previous    = 0,
};

static void run() {
  int target = acc.data;
  if (acc.previous != target) {
    int diff = abs(acc.previous - target);
    if (diff > 20) {
      acc.accumulated = diff / 5;
    } else {
      acc.accumulated = 1;
    }
    acc.previous = target;
  }

  int inc = acc.accumulated;
  if (acc.current < target) {
    acc.current = (acc.current + inc > target) ? target : acc.current + inc;
  } else if (acc.current > target) {
    acc.current = (acc.current - inc < target) ? target : acc.current - inc;
  }
}

static inline bool should_udpate(int target, int current) {
  return (target <= 2 || target != current);
}

void speed_view_run(int target) 
{
  acc.data = target;
  if (should_udpate(acc.data, acc.current))
    run();
}

int speed_view_current() {
  return acc.current;
}

void speed_view_set_current(int value) {
  acc.current = value;
}
