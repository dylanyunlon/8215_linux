#include "awtk.h"
#include "base/assets_manager.h"
#ifndef WITH_FS_RES
extern TK_CONST_DATA_ALIGN(const unsigned char ui_component[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_device_page[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_home_page[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_link_page[]);
extern TK_CONST_DATA_ALIGN(const unsigned char ui_update_page[]);
extern TK_CONST_DATA_ALIGN(const unsigned char strings_en_US[]);
extern TK_CONST_DATA_ALIGN(const unsigned char strings_zh_CN[]);
#include "day/inc/styles/default.data"
#ifdef WITH_STB_IMAGE
#include "day/inc/images/bg_center.res"
#include "day/inc/images/bg_cricle.res"
#include "day/inc/images/bg_halo_0.res"
#include "day/inc/images/bg_halo_1.res"
#include "day/inc/images/bg_halo_2.res"
#include "day/inc/images/dock_bluetooth_n.res"
#include "day/inc/images/dock_bluetooth_p.res"
#include "day/inc/images/dock_instrument_n.res"
#include "day/inc/images/dock_instrument_p.res"
#include "day/inc/images/dock_music_n.res"
#include "day/inc/images/dock_music_p.res"
#include "day/inc/images/dock_navi_n.res"
#include "day/inc/images/dock_navi_p.res"
#include "day/inc/images/dock_settings_n.res"
#include "day/inc/images/dock_settings_p.res"
#include "day/inc/images/gear_d_n.res"
#include "day/inc/images/gear_d_p.res"
#include "day/inc/images/gear_n_n.res"
#include "day/inc/images/gear_n_p.res"
#include "day/inc/images/gear_r_n.res"
#include "day/inc/images/gear_r_p.res"
#include "day/inc/images/icon_nav_0.res"
#include "day/inc/images/icon_nav_1.res"
#include "day/inc/images/icon_nav_10.res"
#include "day/inc/images/icon_nav_11.res"
#include "day/inc/images/icon_nav_12.res"
#include "day/inc/images/icon_nav_13.res"
#include "day/inc/images/icon_nav_14.res"
#include "day/inc/images/icon_nav_15.res"
#include "day/inc/images/icon_nav_16.res"
#include "day/inc/images/icon_nav_17.res"
#include "day/inc/images/icon_nav_18.res"
#include "day/inc/images/icon_nav_19.res"
#include "day/inc/images/icon_nav_2.res"
#include "day/inc/images/icon_nav_20.res"
#include "day/inc/images/icon_nav_21.res"
#include "day/inc/images/icon_nav_22.res"
#include "day/inc/images/icon_nav_23.res"
#include "day/inc/images/icon_nav_24.res"
#include "day/inc/images/icon_nav_25.res"
#include "day/inc/images/icon_nav_26.res"
#include "day/inc/images/icon_nav_27.res"
#include "day/inc/images/icon_nav_28.res"
#include "day/inc/images/icon_nav_29.res"
#include "day/inc/images/icon_nav_3.res"
#include "day/inc/images/icon_nav_30.res"
#include "day/inc/images/icon_nav_31.res"
#include "day/inc/images/icon_nav_4.res"
#include "day/inc/images/icon_nav_5.res"
#include "day/inc/images/icon_nav_51.res"
#include "day/inc/images/icon_nav_52.res"
#include "day/inc/images/icon_nav_53.res"
#include "day/inc/images/icon_nav_54.res"
#include "day/inc/images/icon_nav_55.res"
#include "day/inc/images/icon_nav_56.res"
#include "day/inc/images/icon_nav_57.res"
#include "day/inc/images/icon_nav_58.res"
#include "day/inc/images/icon_nav_59.res"
#include "day/inc/images/icon_nav_6.res"
#include "day/inc/images/icon_nav_60.res"
#include "day/inc/images/icon_nav_61.res"
#include "day/inc/images/icon_nav_62.res"
#include "day/inc/images/icon_nav_63.res"
#include "day/inc/images/icon_nav_64.res"
#include "day/inc/images/icon_nav_65.res"
#include "day/inc/images/icon_nav_66.res"
#include "day/inc/images/icon_nav_67.res"
#include "day/inc/images/icon_nav_68.res"
#include "day/inc/images/icon_nav_69.res"
#include "day/inc/images/icon_nav_7.res"
#include "day/inc/images/icon_nav_70.res"
#include "day/inc/images/icon_nav_71.res"
#include "day/inc/images/icon_nav_72.res"
#include "day/inc/images/icon_nav_73.res"
#include "day/inc/images/icon_nav_74.res"
#include "day/inc/images/icon_nav_75.res"
#include "day/inc/images/icon_nav_76.res"
#include "day/inc/images/icon_nav_77.res"
#include "day/inc/images/icon_nav_78.res"
#include "day/inc/images/icon_nav_79.res"
#include "day/inc/images/icon_nav_8.res"
#include "day/inc/images/icon_nav_80.res"
#include "day/inc/images/icon_nav_9.res"
#include "day/inc/images/icon_next_n.res"
#include "day/inc/images/icon_next_p.res"
#include "day/inc/images/icon_pause_n.res"
#include "day/inc/images/icon_pause_p.res"
#include "day/inc/images/icon_play_n.res"
#include "day/inc/images/icon_play_p.res"
#include "day/inc/images/icon_pre_n.res"
#include "day/inc/images/icon_pre_p.res"
#include "day/inc/images/icon_riding_distance.res"
#include "day/inc/images/icon_riding_time.res"
#include "day/inc/images/left_bg_n.res"
#include "day/inc/images/left_bg_p.res"
#include "day/inc/images/music_bar.res"
#include "day/inc/images/music_bar_bg.res"
#include "day/inc/images/music_bar_bg_n.res"
#include "day/inc/images/pointer_0.res"
#include "day/inc/images/pointer_1.res"
#include "day/inc/images/pointer_2.res"
#include "day/inc/images/power_0.res"
#include "day/inc/images/power_1.res"
#include "day/inc/images/power_2.res"
#include "day/inc/images/power_3.res"
#include "day/inc/images/power_4.res"
#include "day/inc/images/power_5.res"
#include "day/inc/images/power_6.res"
#include "day/inc/images/power_7.res"
#include "day/inc/images/power_8.res"
#include "day/inc/images/power_9.res"
#include "day/inc/images/power_bg.res"
#include "day/inc/images/power_level_bg.res"
#include "day/inc/images/power_level_n.res"
#include "day/inc/images/power_level_p.res"
#include "day/inc/images/power_pointer.res"
#include "day/inc/images/power_progress.res"
#include "day/inc/images/setting_bg.res"
#include "day/inc/images/setting_line.res"
#include "day/inc/images/setting_menu_selec.res"
#include "day/inc/images/setting_page_down_n.res"
#include "day/inc/images/setting_page_up_n.res"
#include "day/inc/images/setting_selected.res"
#include "day/inc/images/setting_tire_pressure_bg.res"
#include "day/inc/images/speed_0.res"
#include "day/inc/images/speed_1.res"
#include "day/inc/images/speed_2.res"
#include "day/inc/images/speed_3.res"
#include "day/inc/images/speed_4.res"
#include "day/inc/images/speed_5.res"
#include "day/inc/images/speed_6.res"
#include "day/inc/images/speed_7.res"
#include "day/inc/images/speed_8.res"
#include "day/inc/images/speed_9.res"
#include "day/inc/images/top_abs_n.res"
#include "day/inc/images/top_auto_high_beams_n.res"
#include "day/inc/images/top_bluetooth_n.res"
#include "day/inc/images/top_brake_failure_n.res"
#include "day/inc/images/top_charge_n.res"
#include "day/inc/images/top_ecu_n.res"
#include "day/inc/images/top_gps_n.res"
#include "day/inc/images/top_high_beam_n.res"
#include "day/inc/images/top_high_temperature_n.res"
#include "day/inc/images/top_left.res"
#include "day/inc/images/top_left_n.res"
#include "day/inc/images/top_lock_n.res"
#include "day/inc/images/top_low_beam_n.res"
#include "day/inc/images/top_low_temperature_n.res"
#include "day/inc/images/top_motor_failure_n.res"
#include "day/inc/images/top_ready.res"
#include "day/inc/images/top_ready_n.res"
#include "day/inc/images/top_right.res"
#include "day/inc/images/top_right_n.res"
#include "day/inc/images/top_signal_0_n.res"
#include "day/inc/images/top_signal_1_n.res"
#include "day/inc/images/top_signal_2_n.res"
#include "day/inc/images/top_signal_3_n.res"
#include "day/inc/images/top_system_failure_n.res"
#include "day/inc/images/top_tcs_n.res"
#include "day/inc/images/unit_km_h.res"
#include "day/inc/images/unit_mph.res"
#include "day/inc/images/wallpaper.res"
extern TK_CONST_DATA_ALIGN(const unsigned char image_box[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_box_power_level_100_[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_box_power_level_10_[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_box_power_level_25_[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_box_power_level_50_[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_box_power_level_75_[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_box_power_pointer[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_box_power_progress_n[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_box_power_progress_p[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_call_huang[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_call_pick[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_dashboard_progress_0[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_dashboard_progress_1[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_dashboard_progress_2[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_dog[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_drv_mode_0[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_drv_mode_1[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_drv_mode_2[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_icon_answer[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_icon_call[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_icon_incoming_call[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_icon_media_cover_n[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_icon_outgoing_call[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_icon_tmps_n[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_icon_tmps_w[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_icon_WiFi[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_interconnection_bg[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_linkgear_d_n[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_linkgear_d_p[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_linkgear_n_n[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_linkgear_n_p[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_linkgear_r_n[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_linkgear_r_p[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_link_elecRect_head[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_progress_n[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_progress_p[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_setting_bluebooth_close[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_setting_bluebooth_open[]);
extern TK_CONST_DATA_ALIGN(const unsigned char image_top_tire_pressure_n[]);
#else
#endif /*WITH_STB_IMAGE*/
#ifdef WITH_VGCANVAS
#endif /*WITH_VGCANVAS*/
#ifdef WITH_TRUETYPE_FONT
extern TK_CONST_DATA_ALIGN(const unsigned char font_default[]);
extern TK_CONST_DATA_ALIGN(const unsigned char font_default_2[]);
#else /*WITH_TRUETYPE_FONT*/
#endif /*WITH_TRUETYPE_FONT*/
#endif /*WITH_FS_RES*/

ret_t assets_init_day(void) {
  assets_manager_t* am = assets_manager();

#ifdef WITH_FS_RES
  assets_manager_preload(am, ASSET_TYPE_STYLE, "default");
#else
  assets_manager_add(am, ui_component);
  assets_manager_add(am, ui_device_page);
  assets_manager_add(am, ui_home_page);
  assets_manager_add(am, ui_link_page);
  assets_manager_add(am, ui_update_page);
  assets_manager_add(am, strings_en_US);
  assets_manager_add(am, strings_zh_CN);
  assets_manager_add(am, style_default_day);
  assets_manager_add(am, image_bg_center_day);
  assets_manager_add(am, image_bg_cricle_day);
  assets_manager_add(am, image_bg_halo_0_day);
  assets_manager_add(am, image_bg_halo_1_day);
  assets_manager_add(am, image_bg_halo_2_day);
  assets_manager_add(am, image_dock_bluetooth_n_day);
  assets_manager_add(am, image_dock_bluetooth_p_day);
  assets_manager_add(am, image_dock_instrument_n_day);
  assets_manager_add(am, image_dock_instrument_p_day);
  assets_manager_add(am, image_dock_music_n_day);
  assets_manager_add(am, image_dock_music_p_day);
  assets_manager_add(am, image_dock_navi_n_day);
  assets_manager_add(am, image_dock_navi_p_day);
  assets_manager_add(am, image_dock_settings_n_day);
  assets_manager_add(am, image_dock_settings_p_day);
  assets_manager_add(am, image_gear_d_n_day);
  assets_manager_add(am, image_gear_d_p_day);
  assets_manager_add(am, image_gear_n_n_day);
  assets_manager_add(am, image_gear_n_p_day);
  assets_manager_add(am, image_gear_r_n_day);
  assets_manager_add(am, image_gear_r_p_day);
  assets_manager_add(am, image_icon_nav_0_day);
  assets_manager_add(am, image_icon_nav_1_day);
  assets_manager_add(am, image_icon_nav_10_day);
  assets_manager_add(am, image_icon_nav_11_day);
  assets_manager_add(am, image_icon_nav_12_day);
  assets_manager_add(am, image_icon_nav_13_day);
  assets_manager_add(am, image_icon_nav_14_day);
  assets_manager_add(am, image_icon_nav_15_day);
  assets_manager_add(am, image_icon_nav_16_day);
  assets_manager_add(am, image_icon_nav_17_day);
  assets_manager_add(am, image_icon_nav_18_day);
  assets_manager_add(am, image_icon_nav_19_day);
  assets_manager_add(am, image_icon_nav_2_day);
  assets_manager_add(am, image_icon_nav_20_day);
  assets_manager_add(am, image_icon_nav_21_day);
  assets_manager_add(am, image_icon_nav_22_day);
  assets_manager_add(am, image_icon_nav_23_day);
  assets_manager_add(am, image_icon_nav_24_day);
  assets_manager_add(am, image_icon_nav_25_day);
  assets_manager_add(am, image_icon_nav_26_day);
  assets_manager_add(am, image_icon_nav_27_day);
  assets_manager_add(am, image_icon_nav_28_day);
  assets_manager_add(am, image_icon_nav_29_day);
  assets_manager_add(am, image_icon_nav_3_day);
  assets_manager_add(am, image_icon_nav_30_day);
  assets_manager_add(am, image_icon_nav_31_day);
  assets_manager_add(am, image_icon_nav_4_day);
  assets_manager_add(am, image_icon_nav_5_day);
  assets_manager_add(am, image_icon_nav_51_day);
  assets_manager_add(am, image_icon_nav_52_day);
  assets_manager_add(am, image_icon_nav_53_day);
  assets_manager_add(am, image_icon_nav_54_day);
  assets_manager_add(am, image_icon_nav_55_day);
  assets_manager_add(am, image_icon_nav_56_day);
  assets_manager_add(am, image_icon_nav_57_day);
  assets_manager_add(am, image_icon_nav_58_day);
  assets_manager_add(am, image_icon_nav_59_day);
  assets_manager_add(am, image_icon_nav_6_day);
  assets_manager_add(am, image_icon_nav_60_day);
  assets_manager_add(am, image_icon_nav_61_day);
  assets_manager_add(am, image_icon_nav_62_day);
  assets_manager_add(am, image_icon_nav_63_day);
  assets_manager_add(am, image_icon_nav_64_day);
  assets_manager_add(am, image_icon_nav_65_day);
  assets_manager_add(am, image_icon_nav_66_day);
  assets_manager_add(am, image_icon_nav_67_day);
  assets_manager_add(am, image_icon_nav_68_day);
  assets_manager_add(am, image_icon_nav_69_day);
  assets_manager_add(am, image_icon_nav_7_day);
  assets_manager_add(am, image_icon_nav_70_day);
  assets_manager_add(am, image_icon_nav_71_day);
  assets_manager_add(am, image_icon_nav_72_day);
  assets_manager_add(am, image_icon_nav_73_day);
  assets_manager_add(am, image_icon_nav_74_day);
  assets_manager_add(am, image_icon_nav_75_day);
  assets_manager_add(am, image_icon_nav_76_day);
  assets_manager_add(am, image_icon_nav_77_day);
  assets_manager_add(am, image_icon_nav_78_day);
  assets_manager_add(am, image_icon_nav_79_day);
  assets_manager_add(am, image_icon_nav_8_day);
  assets_manager_add(am, image_icon_nav_80_day);
  assets_manager_add(am, image_icon_nav_9_day);
  assets_manager_add(am, image_icon_next_n_day);
  assets_manager_add(am, image_icon_next_p_day);
  assets_manager_add(am, image_icon_pause_n_day);
  assets_manager_add(am, image_icon_pause_p_day);
  assets_manager_add(am, image_icon_play_n_day);
  assets_manager_add(am, image_icon_play_p_day);
  assets_manager_add(am, image_icon_pre_n_day);
  assets_manager_add(am, image_icon_pre_p_day);
  assets_manager_add(am, image_icon_riding_distance_day);
  assets_manager_add(am, image_icon_riding_time_day);
  assets_manager_add(am, image_left_bg_n_day);
  assets_manager_add(am, image_left_bg_p_day);
  assets_manager_add(am, image_music_bar_day);
  assets_manager_add(am, image_music_bar_bg_day);
  assets_manager_add(am, image_music_bar_bg_n_day);
  assets_manager_add(am, image_pointer_0_day);
  assets_manager_add(am, image_pointer_1_day);
  assets_manager_add(am, image_pointer_2_day);
  assets_manager_add(am, image_power_0_day);
  assets_manager_add(am, image_power_1_day);
  assets_manager_add(am, image_power_2_day);
  assets_manager_add(am, image_power_3_day);
  assets_manager_add(am, image_power_4_day);
  assets_manager_add(am, image_power_5_day);
  assets_manager_add(am, image_power_6_day);
  assets_manager_add(am, image_power_7_day);
  assets_manager_add(am, image_power_8_day);
  assets_manager_add(am, image_power_9_day);
  assets_manager_add(am, image_power_bg_day);
  assets_manager_add(am, image_power_level_bg_day);
  assets_manager_add(am, image_power_level_n_day);
  assets_manager_add(am, image_power_level_p_day);
  assets_manager_add(am, image_power_pointer_day);
  assets_manager_add(am, image_power_progress_day);
  assets_manager_add(am, image_setting_bg_day);
  assets_manager_add(am, image_setting_line_day);
  assets_manager_add(am, image_setting_menu_selec_day);
  assets_manager_add(am, image_setting_page_down_n_day);
  assets_manager_add(am, image_setting_page_up_n_day);
  assets_manager_add(am, image_setting_selected_day);
  assets_manager_add(am, image_setting_tire_pressure_bg_day);
  assets_manager_add(am, image_speed_0_day);
  assets_manager_add(am, image_speed_1_day);
  assets_manager_add(am, image_speed_2_day);
  assets_manager_add(am, image_speed_3_day);
  assets_manager_add(am, image_speed_4_day);
  assets_manager_add(am, image_speed_5_day);
  assets_manager_add(am, image_speed_6_day);
  assets_manager_add(am, image_speed_7_day);
  assets_manager_add(am, image_speed_8_day);
  assets_manager_add(am, image_speed_9_day);
  assets_manager_add(am, image_top_abs_n_day);
  assets_manager_add(am, image_top_auto_high_beams_n_day);
  assets_manager_add(am, image_top_bluetooth_n_day);
  assets_manager_add(am, image_top_brake_failure_n_day);
  assets_manager_add(am, image_top_charge_n_day);
  assets_manager_add(am, image_top_ecu_n_day);
  assets_manager_add(am, image_top_gps_n_day);
  assets_manager_add(am, image_top_high_beam_n_day);
  assets_manager_add(am, image_top_high_temperature_n_day);
  assets_manager_add(am, image_top_left_day);
  assets_manager_add(am, image_top_left_n_day);
  assets_manager_add(am, image_top_lock_n_day);
  assets_manager_add(am, image_top_low_beam_n_day);
  assets_manager_add(am, image_top_low_temperature_n_day);
  assets_manager_add(am, image_top_motor_failure_n_day);
  assets_manager_add(am, image_top_ready_day);
  assets_manager_add(am, image_top_ready_n_day);
  assets_manager_add(am, image_top_right_day);
  assets_manager_add(am, image_top_right_n_day);
  assets_manager_add(am, image_top_signal_0_n_day);
  assets_manager_add(am, image_top_signal_1_n_day);
  assets_manager_add(am, image_top_signal_2_n_day);
  assets_manager_add(am, image_top_signal_3_n_day);
  assets_manager_add(am, image_top_system_failure_n_day);
  assets_manager_add(am, image_top_tcs_n_day);
  assets_manager_add(am, image_unit_km_h_day);
  assets_manager_add(am, image_unit_mph_day);
  assets_manager_add(am, image_wallpaper_day);
  assets_manager_add(am, image_box);
  assets_manager_add(am, image_box_power_level_100_);
  assets_manager_add(am, image_box_power_level_10_);
  assets_manager_add(am, image_box_power_level_25_);
  assets_manager_add(am, image_box_power_level_50_);
  assets_manager_add(am, image_box_power_level_75_);
  assets_manager_add(am, image_box_power_pointer);
  assets_manager_add(am, image_box_power_progress_n);
  assets_manager_add(am, image_box_power_progress_p);
  assets_manager_add(am, image_call_huang);
  assets_manager_add(am, image_call_pick);
  assets_manager_add(am, image_dashboard_progress_0);
  assets_manager_add(am, image_dashboard_progress_1);
  assets_manager_add(am, image_dashboard_progress_2);
  assets_manager_add(am, image_dog);
  assets_manager_add(am, image_drv_mode_0);
  assets_manager_add(am, image_drv_mode_1);
  assets_manager_add(am, image_drv_mode_2);
  assets_manager_add(am, image_icon_answer);
  assets_manager_add(am, image_icon_call);
  assets_manager_add(am, image_icon_incoming_call);
  assets_manager_add(am, image_icon_media_cover_n);
  assets_manager_add(am, image_icon_outgoing_call);
  assets_manager_add(am, image_icon_tmps_n);
  assets_manager_add(am, image_icon_tmps_w);
  assets_manager_add(am, image_icon_WiFi);
  assets_manager_add(am, image_interconnection_bg);
  assets_manager_add(am, image_linkgear_d_n);
  assets_manager_add(am, image_linkgear_d_p);
  assets_manager_add(am, image_linkgear_n_n);
  assets_manager_add(am, image_linkgear_n_p);
  assets_manager_add(am, image_linkgear_r_n);
  assets_manager_add(am, image_linkgear_r_p);
  assets_manager_add(am, image_link_elecRect_head);
  assets_manager_add(am, image_progress_n);
  assets_manager_add(am, image_progress_p);
  assets_manager_add(am, image_setting_bluebooth_close);
  assets_manager_add(am, image_setting_bluebooth_open);
  assets_manager_add(am, image_top_tire_pressure_n);
#ifdef WITH_VGCANVAS
#endif /*WITH_VGCANVAS*/
#ifdef WITH_TRUETYPE_FONT
  assets_manager_add(am, font_default);
  assets_manager_add(am, font_default_2);
#else /*WITH_TRUETYPE_FONT*/
#endif /*WITH_TRUETYPE_FONT*/
#endif

  tk_init_assets();
  return RET_OK;
}
