/**
 * @file ui.c
 * @brief Fallback UI variant: SquareLine home and dashboard view.
 */

#include "ui.h"

#ifdef HCN_APP_FULL
#include "screens/dashboard_view.h"
#endif

static void home_app_init_cb(lv_obj_t **screen)
{
    ui_Home_screen_init();
    *screen = ui_Home;

#ifdef HCN_APP_FULL
    dashboard_view_attach(ui_Home);
#endif
}

int hcn_ui_fallback_init(void)
{
    const app_desc_t home_desc = {
        .app_id = APP_HOME,
        .priority = APP_PRIORITY_SYSTEM,
        .name = "Home",
        .init_cb = home_app_init_cb,
        .destroy_cb = NULL,
        .screen_policy = APP_SCREEN_PERSISTENT,
    };

    if (displayfocusmanager_register_app(&home_desc) != APP_OK) {
        return -1;
    }
    if (displayfocusmanager_open_app(APP_HOME) != APP_OK) {
        return -1;
    }
    return 0;
}

void hcn_ui_fallback_deinit(void)
{
    ui_Home = NULL;
}
