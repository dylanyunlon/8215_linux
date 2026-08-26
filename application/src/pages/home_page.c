#include "awtk.h"
#include "../common/navigator.h"

/**
 * 随机数int
 */
static int32_t get_random_int(int32_t min_num, int32_t max_num) {
  return fmod(rand(), max_num - min_num) + min_num;
}

/**
 * 关闭所有警告
 */
static void on_all_alarm_off(const timer_info_t* timer) {
  widget_t* win = WIDGET(timer->ctx);
  widget_t* alarm = NULL;
  
  alarm = widget_lookup(win, "engine_warn_r", TRUE);
  widget_set_visible(alarm, FALSE, FALSE);
  
  alarm = widget_lookup(win, "abs_r", TRUE);
  widget_set_visible(alarm, FALSE, FALSE);
  
  alarm = widget_lookup(win, "engine_oil_r", TRUE);
  widget_set_visible(alarm, FALSE, FALSE);
  
  alarm = widget_lookup(win, "hazard_r", TRUE);
  widget_set_visible(alarm, FALSE, FALSE);
  
  alarm = widget_lookup(win, "hot_r", TRUE);
  widget_set_visible(alarm, FALSE, FALSE);
}

/**
 * 显示左转灯
 */
static void on_left_on(widget_t* win) {
  widget_t* alarm = widget_lookup(win, "left", TRUE);
  widget_set_visible(alarm, TRUE, FALSE);
  
}

/**
 * 关闭左转灯
 */
static void on_left_off(widget_t* win) {
  widget_t* alarm = widget_lookup(win, "left", TRUE);
  widget_set_visible(alarm, FALSE, FALSE);
}

/**
 * 显示右转灯
 */
static void on_right_on(widget_t* win) {
  widget_t* alarm = widget_lookup(win, "right", TRUE);
  widget_set_visible(alarm, TRUE, FALSE);
}

/**
 * 关闭右转灯
 */
static void on_right_off(widget_t* win) {
  widget_t* alarm = widget_lookup(win, "right", TRUE);
  widget_set_visible(alarm, FALSE, FALSE);
}

/**
 * 模拟转向灯双闪
 */
static ret_t on_blkr_update(const timer_info_t* timer) {
  static bool_t alarm = FALSE;
  widget_t* win = WIDGET(timer->ctx);

  if (alarm) {
	alarm = 0;
    on_left_on(win);
	on_right_on(win);
  } else {
	alarm = 1;
    on_left_off(win);
	on_right_off(win);
  }

  return RET_REPEAT;
}


/**
 * 更新速度的状态
 */
static ret_t on_update_speed_status(const timer_info_t* timer) {
  widget_t* win = WIDGET(timer->ctx);
  widget_t* speed_single = NULL;
  widget_t* speed_unit = NULL;
  widget_t* speed_decade = NULL;
  widget_t* speed3[3] = {NULL};
  //char label_name[32] = {0};
  char style[10];
  static unsigned int speed = 0;

  //tk_snprintf(label_name, sizeof(label_name) - 1, "label%d", label_index);

  if (speed < 10) {
	  speed_single = widget_lookup(win, "speed_single", TRUE);
	  widget_set_visible(speed_single, TRUE, FALSE);
	  tk_snprintf(style, sizeof(style), "big_%d", speed);
	  widget_use_style(speed_single, style);
	  speed++;
  } else if (speed<100) {
	  speed_single = widget_lookup(win, "speed_single", TRUE);
	  widget_set_visible(speed_single, FALSE, FALSE);
	  
	  speed_unit = widget_lookup(win, "speed_unit", TRUE);
	  speed_decade = widget_lookup(win, "speed_decade", TRUE);
	  widget_set_visible(speed_unit, TRUE, FALSE);
	  widget_set_visible(speed_decade, TRUE, FALSE);
	  
	  tk_snprintf(style, sizeof(style), "big_%d", speed/10);
	  widget_use_style(speed_unit, style);
	  
	  tk_snprintf(style, sizeof(style), "big_%d", speed%10);
	  widget_use_style(speed_decade, style);
	  
	  speed++;
  } else if (speed<151) {
	  speed_unit = widget_lookup(win, "speed_unit", TRUE);
	  speed_decade = widget_lookup(win, "speed_decade", TRUE);
	  widget_set_visible(speed_unit, FALSE, FALSE);
	  widget_set_visible(speed_decade, FALSE, FALSE);
	  
	  speed3[0] = widget_lookup(win, "speed_3", TRUE);
	  speed3[1] = widget_lookup(win, "speed_2", TRUE);
	  speed3[2] = widget_lookup(win, "speed_1", TRUE);
	  widget_set_visible(speed3[0], TRUE, FALSE);
	  widget_set_visible(speed3[1], TRUE, FALSE);
	  widget_set_visible(speed3[2], TRUE, FALSE);
	  
	  tk_snprintf(style, sizeof(style), "big_%d", speed/100);
	  widget_use_style(speed3[0], style);
	  
	  tk_snprintf(style, sizeof(style), "big_%d", (speed/10)%10);
	  widget_use_style(speed3[1], style);
	  
	  tk_snprintf(style, sizeof(style), "big_%d", speed%10);
	  widget_use_style(speed3[2], style);
	  
	  speed++;
  } else {
	  speed3[0] = widget_lookup(win, "speed_1", TRUE);
	  speed3[1] = widget_lookup(win, "speed_2", TRUE);
	  speed3[2] = widget_lookup(win, "speed_3", TRUE);
	  widget_set_visible(speed3[0], FALSE, FALSE);
	  widget_set_visible(speed3[1], FALSE, FALSE);
	  widget_set_visible(speed3[2], FALSE, FALSE);
	  speed = 0;
  }

  return RET_REPEAT;
}

/**
 * 更新转速表的状态
 */
static ret_t on_update_rpm_status(const timer_info_t* timer) {
  widget_t* win = WIDGET(timer->ctx);
  widget_t* rpm_label = NULL;
  
  char style[10];
  static unsigned char rpm_speed = 0;

  //tk_snprintf(label_name, sizeof(label_name) - 1, "label%d", label_index);

  if (rpm_speed < 10) {
	  rpm_label = widget_lookup(win, "rpm", TRUE);
	  tk_snprintf(style, sizeof(style), "small_%d", rpm_speed);
	  widget_use_style(rpm_label, style);
	  rpm_speed++;
  } else {
	  rpm_speed = 0;
  }

  return RET_REPEAT;
}

/**
 * 更新油耗的状态
 */
static ret_t on_update_fuel_consumption_status(const timer_info_t* timer) {
  widget_t* win = WIDGET(timer->ctx);
  widget_t* l_km_label = NULL;
  
  char style[10];
  static unsigned char fuel_con = 0;

  //tk_snprintf(label_name, sizeof(label_name) - 1, "label%d", label_index);

  if (fuel_con < 10) {
	  l_km_label = widget_lookup(win, "l_km", TRUE);
	  tk_snprintf(style, sizeof(style), "small_%d", fuel_con);
	  widget_use_style(l_km_label, style);
	  fuel_con++;
  } else {
	  fuel_con = 0;
  }

  return RET_REPEAT;
}

/**
 * 更新左侧水温刻度表
 */
static ret_t on_update_left_gs_status(widget_t* win, unsigned char temp) {
  widget_t* left_gs_label[6] = {NULL};
  unsigned char label_cnt;
  unsigned char style[10];

  switch (temp) {
	  case 0:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "left_gs_%d", label_cnt);
			left_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			widget_set_visible(left_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
	  case 1:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "left_gs_%d", label_cnt);
			left_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			if (label_cnt < 1) {
				widget_set_visible(left_gs_label[label_cnt], TRUE, FALSE);
			} else 
				widget_set_visible(left_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
	  case 2:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "left_gs_%d", label_cnt);
			left_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			if (label_cnt < 2) {
				widget_set_visible(left_gs_label[label_cnt], TRUE, FALSE);
			} else 
				widget_set_visible(left_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
	  case 3:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "left_gs_%d", label_cnt);
			left_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			if (label_cnt < 3) {
				widget_set_visible(left_gs_label[label_cnt], TRUE, FALSE);
			} else 
				widget_set_visible(left_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
	  case 4:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "left_gs_%d", label_cnt);
			left_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			if (label_cnt < 4) {
				widget_set_visible(left_gs_label[label_cnt], TRUE, FALSE);
			} else 
				widget_set_visible(left_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
	  case 5:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "left_gs_%d", label_cnt);
			left_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			if (label_cnt < 5) {
				widget_set_visible(left_gs_label[label_cnt], TRUE, FALSE);
			} else 
				widget_set_visible(left_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
	  case 6:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "left_gs_%d", label_cnt);
			left_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			widget_set_visible(left_gs_label[label_cnt], TRUE, FALSE);
		}
	  break;
	  default:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "left_gs_%d", label_cnt);
			left_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			widget_set_visible(left_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
  }

  return RET_OK;
}

/**
 * 更新右侧油箱刻度表
 */
static ret_t on_update_right_gs_status(widget_t* win, unsigned char fuel) {
  widget_t* right_gs_label[6] = {NULL};
  unsigned char label_cnt;
  unsigned char style[12];

  switch (fuel) {
	  case 0:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "right_gs_%d", label_cnt);
			right_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			widget_set_visible(right_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
	  case 1:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "right_gs_%d", label_cnt);
			right_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			if (label_cnt < 1) {
				widget_set_visible(right_gs_label[label_cnt], TRUE, FALSE);
			} else 
				widget_set_visible(right_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
	  case 2:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "right_gs_%d", label_cnt);
			right_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			if (label_cnt < 2) {
				widget_set_visible(right_gs_label[label_cnt], TRUE, FALSE);
			} else 
				widget_set_visible(right_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
	  case 3:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "right_gs_%d", label_cnt);
			right_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			if (label_cnt < 3) {
				widget_set_visible(right_gs_label[label_cnt], TRUE, FALSE);
			} else 
				widget_set_visible(right_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
	  case 4:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "right_gs_%d", label_cnt);
			right_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			if (label_cnt < 4) {
				widget_set_visible(right_gs_label[label_cnt], TRUE, FALSE);
			} else 
				widget_set_visible(right_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
	  case 5:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "right_gs_%d", label_cnt);
			right_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			if (label_cnt < 5) {
				widget_set_visible(right_gs_label[label_cnt], TRUE, FALSE);
			} else 
				widget_set_visible(right_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
	  case 6:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "right_gs_%d", label_cnt);
			right_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			widget_set_visible(right_gs_label[label_cnt], TRUE, FALSE);
		}
	  break;
	  default:
		for (label_cnt=0;label_cnt<6;label_cnt++) {
			tk_snprintf(style, sizeof(style), "right_gs_%d", label_cnt);
			right_gs_label[label_cnt] = widget_lookup(win, style, TRUE);
			widget_set_visible(right_gs_label[label_cnt], FALSE, FALSE);
		}
	  break;
  }

  return RET_OK;
}

/**
 * 显示水温报警
 */
static void on_temp_alarm_on(widget_t* win) {
  widget_t* alarm = widget_lookup(win, "temp_r", TRUE);
  widget_set_visible(alarm, TRUE, FALSE);
  
}

/**
 * 关闭水温报警
 */
static void on_temp_alarm_off(widget_t* win) {
  widget_t* alarm = widget_lookup(win, "temp_r", TRUE);
  widget_set_visible(alarm, FALSE, FALSE);
}

/**
 * 显示油箱报警
 */
static void on_fuel_alarm_on(widget_t* win) {
  widget_t* alarm = widget_lookup(win, "fuel_r", TRUE);
  widget_set_visible(alarm, TRUE, FALSE);
  
}

/**
 * 关闭油箱报警
 */
static void on_fuel_alarm_off(widget_t* win) {
  widget_t* alarm = widget_lookup(win, "fuel_r", TRUE);
  widget_set_visible(alarm, FALSE, FALSE);
}

/**
 * 模拟左侧水温刻度表，并更新水温报警状态
 */
static ret_t on_update_water_temp_status(const timer_info_t* timer) {
  widget_t* win = WIDGET(timer->ctx);
  
  static unsigned char left_gs_cnt = 0;

  on_update_left_gs_status(win, left_gs_cnt);
  left_gs_cnt++;
  
  if (left_gs_cnt > 7) {
	  left_gs_cnt = 0;
	  on_temp_alarm_off(win);
  } else if (left_gs_cnt > 6) {
	  on_temp_alarm_on(win);
  }
	  
  return RET_REPEAT;
}

/**
 * 模拟右侧油箱刻度表，并更新油箱报警状态
 */
static ret_t on_update_fuel_status(const timer_info_t* timer) {
  widget_t* win = WIDGET(timer->ctx);
  
  static unsigned char right_gs_cnt = 6;

  on_update_right_gs_status(win, right_gs_cnt);
  
  if (right_gs_cnt < 2) {
	  on_fuel_alarm_on(win);
	  if (right_gs_cnt == 0)
		right_gs_cnt = 7;
  } else {
	  on_fuel_alarm_off(win);
  }
  
  
  right_gs_cnt--;
	  
  return RET_REPEAT;
}

/**
 * 显示远光灯
 */
static void on_high_light_on(widget_t* win) {
  widget_t* light = widget_lookup(win, "high_light_y", TRUE);
  widget_set_visible(light, TRUE, FALSE);
  
}

/**
 * 关闭远光灯
 */
static void on_high_light_off(widget_t* win) {
  widget_t* light = widget_lookup(win, "high_light_y", TRUE);
  widget_set_visible(light, FALSE, FALSE);
}

/**
 * 显示近光灯
 */
static void on_low_light_on(widget_t* win) {
  widget_t* light = widget_lookup(win, "low_light_y", TRUE);
  widget_set_visible(light, TRUE, FALSE);
}

/**
 * 关闭近光灯
 */
static void on_low_light_off(widget_t* win) {
  widget_t* light = widget_lookup(win, "low_light_y", TRUE);
  widget_set_visible(light, FALSE, FALSE);
}

/**
 * 模拟远近光灯切换
 */
static ret_t on_high_low_light_update(const timer_info_t* timer) {
  static bool_t light_flag = FALSE;
  widget_t* win = WIDGET(timer->ctx);

  if (light_flag) {
	light_flag = 0;
    on_high_light_off(win);
	on_low_light_on(win);
  } else {
	light_flag = 1;
    on_high_light_on(win);
	on_low_light_off(win);
  }

  return RET_REPEAT;
}

static ret_t on_button_pointer_up(void* ctx, event_t* e) {
  static unsigned char current_language = 0;
	
  pointer_event_t* evt = pointer_event_cast(e);
  // TODO: 在此添加控件事件处理程序代码
  if (current_language) {
	  current_language = 0;
	  locale_info_change(locale_info(), "zh", "CN");
  } else {
	  current_language = 1;
	  locale_info_change(locale_info(), "en", "US");
  }
  

  return RET_OK;
}

/**
 * 初始化窗口的子控件
 */
static ret_t visit_init_child(void* ctx, const void* iter) {
  widget_t* win = WIDGET(ctx);
  widget_t* widget = WIDGET(iter);
  const char* name = widget->name;

  // 初始化指定名称的控件（设置属性或注册事件），请保证控件名称在窗口上唯一
  if (name != NULL && *name != '\0') {
  	#if defined(LINUX) || defined(APPLE) || defined(HAS_STDIO) || defined(WINDOWS)
	if (tk_str_eq(name, "button")) {
	  widget_on(widget, EVT_POINTER_UP, on_button_pointer_up, win);
    }
	#endif
  }

  return RET_OK;
}

/**
 * 初始化窗口
 */
ret_t home_page_init(widget_t* win, void* ctx) {
  widget_t* button;
  (void)ctx;
  return_value_if_fail(win != NULL, RET_BAD_PARAMS);

  #if defined(LINUX) || defined(APPLE) || defined(HAS_STDIO) || defined(WINDOWS)
  button= button_create(win, 346, 13, 108, 43);
  widget_set_name(button, "button");
  widget_set_style_color(button, "normal:bg_color", 0x00000000);
  #endif

  widget_foreach(win, visit_init_child, win);
  
  timer_add(on_all_alarm_off, win, 2000);
  timer_add(on_blkr_update, win, 500);
  timer_add(on_update_speed_status, win, 20);
  timer_add(on_update_rpm_status, win, 500);
  timer_add(on_update_fuel_consumption_status, win, 1000);
  timer_add(on_update_water_temp_status, win, 1000);
  timer_add(on_update_fuel_status, win, 1000);
  timer_add(on_high_low_light_update, win, 1000);
  
  
  
  
  //widget_add_timer(win, on_update_speed_status, 500);

  return RET_OK;
}
