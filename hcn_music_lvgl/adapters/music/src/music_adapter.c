/**
 * @file music_adapter.c
 */
#include "music_adapter.h"
#include "compat/awtk.h" /* music_mainloop_* */

#ifndef HCN_MUSIC_SHIM
#error "music_app.h pulled the upstream awtk_stub.h / real awtk.h: build WITHOUT -DAWTK_STUB and with include/compat on the include path"
#endif

#include <stdio.h>

#define MAX_HOOKS 4

static MusicAdapterEventHook s_hooks[MAX_HOOKS];
static int  s_nhooks;
static bool s_ready;

static void dispatch(music_app_event_t event, void *param)
{
    for (int i = 0; i < s_nhooks; i++) s_hooks[i](event, param);
}

void music_adapter_add_event_hook(MusicAdapterEventHook hook)
{
    if (hook == NULL || s_nhooks >= MAX_HOOKS) return;
    for (int i = 0; i < s_nhooks; i++) if (s_hooks[i] == hook) return;
    s_hooks[s_nhooks++] = hook;
}

void music_adapter_clear_event_hooks(void) { s_nhooks = 0; }

int music_adapter_init(void)
{
    int ret;
    if (s_ready) return 0;
    ret = music_app_init(dispatch);
    if (ret != 0) {
        fprintf(stderr, "[music_adapter] music_app_init failed (%d): music UI stays empty\n", ret);
        return ret < 0 ? ret : -ret;
    }
    s_ready = true;
    return 0;
}

void music_adapter_deinit(void)
{
    if (!s_ready) return;
    music_app_save_state();
    music_app_deinit();
    s_ready = false;
    music_mainloop_deinit();
}

bool music_adapter_is_ready(void) { return s_ready; }

void music_adapter_poll(void)
{
    if (s_ready) music_mainloop_poll();
}
