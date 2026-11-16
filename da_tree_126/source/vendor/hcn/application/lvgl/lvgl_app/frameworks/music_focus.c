/**
 * @file music_focus.c
 *
 * Rules (same vocabulary as bt_focus_adapter.c):
 *   - player enters PLAYING            -> request FOCUS_MUSIC (id = APP_MUSIC); not granted -> pause
 *   - player STOPPED / IDLE            -> release focus (deferred 1 s so a track change does not flap)
 *   - user pause                       -> keep focus (nothing to give up, resume must stay possible)
 *   - FOCUS_LOST_TRANSIENT[_MAY_DUCK]  -> pause, remember "paused by focus" (no ducking API: same as BT)
 *   - FOCUS_GRANTED after that         -> resume, but only if WE paused it (a user pause is respected)
 *   - FOCUS_LOST (permanent)           -> pause, forget; never auto-resume
 *
 * Policy events (send_policy_event(MM_HANDLE, START/STOP)): BT music sends them because its audio does
 * not go through libatcmediaplayer.  Local playback does, and the media player owns its stream, so they
 * are OFF here.  Build with -DMUSIC_FOCUS_SEND_POLICY=1 only if on-device tests show the route is not
 * switched without them.
 */
#include "music_focus.h"
#include "audiofocusmanager.h"
#include "app_common.h"
#include "lvgl/lvgl.h"

#include <stdio.h>

#if defined(MUSIC_FOCUS_SEND_POLICY) && MUSIC_FOCUS_SEND_POLICY
#include "policyEvent.h"
#endif

#define MUSIC_FOCUS_ID        ((int)APP_MUSIC)
#define RELEASE_DEFER_MS      1000

#ifndef MUSIC_FOCUS_LOG
#define MUSIC_FOCUS_LOG(fmt, ...) printf("[MUSIC_FOCUS] " fmt "\r\n", ##__VA_ARGS__)
#endif

static bool s_inited;
static bool s_held;
static bool s_paused_by_focus;
static bool s_policy_started;
static PlayerState s_state = PLAYER_STATE_IDLE;
static lv_timer_t *s_release_timer;

static void set_policy(bool started)
{
#if defined(MUSIC_FOCUS_SEND_POLICY) && MUSIC_FOCUS_SEND_POLICY
    if (s_policy_started == started) return;
    s_policy_started = started;
    send_policy_event(MM_HANDLE, started ? START : STOP);
#else
    (void)started; (void)s_policy_started;
#endif
}

static bool owner_is_us(void)
{
    focus_owner_info_t o;
    return audiofocusmanager_get_owner_info(&o) && o.id == MUSIC_FOCUS_ID &&
           o.focus_type == FOCUS_MUSIC && o.state == FOCUS_GRANTED;
}

static void cancel_deferred_release(void)
{
    if (s_release_timer) { lv_timer_delete(s_release_timer); s_release_timer = NULL; }
}

static void release_focus_now(void)
{
    cancel_deferred_release();
    if (!s_held) return;
    audiofocusmanager_release_focus(MUSIC_FOCUS_ID);
    s_held = false;
    s_paused_by_focus = false;
    set_policy(false);
    MUSIC_FOCUS_LOG("released");
}

static void on_release_timer(lv_timer_t *t)
{
    (void)t;
    s_release_timer = NULL;                 /* one-shot: LVGL deletes it after the single run */
    if (s_state == PLAYER_STATE_STOPPED || s_state == PLAYER_STATE_IDLE) release_focus_now();
}

static void schedule_release(void)
{
    cancel_deferred_release();
    s_release_timer = lv_timer_create(on_release_timer, RELEASE_DEFER_MS, NULL);
    lv_timer_set_repeat_count(s_release_timer, 1);
}

static bool request_focus(void)
{
    audio_focus_request_t req;
    int ret;
    cancel_deferred_release();
    if (s_held && owner_is_us()) return true;
    req.id = MUSIC_FOCUS_ID;
    req.focus_type = FOCUS_MUSIC;
    req.state = FOCUS_GRANTED;
    ret = audiofocusmanager_request_focus(&req);
    if (ret == AUDIO_OK && owner_is_us()) { s_held = true; set_policy(true); MUSIC_FOCUS_LOG("granted"); return true; }
    s_held = (ret == AUDIO_OK);             /* on the stack but not on top: we may get GRANTED later */
    MUSIC_FOCUS_LOG("not granted (ret=%d)", ret);
    return false;
}

static void on_focus_changed(const audio_focus_change_info_t *info, void *user_data)
{
    (void)user_data;
    if (!s_inited || info == NULL) return;
    switch (info->state) {
    case FOCUS_GRANTED:
        s_held = true;
        set_policy(true);
        if (s_paused_by_focus) {
            s_paused_by_focus = false;
            music_app_resume();             /* idempotent if the player is already running */
            MUSIC_FOCUS_LOG("resumed after focus grant");
        }
        break;
    case FOCUS_LOST_TRANSIENT:
    case FOCUS_LOST_TRANSIENT_MAY_DUCK:
        set_policy(false);
        if (s_state == PLAYER_STATE_PLAYING || music_app_get_state()->player_state == PLAYER_STATE_PLAYING) {
            music_app_pause();
            s_paused_by_focus = true;
            MUSIC_FOCUS_LOG("paused by transient focus loss");
        }
        break;
    case FOCUS_LOST:
        set_policy(false);
        if (s_state == PLAYER_STATE_PLAYING || music_app_get_state()->player_state == PLAYER_STATE_PLAYING) music_app_pause();
        s_held = false;
        s_paused_by_focus = false;
        MUSIC_FOCUS_LOG("lost permanently");
        break;
    default:
        break;
    }
}

int music_focus_init(void)
{
    if (s_inited) return 0;
    s_held = s_paused_by_focus = s_policy_started = false;
    s_state = PLAYER_STATE_IDLE;
    audiofocusmanager_register_focus_callback(MUSIC_FOCUS_ID, on_focus_changed, NULL);
    s_inited = true;
    return 0;
}

void music_focus_deinit(void)
{
    if (!s_inited) return;
    release_focus_now();
    audiofocusmanager_unregister_focus_callback(MUSIC_FOCUS_ID);
    s_inited = false;
}

void music_focus_on_app_event(music_app_event_t event, void *param)
{
    PlayerState st;
    if (!s_inited || event != APP_EVENT_STATE_CHANGED) return;
    st = (PlayerState)(param ? *(int *)param : 0);
    s_state = st;
    if (st == PLAYER_STATE_PLAYING) {
        if (!request_focus()) {
            music_app_pause();              /* somebody with priority owns the audio: do not talk over them */
            s_paused_by_focus = s_held;     /* queued on the stack: resume if focus comes back */
        }
    } else if (st == PLAYER_STATE_STOPPED || st == PLAYER_STATE_IDLE) {
        schedule_release();
    }
}
