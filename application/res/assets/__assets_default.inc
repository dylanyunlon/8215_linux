#include "awtk.h"
#include "base/assets_manager.h"
#ifdef STATIC_LOAD_RES
#ifndef WITH_FS_RES
#include "default/inc/ui/home_page.data"
#include "default/inc/strings/en_US.data"
#include "default/inc/strings/zh_CN.data"
#include "default/inc/styles/default.data"
#include "default/inc/styles/home_page.data"
#ifdef WITH_STB_IMAGE
#include "default/inc/images/abs_r.res"
#include "default/inc/images/background.res"
#include "default/inc/images/big_0.res"
#include "default/inc/images/big_1.res"
#include "default/inc/images/big_2.res"
#include "default/inc/images/big_3.res"
#include "default/inc/images/big_4.res"
#include "default/inc/images/big_5.res"
#include "default/inc/images/big_6.res"
#include "default/inc/images/big_7.res"
#include "default/inc/images/big_8.res"
#include "default/inc/images/big_9.res"
#include "default/inc/images/engine_oil_r.res"
#include "default/inc/images/engine_warn_r.res"
#include "default/inc/images/fuel.res"
#include "default/inc/images/fuel_r.res"
#include "default/inc/images/hazard_r.res"
#include "default/inc/images/high_light_y.res"
#include "default/inc/images/hot_r.res"
#include "default/inc/images/left_gs_0.res"
#include "default/inc/images/left_gs_1.res"
#include "default/inc/images/left_gs_2.res"
#include "default/inc/images/left_gs_3.res"
#include "default/inc/images/left_gs_4.res"
#include "default/inc/images/left_gs_5.res"
#include "default/inc/images/left_y.res"
#include "default/inc/images/low_light_y.res"
#include "default/inc/images/right_gs_0.res"
#include "default/inc/images/right_gs_1.res"
#include "default/inc/images/right_gs_2.res"
#include "default/inc/images/right_gs_3.res"
#include "default/inc/images/right_gs_4.res"
#include "default/inc/images/right_gs_5.res"
#include "default/inc/images/right_y.res"
#include "default/inc/images/small_0.res"
#include "default/inc/images/small_1.res"
#include "default/inc/images/small_2.res"
#include "default/inc/images/small_3.res"
#include "default/inc/images/small_4.res"
#include "default/inc/images/small_5.res"
#include "default/inc/images/small_6.res"
#include "default/inc/images/small_7.res"
#include "default/inc/images/small_8.res"
#include "default/inc/images/small_9.res"
#include "default/inc/images/temp.res"
#include "default/inc/images/temp_r.res"
#else
#endif /*WITH_STB_IMAGE*/
#ifdef WITH_VGCANVAS
#endif /*WITH_VGCANVAS*/
#ifdef WITH_TRUETYPE_FONT
#include "default/inc/fonts/default.res"
#else /*WITH_TRUETYPE_FONT*/
#endif /*WITH_TRUETYPE_FONT*/
#endif /*WITH_FS_RES*/
#endif

ret_t assets_init_default(void) {
  assets_manager_t* am = assets_manager();

#ifndef STATIC_LOAD_RES
  if (atc_awtk_load_res(am)) {
	log_error("assets load failed, please check!\n");
  }
#else
#ifdef WITH_FS_RES
  assets_manager_preload(am, ASSET_TYPE_STYLE, "default");
#else
  assets_manager_add(am, ui_home_page);
  assets_manager_add(am, strings_en_US);
  assets_manager_add(am, strings_zh_CN);
  assets_manager_add(am, style_default);
  assets_manager_add(am, style_home_page);
  assets_manager_add(am, image_abs_r);
  assets_manager_add(am, image_background);
  assets_manager_add(am, image_big_0);
  assets_manager_add(am, image_big_1);
  assets_manager_add(am, image_big_2);
  assets_manager_add(am, image_big_3);
  assets_manager_add(am, image_big_4);
  assets_manager_add(am, image_big_5);
  assets_manager_add(am, image_big_6);
  assets_manager_add(am, image_big_7);
  assets_manager_add(am, image_big_8);
  assets_manager_add(am, image_big_9);
  assets_manager_add(am, image_engine_oil_r);
  assets_manager_add(am, image_engine_warn_r);
  assets_manager_add(am, image_fuel);
  assets_manager_add(am, image_fuel_r);
  assets_manager_add(am, image_hazard_r);
  assets_manager_add(am, image_high_light_y);
  assets_manager_add(am, image_hot_r);
  assets_manager_add(am, image_left_gs_0);
  assets_manager_add(am, image_left_gs_1);
  assets_manager_add(am, image_left_gs_2);
  assets_manager_add(am, image_left_gs_3);
  assets_manager_add(am, image_left_gs_4);
  assets_manager_add(am, image_left_gs_5);
  assets_manager_add(am, image_left_y);
  assets_manager_add(am, image_low_light_y);
  assets_manager_add(am, image_right_gs_0);
  assets_manager_add(am, image_right_gs_1);
  assets_manager_add(am, image_right_gs_2);
  assets_manager_add(am, image_right_gs_3);
  assets_manager_add(am, image_right_gs_4);
  assets_manager_add(am, image_right_gs_5);
  assets_manager_add(am, image_right_y);
  assets_manager_add(am, image_small_0);
  assets_manager_add(am, image_small_1);
  assets_manager_add(am, image_small_2);
  assets_manager_add(am, image_small_3);
  assets_manager_add(am, image_small_4);
  assets_manager_add(am, image_small_5);
  assets_manager_add(am, image_small_6);
  assets_manager_add(am, image_small_7);
  assets_manager_add(am, image_small_8);
  assets_manager_add(am, image_small_9);
  assets_manager_add(am, image_temp);
  assets_manager_add(am, image_temp_r);
#ifdef WITH_VGCANVAS
#endif /*WITH_VGCANVAS*/
#ifdef WITH_TRUETYPE_FONT
  assets_manager_add(am, font_default);
#else /*WITH_TRUETYPE_FONT*/
#endif /*WITH_TRUETYPE_FONT*/
#endif
#endif

  tk_init_assets();
  return RET_OK;
}
