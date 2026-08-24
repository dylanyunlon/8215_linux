/**
 * @file main.c
 * @brief AWTK Music Player application entry point.
 *
 * Initialization sequence:
 *   1. tk_init()       — AWTK framework
 *   2. music_app_init() — backend (player, USB monitor, scanner)
 *   3. music_ui_create() — create UI widgets
 *   4. tk_run()         — enter main loop
 *
 * Reference: source/packages/cluster/awtk/awtk_start.sh
 *            source/packages/cluster/cluster_app/src/main.cpp
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#include "awtk.h"
#include "music_app.h"
#include "music_ui.h"

#include <stdio.h>
#include <signal.h>

#define APP_NAME   "MusicPlayer"
#define APP_WIDTH  1024
#define APP_HEIGHT 600

/*============================================================================
 * Signal handler for graceful shutdown
 *==========================================================================*/
static volatile bool s_running = true;

static void signal_handler(int sig) {
    (void)sig;
    printf("[main] Signal %d received, shutting down...\n", sig);
    s_running = false;
    tk_quit();
}

/*============================================================================
 * AWTK app lifecycle callbacks
 *==========================================================================*/
static ret_t application_init(void) {
    printf("[main] application_init\n");

    /* Create main window */
    widget_t* win = window_create(NULL, 0, 0, APP_WIDTH, APP_HEIGHT);
    widget_set_style_str(win, "bg_color", "#1a1a2e");

    /* Initialize backend (player + USB monitor + scanner) */
    int ret = music_app_init(music_ui_on_app_event);
    if (ret != 0) {
        printf("[main] WARNING: music_app_init failed (ret=%d), UI only mode\n", ret);
    }

    /* Create UI widgets on the window */
    music_ui_create(win);

    return RET_OK;
}

static ret_t application_exit(void) {
    printf("[main] application_exit\n");

    music_ui_destroy();
    music_app_deinit();

    return RET_OK;
}

/*============================================================================
 * Main entry point
 *==========================================================================*/

#ifdef AWTK_LINUX_FB
/* Linux framebuffer build — uses awtk-linux-fb main_loop */

#include "main_loop_linux.h"
#include "base/assets_manager.h"
#include "base/locale_info.h"

/**
 * Load AWTK assets from file system.
 *
 * AWTK looks for: {app_root}/assets/default/raw/{styles,fonts,strings,...}
 * The resource files (.bin, .ttf) must be pre-compiled by AWTK's resource
 * packer (packaged in the SDK build).
 *
 * For the standalone Makefile build, we copy them from
 * out/build/ac83xx/awtk-1.0/awtk/res/assets/ to /tmp/res/assets/ on the board.
 */
ret_t assets_init(void) {
    /* Tell assets_manager to load from file system instead of compiled-in data */
    assets_manager_set_res_root(assets_manager(), "");

    /* Load the default theme style */
    assets_manager_load(assets_manager(), ASSET_TYPE_STYLE, "default");

    /* Load default font */
    assets_manager_load(assets_manager(), ASSET_TYPE_FONT, "default");

    /* Load Chinese strings if available */
    locale_info_change(locale_info(), "zh", "CN");

    return RET_OK;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    /* Issue #39: Prevent zombie child processes (e.g. from mount helpers).
     * In single-process architecture (ref: 0314_ad008), any fork()ed
     * child must be reaped. SIG_IGN auto-reaps on Linux. */
    signal(SIGCHLD, SIG_IGN);

    printf("=== %s (AWTK Linux-FB) ===\n", APP_NAME);

    /* AWTK init — lcd, input, fonts
     * APP_MOBILE is defined via AWTK CCFLAGS (-DAPP_TYPE=APP_MOBILE) */
    tk_init(APP_WIDTH, APP_HEIGHT, APP_MOBILE, APP_NAME, NULL);

    /* Load resources (stub — we create widgets in code) */
    assets_init();

    /* App init */
    application_init();

    /* Run main loop (blocks until tk_quit) */
    tk_run();

    /* Cleanup */
    application_exit();

    printf("=== %s exited ===\n", APP_NAME);
    return 0;
}

#else
/* Desktop simulation build (for development/testing) */

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("=== %s (AWTK Desktop Sim) ===\n", APP_NAME);

    tk_init(APP_WIDTH, APP_HEIGHT, APP_MOBILE, APP_NAME, NULL);

    application_init();

    tk_run();

    application_exit();

    printf("=== %s exited ===\n", APP_NAME);
    return 0;
}

#endif /* AWTK_LINUX_FB */
