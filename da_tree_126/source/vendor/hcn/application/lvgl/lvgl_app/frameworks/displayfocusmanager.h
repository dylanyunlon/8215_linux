/**
 * @file displayfocusmanager.h
 * @brief 页面/显示焦点管理器（LIFO 栈）
 *
 * 每个 APP 对应一个 LVGL screen。屏幕的销毁策略由 APP 自身声明：
 *   - PERSISTENT：关闭后保留 screen，下次打开复用；
 *   - TRANSIENT ：关闭后调用 destroy_cb，并异步删除 screen。
 *
 * 关闭流程固定为：on_leave -> 出栈 -> 加载新栈顶 -> on_enter。
 * 当前 screen 的删除只允许异步执行，避免在按钮事件回调里同步析构。
 */

#ifndef DISPLAYFOCUSMANAGER_H
#define DISPLAYFOCUSMANAGER_H

#include <stdbool.h>
#include "app_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _lv_obj_t;
typedef struct _lv_obj_t lv_obj_t;

typedef enum {
    APP_SCREEN_PERSISTENT = 0,
    APP_SCREEN_TRANSIENT  = 1
} app_screen_policy_t;

typedef void (*app_init_cb_t)(lv_obj_t **screen);
typedef void (*app_destroy_cb_t)(void);
typedef void (*app_enter_cb_t)(void);
typedef void (*app_leave_cb_t)(void);
typedef void (*app_focus_change_cb_t)(app_id_t app_id, bool focused, void *user_data);

typedef struct {
    app_id_t app_id;
    app_priority_t priority;
    const char *name;
    app_init_cb_t init_cb;
    app_destroy_cb_t destroy_cb;
    app_screen_policy_t screen_policy;
    app_enter_cb_t on_enter;
    app_leave_cb_t on_leave;
} app_desc_t;

int displayfocusmanager_init(void);
void displayfocusmanager_deinit(void);

int displayfocusmanager_register_app(const app_desc_t *desc);
int displayfocusmanager_set_app_priority(app_id_t app_id, app_priority_t priority);
int displayfocusmanager_set_screen_policy(app_id_t app_id, app_screen_policy_t policy);
int displayfocusmanager_open_app(app_id_t app_id);
int displayfocusmanager_close_current_app(void);
int displayfocusmanager_close_app(app_id_t app_id);
int displayfocusmanager_go_home(void);

app_id_t displayfocusmanager_get_current_app(void);
int displayfocusmanager_get_stack_size(void);
bool displayfocusmanager_is_in_stack(app_id_t app_id);

void displayfocusmanager_register_focus_callback(app_id_t app_id, app_focus_change_cb_t cb, void *user_data);
void displayfocusmanager_unregister_focus_callback(app_id_t app_id);
void displayfocusmanager_dump(void);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAYFOCUSMANAGER_H */
