/* music_focus.c against the REAL audiofocusmanager.c, with the real adapter + shim and an async fake engine. */
#include "lvgl/lvgl.h"
#include "music_adapter.h"
#include "music_focus.h"
#include "audiofocusmanager.h"
#include "app_common.h"
#include <stdio.h>
#include <stdlib.h>
void fake_init(void); void fake_shutdown(void);
static int g_checks;
#define CHECK(c) do { g_checks++; if (!(c)) { printf("CHECK FAILED line %d: %s\n", __LINE__, #c); exit(1); } } while (0)
static void settle(int ms) { for (int t = 0; t < ms; t += 20) { lv_tick_inc(20); lv_timer_handler(); music_adapter_poll(); } }
static PlayerState pstate(void) { return music_app_get_state()->player_state; }
static bool music_owns(void) { focus_owner_info_t o; return audiofocusmanager_get_owner_info(&o) && o.id == APP_MUSIC && o.focus_type == FOCUS_MUSIC; }
static int owner_id(void) { focus_owner_info_t o; return audiofocusmanager_get_owner_info(&o) ? o.id : -1; }
static void reset(void) { music_focus_deinit(); audiofocusmanager_deinit(); music_adapter_deinit(); music_adapter_clear_event_hooks();
    fake_init(); audiofocusmanager_init(); music_focus_init(); music_adapter_add_event_hook(music_focus_on_app_event); music_adapter_init(); music_app_stop(); settle(1200); }
static void call_start(void)  { audio_focus_request_t r = {APP_BLUETOOTH, FOCUS_CALL, FOCUS_GRANTED_TRANSIENT}; CHECK(audiofocusmanager_request_focus(&r) == AUDIO_OK); }
static void call_end(void)    { audiofocusmanager_release_focus(APP_BLUETOOTH); }
static void bt_music(void)    { audio_focus_request_t r = {APP_BLUETOOTH, FOCUS_MUSIC, FOCUS_GRANTED}; CHECK(audiofocusmanager_request_focus(&r) == AUDIO_OK); }

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    lv_init();
    printf("-- 1. play -> we own audio focus\n");
    reset(); CHECK(pstate() == PLAYER_STATE_STOPPED); music_app_resume(); settle(60);
    CHECK(pstate() == PLAYER_STATE_PLAYING); CHECK(music_owns());

    printf("-- 2. phone call interrupts, then ends -> auto resume\n");
    call_start(); settle(60); CHECK(pstate() == PLAYER_STATE_PAUSED); CHECK(owner_id() == APP_BLUETOOTH);
    call_end(); settle(60); CHECK(pstate() == PLAYER_STATE_PLAYING); CHECK(music_owns());

    printf("-- 3. user paused before the call -> call ends -> stays paused\n");
    music_app_pause(); settle(60); CHECK(pstate() == PLAYER_STATE_PAUSED);
    call_start(); settle(60); call_end(); settle(60);
    CHECK(pstate() == PLAYER_STATE_PAUSED);

    printf("-- 4. Bluetooth music takes over permanently -> local pauses and never resumes by itself\n");
    music_app_resume(); settle(60); CHECK(pstate() == PLAYER_STATE_PLAYING);
    bt_music(); settle(60); CHECK(pstate() == PLAYER_STATE_PAUSED); CHECK(!music_owns());
    audiofocusmanager_release_focus(APP_BLUETOOTH); settle(60); CHECK(pstate() == PLAYER_STATE_PAUSED);
    music_app_resume(); settle(60); CHECK(pstate() == PLAYER_STATE_PLAYING); CHECK(music_owns());   /* user starts it again: we win it back */

    printf("-- 5. stop: focus is kept for 1 s (track change must not flap), then released\n");
    music_app_stop(); settle(500); CHECK(music_owns());
    music_app_resume(); settle(1200); CHECK(music_owns());                 /* restarted within the window: never released */
    music_app_stop(); settle(1100); CHECK(!music_owns());                  /* really stopped: released */

    printf("-- 6. a call is active when the user presses play -> do not talk over it\n");
    reset(); call_start(); settle(40);
    music_app_resume(); settle(80); CHECK(pstate() == PLAYER_STATE_PAUSED); CHECK(owner_id() == APP_BLUETOOTH);
    call_end(); settle(60);
    printf("   state after the call ended: %d (PAUSED=%d, PLAYING=%d)\n", (int)pstate(), PLAYER_STATE_PAUSED, PLAYER_STATE_PLAYING);

    printf("-- 7. repeated interruptions do not leak focus entries or flip-flop\n");
    reset(); music_app_resume(); settle(60);
    for (int i = 0; i < 50; i++) { call_start(); settle(20); CHECK(pstate() == PLAYER_STATE_PAUSED); call_end(); settle(20); CHECK(pstate() == PLAYER_STATE_PLAYING); CHECK(music_owns()); }

    music_focus_deinit(); audiofocusmanager_deinit(); music_adapter_deinit(); fake_shutdown();
    printf("FOCUS TESTS PASS (%d checks)\n", g_checks);
    return 0;
}
