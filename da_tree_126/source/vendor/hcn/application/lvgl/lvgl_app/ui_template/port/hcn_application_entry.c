/*
 * PC simulator entry override.
 *
 * Provides lv_demo_widgets() to replace the simulator demo with ui_init():
 *   - resize the display to HCN_APPLICATION_SCREEN (default 1024x600);
 *   - call the generated ui_init();
 *   - start fake vehicle data when no CAN source is present.
 *
 * HCN_APPLICATION_SCREEN=480x272 switches to the dashboard layout.
 * lib-ui must be linked before the simulator demo library; see
 * HCN_APPLICATION_HOOK_LV_DEMO_WIDGETS in CMakeLists.txt.
 */

#include "lvgl.h"
#include "ui_init.h"
#include "ui_backend.h"
#include "window_manager.h"   /* HCN_BOOT_INTO_DVR target */

/* HCN_BOOT_INTO_DVR is defined once in hcn_boot_debug.h.
 * Apply the target after ui_backend_init(), when middleware is ready. */
#include "hcn_boot_debug.h"

#include <stdlib.h>

#define HCN_APPLICATION_SCREEN_HOR_RES 1024   /* 默认分辨率：收音页的设计尺寸 */
#define HCN_APPLICATION_SCREEN_VER_RES 600
#define HCN_APPLICATION_SCREEN_ENV     "HCN_APPLICATION_SCREEN"   /* "1024x600" / "480x272" */

/* 取屏幕分辨率：环境变量 HCN_APPLICATION_SCREEN=宽x高 优先（非法值忽略），否则用默认值。
 * 这样同一个 PC 模拟器既能跑 1024x600 的收音页，也能切回 480x272 的仪表页。 */
static void hcn_application_screen_size(int32_t *out_w, int32_t *out_h)
{
    const char *env = getenv(HCN_APPLICATION_SCREEN_ENV);
    char *end = NULL;
    long w;
    long h;

    *out_w = HCN_APPLICATION_SCREEN_HOR_RES;
    *out_h = HCN_APPLICATION_SCREEN_VER_RES;

    if (env == NULL || *env == 0)
        return;

    w = strtol(env, &end, 10);
    if (end == env || (*end != 'x' && *end != 'X'))
        return;
    h = strtol(end + 1, NULL, 10);

    /* 明显不合法的值（太小/太大）忽略，避免把窗口设成 0 或离谱尺寸 */
    if (w < 64 || h < 64 || w > 4096 || h > 4096) {
        LV_LOG_WARN("hcn_application: bad %s='%s', keep %dx%d",
                    HCN_APPLICATION_SCREEN_ENV, env, HCN_APPLICATION_SCREEN_HOR_RES, HCN_APPLICATION_SCREEN_VER_RES);
        return;
    }

    *out_w = (int32_t)w;
    *out_h = (int32_t)h;
    LV_LOG_USER("hcn_application: screen size from %s = %ldx%ld", HCN_APPLICATION_SCREEN_ENV, w, h);
}

/*
 * src/hal/hal.c:15-23 在 lv_display_get_layer_sys()（系统层，永远画在最上面）
 * 上创建了一个鼠标箭头贴图，并用 lv_indev_set_cursor() 挂到指针设备上，
 * 于是窗口里会一直跟着一个箭头。仪表界面用不到它，而且 LVGL 的
 * lv_indev_set_cursor() 不接受 NULL（无法"取消"），所以这里直接把那个对象隐藏。
 * 想从源头去掉：删掉 src/hal/hal.c 的第 15-23 行即可（那是既有文件，本工程不改）。
 */
static void hcn_application_hide_mouse_cursor(void)
{
    lv_indev_t * indev = NULL;

    while((indev = lv_indev_get_next(indev)) != NULL) {
        lv_obj_t * cursor = lv_indev_get_cursor(indev);
        if(cursor != NULL) lv_obj_set_hidden(cursor, true);
    }
}

void lv_demo_widgets(void)
{
    lv_display_t *disp;
    int32_t hor_res = 0;
    int32_t ver_res = 0;

    hcn_application_hide_mouse_cursor();
    hcn_application_screen_size(&hor_res, &ver_res);

    disp = lv_display_get_default();

    if(disp != NULL) {
        /* SDL 驱动会跟着 LV_EVENT_RESOLUTION_CHANGED 把窗口一起改大小 */
        lv_display_set_resolution(disp, hor_res, ver_res);
        LV_LOG_USER("hcn_application: display set to %dx%d", (int)hor_res, (int)ver_res);
        // lv_sdl_window_set_zoom(disp, 1.5f);
    }
    else {
        LV_LOG_WARN("hcn_application: no default display, ui_init() may not render");
    }

    ui_init();

    /* 交给当前后端做它自己的额外初始化：
     *   pc     -> 假车辆数据 + 假按键注入（platform/pc/ui_backend.c）
     *   device -> 引导中间件 mw_init()（platform/device/ui_backend.c）
     * 见 port/ui_backend.h。 */
    ui_backend_init();

#if HCN_BOOT_INTO_DVR && HCN_BOOT_DVR_ENTRY_EARLY
    /* Enter DVR through window_manager so BACK returns to the home page. */
    LV_LOG_USER("[临时] HCN_BOOT_INTO_DVR: 开机直进 DVR 页");
    window_manager_set_target(WINDOW_DVR);
#endif
}
