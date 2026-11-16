
/**
 * @file main
 *
 * HCN LVGL application entry.
 *   - display: LVGL + GLFW/EGL texture
 *   - full mode: vs_init -> dashboard_bridge -> hcn_ui_boot -> main loop
 *   - demo mode: LVGL UI only
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>
#include "lvgl/lvgl.h"
#include "ui.h"
#include "lvgl_app/frameworks/audio_settings.h"
#include "lvgl_app/frameworks/bootproflog.h"
#include <lvgl/src/drivers/evdev/lv_evdev.h>

#ifdef HCN_APP_FULL
/* full 模式：vehicle_services C 门面 + MVVM ViewModel（UI↔中间件数据交互层） */
#include "vehicle_services.h"
#include "MVVM/viewModel/dashboard_vm.h"
#include "lvgl_app/frameworks/dashboard_bridge.h"
#endif

/* 蓝牙适配层（HCN_APP_HAVE_BT 由顶层 CMake HCN_APP_BT 开关注入）：
 * 主循环协作轮询 bt_app_poll（事件队列消费）+ bt_focus_adapter_poll */
#ifdef HCN_APP_HAVE_BT
#include "bt_app.h"
#include "lvgl_app/frameworks/bt_focus_adapter.h"
#endif

/* 本地音乐适配层（HCN_APP_HAVE_MUSIC 由顶层 CMake HCN_APP_MUSIC 注入）：
 * 主循环协作轮询 music_adapter_poll —— music_app 的 idle_queue/timer 回调都在这里的 GUI 线程执行 */
#ifdef HCN_APP_HAVE_MUSIC
#include "music_adapter.h"
#endif

/* CarPlay 复合适配层（LVGL_BUILD_CARPLAY 由顶层 CMake HCN_APP_CARPLAY 注入） */
#ifdef LVGL_BUILD_CARPLAY
#include "carplayapp_if.h"
#endif

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void hcn_app_log_print(lv_log_level_t level, const char *buf);

/**********************
 *  VARIABLES
 **********************/
static lv_display_t *s_display = NULL;
extern lv_img_dsc_t mouse_cursor_icon;

static void hcn_app_log_print(lv_log_level_t level, const char *buf)
{
  (void)level;

  if(buf != NULL) {
    syslog(LOG_INFO, "%s", buf);
  }
}

static void hcn_app_set_mouse_cursor(lv_indev_t *indev)
{
  if(indev == NULL) {
    return;
  }

  lv_obj_t * cursor_obj = lv_image_create(lv_layer_top());
  lv_image_set_src(cursor_obj, &mouse_cursor_icon);
  lv_indev_set_cursor(indev, cursor_obj);
}

static void hcn_app_evdev_discovered(lv_indev_t *indev, lv_evdev_type_t type, void *user_data)
{
  LV_UNUSED(user_data);

  if(indev == NULL) {
    return;
  }

  if(s_display != NULL) {
    lv_indev_set_display(indev, s_display);
  }

  if(type == LV_EVDEV_TYPE_REL) {
    syslog(LOG_INFO, "[hcn-app] mouse device discovered");
    hcn_app_set_mouse_cursor(indev);
  } else if(type == LV_EVDEV_TYPE_ABS) {
    syslog(LOG_INFO, "[hcn-app] touch device discovered");
  }
}

static void getScreenSize(int *x , int *y) {

    int fb = open("/dev/fb0", O_RDONLY);
    if (fb < 0) {
        syslog(LOG_INFO, "[hcn-app] fb = open(/dev/fb0, O_RDONLY) failed");
        return;
    }
    struct fb_var_screeninfo vinfo;
    if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        syslog(LOG_INFO, "[hcn-app] ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) failed");
        close(fb);
        return;
    }

    syslog(LOG_INFO, "[hcn-app] getScreenSize Resolution width = %u  height = %u ", vinfo.xres, vinfo.yres);
    syslog(LOG_INFO, "[hcn-app] getScreenSize Virtual Resolution width = %u  height = %u ", vinfo.xres_virtual, vinfo.yres_virtual);
    *x = vinfo.xres;
    *y = vinfo.yres;

    close(fb);
}

int main(int argc, char **argv)
{
  (void)argc; /*Unused*/
  (void)argv; /*Unused*/

  /*Initialize LVGL*/
  writeBootProf("hcn_application main enter");
  openlog("hcn_application", LOG_PID, LOG_USER);
  writeBootProf("hcn_application before lv_init");
  lv_init();
  lv_log_register_print_cb(hcn_app_log_print);

#if 1
    writeBootProf("hcn_application before lv_glfw_window_create");
    /* create a window and initialize OpenGL */

    int width = 0 ;
    int height = 0;
    getScreenSize(&width, &height);

    lv_glfw_window_t * window = lv_glfw_window_create(width, height, false);

    /* create a display that flushes to a texture */
    lv_display_t * texture = lv_opengles_texture_create(width, height);
    lv_display_set_default(texture);
    s_display = texture;

    /* add the texture to the window */
    unsigned int texture_id = lv_opengles_texture_get_texture_id(texture);
    lv_glfw_texture_t * window_texture = lv_glfw_window_add_texture(window, texture_id, width, height);

    writeBootProf("hcn_application before lv_glfw_texture_get_mouse_indev");
    /* get the mouse indev of the window texture */
    lv_indev_t * mouse = lv_glfw_texture_get_mouse_indev(window_texture);
    if(mouse != NULL) {
      lv_indev_set_display(mouse, s_display);
      hcn_app_set_mouse_cursor(mouse);
    }
#else
    lv_display_t *display = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(display, "/dev/fb0");
    s_display = display;
#endif

#ifdef HCN_APP_FULL
  /* 启动数据通路：vs_init 引导中间件并注册 vehicle_param 回调；
   * dashboard_bridge 订阅 vs_signal_e，由主循环把变更写回 dashboard_model。
   * 置于 hcn_ui_boot 之前，保证 UI 首帧即有初始快照（vs_init 幂等）。 */
  writeBootProf("hcn_application before vs_init");
  if (vs_init() != 0) {
      syslog(LOG_ERR, "[hcn-app] vehicle services init failed, continue in UI-only mode");
  } else if (dashboard_bridge_init() != 0) {
      syslog(LOG_ERR, "[hcn-app] dashboard bridge init failed");
  }
#endif

  writeBootProf("hcn_application before ui_boot");

  if (hcn_ui_boot() != 0) {
      syslog(LOG_ERR, "[hcn-app] UI boot failed");
  }
  lv_evdev_discovery_start(hcn_app_evdev_discovered, NULL);

  writeBootProf("hcn_application done");
  audio_settings_init();
  fprintf(stderr, "[main] audio_settings_init returned\n");
  writeBootProf("audio_settings_init done");

  fprintf(stderr, "[main] enter ui loop\n");
  while(1) {
    /* Periodically call the lv_task handler.
     * It could be done in a timer interrupt or an OS task too.*/
    lv_timer_handler();
#ifdef HCN_APP_FULL
    /* MVVM 派发：将 dashboard_model 变更(CAN 线程写入置脏)冲刷到 LVGL View。
     * emitter 派发固定在 GUI 线程(本循环)执行。 */
    dashboard_bridge_poll();
    dashboard_vm_flush();
#endif
#ifdef HCN_APP_HAVE_BT
    /* 蓝牙：事件队列消费（SDK 回调线程 → GUI 线程快照合并）+ 焦点桥轮询 */
    bt_app_poll();
    bt_focus_adapter_poll();
#endif
#ifdef HCN_APP_HAVE_MUSIC
    /* 本地音乐：USB/扫描/播放线程投递的事件与定时器在 GUI 线程派发 */
    music_adapter_poll();
#endif
#ifdef LVGL_BUILD_CARPLAY
    {
      static int carplay_poll_logged = 0;
      if (!carplay_poll_logged) {
        fprintf(stderr, "[main] carplayapp_poll begin\n");
        carplay_poll_logged = 1;
      }
    }
    carplayapp_poll();
    {
      static int carplay_poll_done_logged = 0;
      if (!carplay_poll_done_logged) {
        fprintf(stderr, "[main] carplayapp_poll returned\n");
        carplay_poll_done_logged = 1;
      }
    }
#endif
    usleep(5 * 1000);
  }

#ifdef HCN_APP_HAVE_BT
    bt_app_deinit();
#endif
  hcn_ui_shutdown();
#ifdef HCN_APP_FULL
  dashboard_bridge_deinit();
  vs_deinit();
#endif
  closelog();

  return 0;
}
