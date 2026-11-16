/**
 * @file ui_variant.c
 * @brief Selects exactly one UI variant and owns the shared LVGL setup.
 */

#include "ui_variant.h"
#include "displayfocusmanager.h"
#include "audiofocusmanager.h"
#include "ui_shared_apps.h"
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)
#include <unistd.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifndef HCN_APP_UI_VARIANT_MODE
#define HCN_APP_UI_VARIANT_MODE 0
#endif

#define HCN_UI_VARIANT_TEMPLATE 1
#define HCN_UI_VARIANT_FALLBACK 2

#define HCN_UI_ASSET_PATH_MAX 512
#define HCN_UI_TEMPLATE_PROBE "image/start_animation/000.png"

#ifdef HCN_APP_HAVE_TEMPLATE_UI
#include "ui_template/ui_template.h"
#endif

static hcn_ui_variant_t s_active_variant = HCN_UI_VARIANT_NONE;

static int path_is_file(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        return 0;
    }
    fclose(fp);
    return 1;
}

static int template_assets_ready(void)
{
    const char *candidates[4];
    char exe_dir[HCN_UI_ASSET_PATH_MAX];
    char exe_assets[HCN_UI_ASSET_PATH_MAX];
    const char *env;

    env = getenv("HCN_APPLICATION_ASSETS");
    candidates[0] = env;
    candidates[1] = NULL;
    candidates[2] = "./assets";
#ifdef HCN_APP_UI_TEMPLATE_ASSETS
    candidates[3] = HCN_APP_UI_TEMPLATE_ASSETS;
#else
    candidates[3] = NULL;
#endif

#if defined(__linux__)
    {
        ssize_t n = readlink("/proc/self/exe", exe_dir, sizeof(exe_dir) - 1);
        if (n > 0) {
            char *slash;
            exe_dir[n] = '\0';
            slash = strrchr(exe_dir, '/');
            if (slash != NULL) {
                *slash = '\0';
                snprintf(exe_assets, sizeof(exe_assets), "%s/assets", exe_dir);
                candidates[1] = exe_assets;
            }
        }
    }
#elif defined(_WIN32)
    {
        DWORD n = GetModuleFileNameA(NULL, exe_dir, sizeof(exe_dir) - 1);
        if (n > 0) {
            char *slash;
            exe_dir[n] = '\0';
            slash = strrchr(exe_dir, '\\');
            if (slash != NULL) {
                *slash = '\0';
                snprintf(exe_assets, sizeof(exe_assets), "%s\\assets", exe_dir);
                candidates[1] = exe_assets;
            }
        }
    }
#endif

    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
        const char *dir = candidates[i];
        char probe[HCN_UI_ASSET_PATH_MAX + 64];
        size_t len;

        if (dir == NULL || dir[0] == '\0') {
            continue;
        }
        if (dir[0] == 'A' && dir[1] == ':') {
            dir += 2;
        }
        len = strlen(dir);
        if (len >= 2 && (dir[len - 1] == '/' || dir[len - 1] == '\\')) {
            snprintf(probe, sizeof(probe), "%s%s", dir, HCN_UI_TEMPLATE_PROBE);
        } else {
            snprintf(probe, sizeof(probe), "%s/%s", dir, HCN_UI_TEMPLATE_PROBE);
        }
        if (path_is_file(probe)) {
            return 1;
        }
    }
    return 0;
}

static void prepare_display(void)
{
    lv_disp_t *disp;
    lv_theme_t *theme;

    LV_EVENT_GET_COMP_CHILD = lv_event_register_id();

    disp = lv_display_get_default();
    theme = lv_theme_default_init(disp,
                                  lv_palette_main(LV_PALETTE_BLUE),
                                  lv_palette_main(LV_PALETTE_RED),
                                  false,
                                  LV_FONT_DEFAULT);
    lv_disp_set_theme(disp, theme);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_ARGB8888);
}

static void show_error_page(void)
{
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_t *label;

    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    label = lv_label_create(screen);
    lv_label_set_text(label, "UI initialization failed");
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);
    lv_screen_load(screen);
}

static int start_template_ui(void)
{
#ifdef HCN_APP_HAVE_TEMPLATE_UI
    int ret = hcn_ui_template_init();
    if (ret != 0) {
        printf("[ui] template init failed, fallback once\n");
        displayfocusmanager_deinit();
        displayfocusmanager_init();
        return -1;
    }
    s_active_variant = HCN_UI_VARIANT_TEMPLATE;
    printf("[ui] template UI selected\n");
    return 0;
#else
    printf("[ui] template UI not compiled\n");
    return -1;
#endif
}

static int start_fallback_ui(void)
{
    int ret;

    ret = hcn_ui_shared_apps_register();
    if (ret != APP_OK) {
        printf("[ui] fallback shared app registration failed: %d\n", ret);
        return -1;
    }
    ret = hcn_ui_fallback_init();
    if (ret != 0) {
        printf("[ui] fallback init failed: %d\n", ret);
        displayfocusmanager_deinit();
        displayfocusmanager_init();
        return -1;
    }
    s_active_variant = HCN_UI_VARIANT_FALLBACK;
    printf("[ui] fallback UI selected\n");
    return 0;
}

int hcn_ui_boot(void)
{
    prepare_display();
    displayfocusmanager_init();
    audiofocusmanager_init();

    if (HCN_APP_UI_VARIANT_MODE == HCN_UI_VARIANT_TEMPLATE) {
#ifdef HCN_APP_HAVE_TEMPLATE_UI
        if (template_assets_ready() && start_template_ui() == 0) {
            hcn_ui_shared_apps_init();
            return 0;
        }
        printf("[ui] template unavailable, fallback once\n");
#endif
        if (start_fallback_ui() == 0) {
            hcn_ui_shared_apps_init();
            return 0;
        }
    } else if (HCN_APP_UI_VARIANT_MODE == HCN_UI_VARIANT_FALLBACK) {
        if (start_fallback_ui() == 0) {
            hcn_ui_shared_apps_init();
            return 0;
        }
    } else {
#ifdef HCN_APP_HAVE_TEMPLATE_UI
        if (!template_assets_ready()) {
            printf("[ui] template assets missing: %s\n",
                   getenv("HCN_APPLICATION_ASSETS") != NULL ?
                   getenv("HCN_APPLICATION_ASSETS") : "built-in asset paths");
        } else if (start_template_ui() == 0) {
            hcn_ui_shared_apps_init();
            return 0;
        }
#endif
        if (start_fallback_ui() == 0) {
            hcn_ui_shared_apps_init();
            return 0;
        }
    }

    if (s_active_variant == HCN_UI_VARIANT_NONE) {
        show_error_page();
    }
    return -1;
}

void hcn_ui_shutdown(void)
{
    hcn_ui_shared_apps_deinit();

    audiofocusmanager_deinit();
    displayfocusmanager_deinit();

    if (s_active_variant == HCN_UI_VARIANT_TEMPLATE) {
#ifdef HCN_APP_HAVE_TEMPLATE_UI
        hcn_ui_template_deinit();
#endif
    } else if (s_active_variant == HCN_UI_VARIANT_FALLBACK) {
        hcn_ui_fallback_deinit();
    }

    s_active_variant = HCN_UI_VARIANT_NONE;
}

hcn_ui_variant_t hcn_ui_active_variant(void)
{
    return s_active_variant;
}
