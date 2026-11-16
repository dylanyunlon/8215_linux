/**
 * @file displayfocusmanager.c
 * @brief 页面/显示焦点管理器实现
 */

#include "displayfocusmanager.h"
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <string.h>

#define MAX_APPS         32
#define MAX_STACK_DEPTH  32
#define MAX_FOCUS_CBS    8

typedef struct {
    app_id_t app_id;
    app_priority_t priority;
    const char *name;
    app_init_cb_t init_cb;
    app_destroy_cb_t destroy_cb;
    app_screen_policy_t screen_policy;
    app_enter_cb_t on_enter;
    app_leave_cb_t on_leave;
    lv_obj_t *screen;
    bool initialized;
} app_reg_t;

typedef struct {
    app_id_t app_id;
    app_focus_change_cb_t cb;
    void *user_data;
} focus_cb_entry_t;

typedef struct {
    app_reg_t apps[MAX_APPS];
    int app_count;
    app_id_t stack[MAX_STACK_DEPTH];
    int stack_size;
    focus_cb_entry_t focus_cbs[MAX_FOCUS_CBS];
    int focus_cb_count;
    bool initialized;
} displayfocus_manager_t;

static displayfocus_manager_t g_dfm;

static app_reg_t *find_app_by_id(app_id_t app_id)
{
    for (int i = 0; i < g_dfm.app_count; i++) {
        if (g_dfm.apps[i].app_id == app_id) {
            return &g_dfm.apps[i];
        }
    }
    return NULL;
}

static int find_in_stack(app_id_t app_id)
{
    for (int i = 0; i < g_dfm.stack_size; i++) {
        if (g_dfm.stack[i] == app_id) {
            return i;
        }
    }
    return -1;
}

static app_reg_t *get_current_app_reg(void)
{
    if (g_dfm.stack_size == 0) {
        return NULL;
    }
    return find_app_by_id(g_dfm.stack[g_dfm.stack_size - 1]);
}

static void notify_focus_change(app_id_t app_id, bool focused)
{
    for (int i = 0; i < g_dfm.focus_cb_count; i++) {
        if (g_dfm.focus_cbs[i].cb && g_dfm.focus_cbs[i].app_id == app_id) {
            g_dfm.focus_cbs[i].cb(app_id, focused, g_dfm.focus_cbs[i].user_data);
            return;
        }
    }
}

static void call_on_enter(app_reg_t *app)
{
    if (app && app->on_enter) {
        app->on_enter();
    }
}

static void call_on_leave(app_reg_t *app)
{
    if (app && app->on_leave) {
        app->on_leave();
    }
}

static int ensure_app_initialized(app_reg_t *app)
{
    lv_obj_t *screen = NULL;

    if (!app) {
        return APP_ERR_PARAM;
    }
    if (app->initialized && app->screen) {
        return APP_OK;
    }
    if (!app->init_cb) {
        printf("[display] null init_cb for app id=%d\n", (int)app->app_id);
        return APP_ERR_PARAM;
    }

    app->init_cb(&screen);
    if (!screen) {
        printf("[display] init_cb for %s(id=%d) returned NULL screen\n",
               app->name ? app->name : "?", (int)app->app_id);
        return APP_ERR_PARAM;
    }

    app->screen = screen;
    app->initialized = true;
    return APP_OK;
}

static void destroy_app_resources(app_reg_t *app, bool async)
{
    lv_obj_t *screen;

    if (!app || !app->initialized) {
        return;
    }

    if (app->destroy_cb) {
        app->destroy_cb();
    }

    screen = app->screen;
    app->screen = NULL;
    app->initialized = false;

    if (screen) {
        if (async) {
            lv_obj_delete_async(screen);
        } else {
            lv_obj_delete(screen);
        }
    }
}

static void remove_stack_pos(int pos)
{
    if (pos < 0 || pos >= g_dfm.stack_size) {
        return;
    }
    for (int i = pos; i < g_dfm.stack_size - 1; i++) {
        g_dfm.stack[i] = g_dfm.stack[i + 1];
    }
    g_dfm.stack_size--;
}

static void push_stack(app_id_t app_id)
{
    g_dfm.stack[g_dfm.stack_size] = app_id;
    g_dfm.stack_size++;
}

static void move_stack_entry_to_top(int pos)
{
    app_id_t app_id;

    if (pos < 0 || pos >= g_dfm.stack_size - 1) {
        return;
    }
    app_id = g_dfm.stack[pos];
    remove_stack_pos(pos);
    push_stack(app_id);
}

static void load_app_screen(app_reg_t *app)
{
    if (app && app->screen) {
        lv_screen_load(app->screen);
    }
}

int displayfocusmanager_init(void)
{
    if (g_dfm.initialized) {
        return APP_OK;
    }
    memset(&g_dfm, 0, sizeof(g_dfm));
    g_dfm.initialized = true;
    return APP_OK;
}

void displayfocusmanager_deinit(void)
{
    app_reg_t *current;

    if (!g_dfm.initialized) {
        return;
    }

    current = get_current_app_reg();
    if (current) {
        call_on_leave(current);
        notify_focus_change(current->app_id, false);
    }

    for (int i = 0; i < g_dfm.app_count; i++) {
        app_reg_t *app = &g_dfm.apps[i];
        if (app->initialized) {
            if (app != current) {
                call_on_leave(app);
            }
            destroy_app_resources(app, false);
        }
    }

    memset(&g_dfm, 0, sizeof(g_dfm));
}

int displayfocusmanager_register_app(const app_desc_t *desc)
{
    app_reg_t *reg;

    if (!g_dfm.initialized) {
        return APP_ERR_INIT;
    }
    if (!desc || !desc->init_cb) {
        return APP_ERR_PARAM;
    }

    reg = find_app_by_id(desc->app_id);
    if (reg) {
        reg->priority = desc->priority;
        reg->name = desc->name;
        reg->init_cb = desc->init_cb;
        reg->destroy_cb = desc->destroy_cb;
        reg->screen_policy = desc->screen_policy;
        reg->on_enter = desc->on_enter;
        reg->on_leave = desc->on_leave;
        return APP_OK;
    }

    if (g_dfm.app_count >= MAX_APPS) {
        return APP_ERR_FULL;
    }

    reg = &g_dfm.apps[g_dfm.app_count];
    reg->app_id = desc->app_id;
    reg->priority = desc->priority;
    reg->name = desc->name;
    reg->init_cb = desc->init_cb;
    reg->destroy_cb = desc->destroy_cb;
    reg->screen_policy = desc->screen_policy;
    reg->on_enter = desc->on_enter;
    reg->on_leave = desc->on_leave;
    reg->screen = NULL;
    reg->initialized = false;
    g_dfm.app_count++;

    printf("[display] registered %s(id=%d, pri=%s, policy=%s)\n",
           desc->name ? desc->name : "?", (int)desc->app_id,
           app_priority_to_str(desc->priority),
           desc->screen_policy == APP_SCREEN_TRANSIENT ? "TRANSIENT" : "PERSISTENT");
    return APP_OK;
}

int displayfocusmanager_set_app_priority(app_id_t app_id, app_priority_t priority)
{
    app_reg_t *reg;

    if (!g_dfm.initialized) {
        return APP_ERR_INIT;
    }
    reg = find_app_by_id(app_id);
    if (!reg) {
        return APP_ERR_PARAM;
    }
    reg->priority = priority;
    return APP_OK;
}

int displayfocusmanager_set_screen_policy(app_id_t app_id, app_screen_policy_t policy)
{
    app_reg_t *reg;

    if (!g_dfm.initialized) {
        return APP_ERR_INIT;
    }
    reg = find_app_by_id(app_id);
    if (!reg) {
        return APP_ERR_PARAM;
    }
    reg->screen_policy = policy;
    return APP_OK;
}

int displayfocusmanager_open_app(app_id_t app_id)
{
    app_reg_t *app;
    app_reg_t *current;
    int pos;
    int ret;

    if (!g_dfm.initialized) {
        return APP_ERR_INIT;
    }
    if (app_id == APP_HOME) {
        return displayfocusmanager_go_home();
    }

    app = find_app_by_id(app_id);
    if (!app) {
        printf("[display] app not registered: id=%d\n", (int)app_id);
        return APP_ERR_PARAM;
    }

    current = get_current_app_reg();
    pos = find_in_stack(app_id);
    if (pos == g_dfm.stack_size - 1) {
        return APP_OK;
    }

    if (current && current->priority != APP_PRIORITY_SYSTEM &&
        app->priority < current->priority) {
        return APP_ERR_BUSY;
    }

    if (pos < 0) {
        if (g_dfm.stack_size >= MAX_STACK_DEPTH) {
            return APP_ERR_FULL;
        }
    }

    ret = ensure_app_initialized(app);
    if (ret != APP_OK) {
        return ret;
    }

    if (current && current != app) {
        call_on_leave(current);
        notify_focus_change(current->app_id, false);
    }

    if (pos < 0) {
        push_stack(app_id);
    } else {
        move_stack_entry_to_top(pos);
    }

    load_app_screen(app);
    call_on_enter(app);
    notify_focus_change(app_id, true);
    return APP_OK;
}

int displayfocusmanager_close_current_app(void)
{
    app_reg_t *closing_app;
    app_reg_t *restored_app;
    app_id_t closing_id;

    if (!g_dfm.initialized) {
        return APP_ERR_INIT;
    }
    if (g_dfm.stack_size <= 1) {
        return APP_ERR_PARAM;
    }

    closing_id = g_dfm.stack[g_dfm.stack_size - 1];
    closing_app = find_app_by_id(closing_id);
    if (!closing_app) {
        return APP_ERR_PARAM;
    }

    call_on_leave(closing_app);
    notify_focus_change(closing_id, false);
    g_dfm.stack_size--;

    restored_app = get_current_app_reg();
    if (restored_app) {
        load_app_screen(restored_app);
        call_on_enter(restored_app);
        notify_focus_change(restored_app->app_id, true);
    }

    if (closing_app->screen_policy == APP_SCREEN_TRANSIENT) {
        destroy_app_resources(closing_app, true);
    }
    return APP_OK;
}

int displayfocusmanager_close_app(app_id_t app_id)
{
    app_reg_t *app;
    app_reg_t *current;
    int pos;
    bool was_top;

    if (!g_dfm.initialized) {
        return APP_ERR_INIT;
    }
    if (app_id == APP_HOME) {
        return APP_ERR_PARAM;
    }

    pos = find_in_stack(app_id);
    if (pos < 0) {
        return APP_ERR_PARAM;
    }

    app = find_app_by_id(app_id);
    if (!app) {
        return APP_ERR_PARAM;
    }

    current = get_current_app_reg();
    was_top = (pos == g_dfm.stack_size - 1);

    if (was_top) {
        call_on_leave(app);
        notify_focus_change(app_id, false);
    }

    remove_stack_pos(pos);

    if (was_top) {
        app_reg_t *new_top = get_current_app_reg();
        if (new_top) {
            load_app_screen(new_top);
            call_on_enter(new_top);
            notify_focus_change(new_top->app_id, true);
        }
    }

    if (app->screen_policy == APP_SCREEN_TRANSIENT) {
        destroy_app_resources(app, true);
    }
    return APP_OK;
}

int displayfocusmanager_go_home(void)
{
    app_reg_t *home_app;
    app_reg_t *current;
    int ret;

    if (!g_dfm.initialized) {
        return APP_ERR_INIT;
    }

    home_app = find_app_by_id(APP_HOME);
    if (!home_app) {
        return APP_ERR_PARAM;
    }

    if (g_dfm.stack_size == 1 && g_dfm.stack[0] == APP_HOME) {
        return APP_OK;
    }

    ret = ensure_app_initialized(home_app);
    if (ret != APP_OK) {
        return ret;
    }

    current = get_current_app_reg();
    if (current && current->app_id != APP_HOME) {
        call_on_leave(current);
        notify_focus_change(current->app_id, false);
    }

    for (int i = g_dfm.stack_size - 1; i >= 0; i--) {
        app_id_t id = g_dfm.stack[i];
        app_reg_t *app;
        if (id == APP_HOME) {
            continue;
        }
        app = find_app_by_id(id);
        if (app && app->screen_policy == APP_SCREEN_TRANSIENT && app != current) {
            destroy_app_resources(app, true);
        }
    }

    if (current && current->app_id != APP_HOME &&
        current->screen_policy == APP_SCREEN_TRANSIENT) {
        destroy_app_resources(current, true);
    }

    g_dfm.stack[0] = APP_HOME;
    g_dfm.stack_size = 1;
    load_app_screen(home_app);
    call_on_enter(home_app);
    notify_focus_change(APP_HOME, true);
    return APP_OK;
}

app_id_t displayfocusmanager_get_current_app(void)
{
    if (!g_dfm.initialized || g_dfm.stack_size == 0) {
        return APP_NONE;
    }
    return g_dfm.stack[g_dfm.stack_size - 1];
}

int displayfocusmanager_get_stack_size(void)
{
    if (!g_dfm.initialized) {
        return -1;
    }
    return g_dfm.stack_size;
}

bool displayfocusmanager_is_in_stack(app_id_t app_id)
{
    if (!g_dfm.initialized) {
        return false;
    }
    return find_in_stack(app_id) >= 0;
}

void displayfocusmanager_register_focus_callback(app_id_t app_id, app_focus_change_cb_t cb, void *user_data)
{
    if (!g_dfm.initialized || !cb) {
        return;
    }

    for (int i = 0; i < g_dfm.focus_cb_count; i++) {
        if (g_dfm.focus_cbs[i].app_id == app_id) {
            g_dfm.focus_cbs[i].cb = cb;
            g_dfm.focus_cbs[i].user_data = user_data;
            return;
        }
    }

    if (g_dfm.focus_cb_count >= MAX_FOCUS_CBS) {
        return;
    }

    g_dfm.focus_cbs[g_dfm.focus_cb_count].app_id = app_id;
    g_dfm.focus_cbs[g_dfm.focus_cb_count].cb = cb;
    g_dfm.focus_cbs[g_dfm.focus_cb_count].user_data = user_data;
    g_dfm.focus_cb_count++;
}

void displayfocusmanager_unregister_focus_callback(app_id_t app_id)
{
    if (!g_dfm.initialized) {
        return;
    }
    for (int i = 0; i < g_dfm.focus_cb_count; i++) {
        if (g_dfm.focus_cbs[i].app_id == app_id) {
            for (int j = i; j < g_dfm.focus_cb_count - 1; j++) {
                g_dfm.focus_cbs[j] = g_dfm.focus_cbs[j + 1];
            }
            g_dfm.focus_cb_count--;
            memset(&g_dfm.focus_cbs[g_dfm.focus_cb_count], 0,
                   sizeof(g_dfm.focus_cbs[g_dfm.focus_cb_count]));
            return;
        }
    }
}

void displayfocusmanager_dump(void)
{
    if (!g_dfm.initialized) {
        printf("[display] not initialized\n");
        return;
    }

    printf("\n========== DISPLAY FOCUS DUMP ==========\n");
    printf("Stack (%d entries):\n", g_dfm.stack_size);
    for (int i = 0; i < g_dfm.stack_size; i++) {
        app_reg_t *app = find_app_by_id(g_dfm.stack[i]);
        if (app) {
            printf("  [%d] %s(id=%d, pri=%s, policy=%s, screen=%p)%s\n",
                   i, app->name, (int)app->app_id,
                   app_priority_to_str(app->priority),
                   app->screen_policy == APP_SCREEN_TRANSIENT ? "TRANSIENT" : "PERSISTENT",
                   (void *)app->screen,
                   (i == g_dfm.stack_size - 1) ? " [TOP]" : "");
        }
    }
    printf("========================================\n\n");
}
