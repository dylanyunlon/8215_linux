/**
 * @file ui_shared_apps.h
 * @brief UI apps shared by the template and fallback variants.
 */

#ifndef UI_SHARED_APPS_H
#define UI_SHARED_APPS_H

#ifdef __cplusplus
extern "C" {
#endif

int hcn_ui_shared_apps_register(void);
int hcn_ui_shared_apps_init(void);
void hcn_ui_shared_apps_deinit(void);
void hcn_ui_shared_apps_open_bluetooth(void);
void hcn_ui_shared_apps_open_carplay(void);
void hcn_ui_shared_apps_open_music(void);   /* no-op unless built with HCN_APP_MUSIC */

#ifdef __cplusplus
}
#endif

#endif /* UI_SHARED_APPS_H */
