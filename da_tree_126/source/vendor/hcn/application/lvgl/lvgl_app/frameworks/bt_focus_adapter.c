/**
 * @file bt_focus_adapter.c
 * @brief Bluetooth adapter for audio and video focus managers
 */

#include "bt_focus_adapter.h"

#include "audiofocusmanager.h"
#include "bt_app.h"
#include "policyEvent.h"
#include "videofocusmanager.h"

#ifdef LVGL_BUILD_CARPLAY
#include "carplayapp_if.h"
#endif

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef BT_FOCUS_LOG
#define BT_FOCUS_LOG(fmt, ...) \
    do { \
        printf("[BT_FOCUS][%s:%d] " fmt "\r\n", __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#endif

#ifndef BT_FOCUS_LOGI
#define BT_FOCUS_LOGI(fmt, ...) BT_FOCUS_LOG("[I] " fmt, ##__VA_ARGS__)
#endif

#ifndef BT_FOCUS_LOGW
#define BT_FOCUS_LOGW(fmt, ...) BT_FOCUS_LOG("[W] " fmt, ##__VA_ARGS__)
#endif

#ifndef BT_FOCUS_LOGE
#define BT_FOCUS_LOGE(fmt, ...) BT_FOCUS_LOG("[E] " fmt, ##__VA_ARGS__)
#endif

static bool s_initialized;
static bool s_has_prev_snapshot;
static BtAppSnapshot s_prev_snapshot;
static bool s_music_focus_held;
static bool s_call_focus_held;
static bool s_music_paused_by_focus;
static bool s_music_resume_suppressed_by_call;
static bool s_call_triggered_bluetooth_open;
static bool s_music_stop_deferred;
static bool s_bluetooth_music_page_selected;
static bool s_music_policy_started;
static bool s_call_policy_started;
static bool s_local_ringtone_policy_started;
static bool s_local_ringtone_playing;
static bool s_carplay_call_conflict;
static bool s_call_video_open_pending;
static app_id_t s_call_video_blocker = APP_NONE;
static app_id_t s_call_prev_app = APP_NONE;

static void cancel_deferred_music_stop(void);

static void set_music_audio_policy(bool started)
{
    if (s_music_policy_started == started) {
        return;
    }
    s_music_policy_started = started;
    send_policy_event(MM_HANDLE, started ? START : STOP);
    BT_FOCUS_LOGI("Bluetooth music audio policy %s", started ? "START" : "STOP");
}

static void set_call_audio_policy(bool started)
{
    if (s_call_policy_started == started) {
        return;
    }
    s_call_policy_started = started;
    send_policy_event(BTHFP_HANDLE, started ? START : STOP);
    BT_FOCUS_LOGI("Bluetooth call audio policy %s", started ? "START" : "STOP");
}

static void set_local_ringtone_audio_policy(bool started)
{
    if (s_local_ringtone_policy_started == started) {
        return;
    }
    s_local_ringtone_policy_started = started;
    send_policy_event(RING_BT_HANDLE, started ? START : STOP);
    BT_FOCUS_LOGI("Bluetooth local ringtone audio policy %s",
                  started ? "START" : "STOP");
}

static void stop_call_audio_policies(void)
{
    set_local_ringtone_audio_policy(false);
    set_call_audio_policy(false);
}

static bool is_audio_focus_owner(app_id_t app_id, audio_focus_type_t focus_type)
{
    focus_owner_info_t owner;

    return audiofocusmanager_get_owner_info(&owner) &&
           owner.id == app_id &&
           owner.focus_type == focus_type;
}

static bool is_bluetooth_call_focus_owner(void)
{
    return is_audio_focus_owner(BT_AUDIO_FOCUS_ID, FOCUS_CALL);
}

static bool is_carplay_call_focus_owner(void)
{
#ifdef LVGL_BUILD_CARPLAY
    return is_audio_focus_owner(APP_CARPLAY, FOCUS_CALL);
#else
    return false;
#endif
}

static void stop_local_ringtone_for_focus_loss(void)
{
    if (s_local_ringtone_playing) {
        bt_app_stop_call_ringtone();
        s_local_ringtone_playing = false;
        BT_FOCUS_LOGI("Bluetooth local ringtone paused by audio focus loss");
    }
}

static void sync_call_audio_policies(const BtAppSnapshot *snapshot)
{
    bool local_ringtone_started;
    bool call_policy_started;
    bool bluetooth_call_focus_owned;

    bluetooth_call_focus_owned =
        s_call_focus_held &&
        is_bluetooth_call_focus_owner() &&
        !s_carplay_call_conflict;

    local_ringtone_started =
        bluetooth_call_focus_owned &&
        snapshot != NULL &&
        snapshot->call_state == BT_APP_CALL_STATE_INCOMING &&
        snapshot->ringtone_type == BT_RINGTONE_TYPE_LOCAL;
    call_policy_started =
        bluetooth_call_focus_owned &&
        snapshot != NULL &&
        snapshot->call_state != BT_APP_CALL_STATE_IDLE &&
        !local_ringtone_started;

    /* Stop the previous policy before starting the policy for the new state. */
    if (!local_ringtone_started) {
        set_local_ringtone_audio_policy(false);
    }
    if (!call_policy_started) {
        set_call_audio_policy(false);
    }
    if (local_ringtone_started) {
        set_local_ringtone_audio_policy(true);
    }
    if (call_policy_started) {
        set_call_audio_policy(true);
    }
}

static bool has_connected_carplay_call(void)
{
#ifdef LVGL_BUILD_CARPLAY
    CarplayApp *carplay_app = carplayapp_get_instance();

    return carplay_app != NULL &&
           carplayapp_has_connected_call(carplay_app);
#else
    return false;
#endif
}

static void sync_call_ringtone_and_route(const BtAppSnapshot *snapshot)
{
    bool has_local_ringtone_request =
        snapshot != NULL &&
        snapshot->in_call &&
        snapshot->call_state == BT_APP_CALL_STATE_INCOMING &&
        snapshot->ringtone_requested &&
        snapshot->ringtone_type_known &&
        snapshot->ringtone_type == BT_RINGTONE_TYPE_LOCAL;
    bool carplay_call_conflict =
        s_carplay_call_conflict || is_carplay_call_focus_owner();
    bool should_play_local_ringtone =
        has_local_ringtone_request &&
        s_call_focus_held &&
        is_bluetooth_call_focus_owner() &&
        !carplay_call_conflict;
    bool should_force_phone_audio =
        snapshot != NULL &&
        snapshot->in_call &&
        carplay_call_conflict &&
        (snapshot->call_state != BT_APP_CALL_STATE_INCOMING ||
         snapshot->ringtone_type == BT_RINGTONE_TYPE_INBAND);

    if (should_play_local_ringtone && !s_local_ringtone_playing) {
        bt_app_play_call_ringtone();
        s_local_ringtone_playing = true;
        BT_FOCUS_LOGI("Bluetooth local ringtone started");
    } else if (!should_play_local_ringtone && s_local_ringtone_playing) {
        bt_app_stop_call_ringtone();
        s_local_ringtone_playing = false;
        BT_FOCUS_LOGI("Bluetooth local ringtone stopped");
    }

    if (has_local_ringtone_request &&
        carplay_call_conflict &&
        (!s_prev_snapshot.ringtone_requested ||
         !s_prev_snapshot.ringtone_type_known ||
         s_prev_snapshot.ringtone_type != BT_RINGTONE_TYPE_LOCAL)) {
        BT_FOCUS_LOGI("Bluetooth local ringtone suppressed for active CarPlay call");
    }

    if (should_force_phone_audio &&
        !snapshot->call_audio_in_phone) {
        bt_app_switch_call_audio(true);
        BT_FOCUS_LOGI("Bluetooth SCO disconnect requested for active CarPlay call");
    }
}

static void bt_focus_set_music_audio(bool enabled)
{
    bt_app_set_music_audio_enabled(enabled);
    set_music_audio_policy(enabled);
}

static bool is_bluetooth_music_page_active(void)
{
    return s_bluetooth_music_page_selected &&
           videofocusmanager_get_current_app() == APP_BLUETOOTH;
}

static bool is_bluetooth_music_focus_owner(void)
{
    focus_owner_info_t owner;

    return audiofocusmanager_get_owner_info(&owner) &&
           owner.id == BT_AUDIO_FOCUS_ID &&
           owner.focus_type == FOCUS_MUSIC &&
           owner.state == FOCUS_GRANTED;
}

static void block_music_without_focus(void)
{
    BtAppSnapshot snapshot;
    focus_owner_info_t owner;

    cancel_deferred_music_stop();
    bt_focus_set_music_audio(false);
    bt_app_get_snapshot(&snapshot);
    if (snapshot.music_playing_state) {
        bt_app_music_pause();
    }
    s_music_paused_by_focus = true;
    s_music_resume_suppressed_by_call = false;

    if (audiofocusmanager_get_owner_info(&owner)) {
        BT_FOCUS_LOGW("Bluetooth music blocked: owner_id=%d owner_type=%d owner_state=%d",
                      owner.id, (int)owner.focus_type, (int)owner.state);
    } else {
        BT_FOCUS_LOGW("Bluetooth music blocked: no audio focus owner");
    }
}

static void pause_music_by_focus_loss(void)
{
    BtAppSnapshot snapshot;

    cancel_deferred_music_stop();
    bt_focus_set_music_audio(false);
    bt_app_get_snapshot(&snapshot);
    if (snapshot.music_playing_state) {
        /*
         * No Bluetooth ducking API is available now, so MAY_DUCK uses
         * the same pause behavior as a normal transient loss.
         */
        bt_app_music_pause();
        BT_FOCUS_LOGI("Bluetooth music paused by audio focus loss");
    }
    s_music_paused_by_focus = true;
    s_music_resume_suppressed_by_call = true;
}

static bool request_music_focus(bool force)
{
    audio_focus_request_t request;
    int ret;

    if (s_music_focus_held && !force) {
        return is_bluetooth_music_focus_owner();
    }

    request.id = BT_AUDIO_FOCUS_ID;
    request.focus_type = FOCUS_MUSIC;
    request.state = FOCUS_GRANTED;

    ret = audiofocusmanager_request_focus(&request);
    if (ret == AUDIO_OK) {
        s_music_focus_held = true;
        BT_FOCUS_LOGI("Bluetooth music audio focus requested");
        if (is_bluetooth_music_focus_owner()) {
            return true;
        }
        BT_FOCUS_LOGW("Bluetooth music focus request returned without ownership");
    } else {
        BT_FOCUS_LOGW("Bluetooth music audio focus rejected: ret=%d", ret);
    }

    block_music_without_focus();
    return false;
}

static void release_music_focus(void)
{
    if (!s_music_focus_held) {
        return;
    }

    s_music_stop_deferred = false;
    bt_focus_set_music_audio(false);
    if (!s_call_focus_held) {
        audiofocusmanager_release_focus(BT_AUDIO_FOCUS_ID);
    }
    s_music_focus_held = false;
    s_music_paused_by_focus = false;
    s_music_resume_suppressed_by_call = false;
    BT_FOCUS_LOGI("Bluetooth music audio focus released");
}

static void request_call_focus(void)
{
    audio_focus_request_t request;
    bool had_music_focus;
    int ret;

    if (s_call_focus_held) {
        return;
    }

    had_music_focus = s_music_focus_held;
    request.id = BT_AUDIO_FOCUS_ID;
    request.focus_type = FOCUS_CALL;
    request.state = FOCUS_GRANTED_TRANSIENT;

    ret = audiofocusmanager_request_focus(&request);
    if (ret == AUDIO_OK) {
        if (had_music_focus) {
            pause_music_by_focus_loss();
        }
        s_call_focus_held = true;
        BT_FOCUS_LOGI("Bluetooth call audio focus requested");
    } else {
        BT_FOCUS_LOGW("Bluetooth call audio focus rejected: ret=%d", ret);
    }
}

static void release_call_focus(void)
{
    if (!s_call_focus_held) {
        stop_call_audio_policies();
        return;
    }

    s_call_focus_held = false;
    stop_call_audio_policies();
    if (s_music_focus_held) {
        (void)request_music_focus(true);
    } else {
        audiofocusmanager_release_focus(BT_AUDIO_FOCUS_ID);
    }
    BT_FOCUS_LOGI("Bluetooth call audio focus released");
}

static void restore_bluetooth_video_priority(void)
{
    (void)videofocusmanager_set_app_priority(APP_BLUETOOTH, APP_PRIORITY_LOW);
}

static void clear_pending_call_video_open(void)
{
    s_call_video_open_pending = false;
    s_call_video_blocker = APP_NONE;
}

static void try_open_bluetooth_for_call(void)
{
    app_id_t current;
    int ret;

    current = videofocusmanager_get_current_app();

    if (current == APP_BLUETOOTH) {
        clear_pending_call_video_open();
        return;
    }

    ret = videofocusmanager_open_app(APP_BLUETOOTH);
    if (ret == APP_OK) {
        s_call_prev_app = current;
        s_call_triggered_bluetooth_open = true;
        clear_pending_call_video_open();
        BT_FOCUS_LOGI("Bluetooth call UI opened, previous app=%d",
                      (int)s_call_prev_app);
        return;
    }

    if (ret == APP_ERR_BUSY) {
        s_call_video_open_pending = true;
        s_call_video_blocker = current;
        BT_FOCUS_LOGI("Bluetooth call UI deferred by app=%d",
                      (int)current);
        return;
    }

    clear_pending_call_video_open();
    BT_FOCUS_LOGW("Open Bluetooth app for call failed: ret=%d", ret);
}

static void handle_call_video_started(void)
{
    s_call_triggered_bluetooth_open = false;
    s_call_prev_app = APP_NONE;
    clear_pending_call_video_open();

    (void)videofocusmanager_set_app_priority(APP_BLUETOOTH, APP_PRIORITY_MEDIUM);
    try_open_bluetooth_for_call();
}

static void handle_call_video_finished(void)
{
    int ret;

    clear_pending_call_video_open();
    restore_bluetooth_video_priority();

    if (s_call_triggered_bluetooth_open &&
        videofocusmanager_is_in_stack(APP_BLUETOOTH)) {
        ret = videofocusmanager_close_app(APP_BLUETOOTH);
        if (ret != APP_OK) {
            BT_FOCUS_LOGW("Close auto-opened Bluetooth app failed: ret=%d",
                          ret);
        }
    }

    s_call_triggered_bluetooth_open = false;
    s_call_prev_app = APP_NONE;
}

static void handle_power_off(void)
{
    if (s_local_ringtone_playing) {
        bt_app_stop_call_ringtone();
    }
    s_local_ringtone_playing = false;
    s_carplay_call_conflict = false;
    s_music_paused_by_focus = false;
    s_music_resume_suppressed_by_call = false;
    s_music_stop_deferred = false;
    release_music_focus();
    release_call_focus();
    handle_call_video_finished();
}

static void defer_music_stop(void)
{
    if (!s_music_focus_held) {
        return;
    }

    s_music_stop_deferred = true;
    BT_FOCUS_LOGI("Bluetooth music stop deferred for call detection");
}

static void cancel_deferred_music_stop(void)
{
    s_music_stop_deferred = false;
}

static void settle_deferred_music_stop(const BtAppSnapshot *snap)
{
    if (!s_music_stop_deferred || snap == NULL) {
        return;
    }

    if (snap->music_playing_state || snap->in_call || s_music_paused_by_focus) {
        s_music_stop_deferred = false;
        return;
    }

    s_music_stop_deferred = false;
    release_music_focus();
}

static void on_music_focus_changed(const audio_focus_change_info_t *info, void *user_data)
{
    BtAppSnapshot snapshot;

    (void)user_data;

    if (!s_initialized || info == NULL) {
        return;
    }

    switch (info->state) {
    case FOCUS_GRANTED:
        cancel_deferred_music_stop();
        bt_focus_set_music_audio(true);
        if (s_music_paused_by_focus) {
            bt_app_get_snapshot(&snapshot);
            if (!s_music_resume_suppressed_by_call && !snapshot.music_playing_state) {
                bt_app_music_play_pause();
                BT_FOCUS_LOGI("Bluetooth music resumed after audio focus grant");
            }
            s_music_paused_by_focus = false;
            s_music_resume_suppressed_by_call = false;
        }
        s_music_focus_held = true;
        break;
    case FOCUS_LOST_TRANSIENT:
    case FOCUS_LOST_TRANSIENT_MAY_DUCK:
        cancel_deferred_music_stop();
        bt_focus_set_music_audio(false);
        bt_app_get_snapshot(&snapshot);
        if (snapshot.music_playing_state) {
            /*
             * No Bluetooth ducking API is available now, so MAY_DUCK uses
             * the same pause behavior as a normal transient loss.
             */
            bt_app_music_pause();
            BT_FOCUS_LOGI("Bluetooth music paused by audio focus loss");
        }
        s_music_paused_by_focus = true;
        s_music_resume_suppressed_by_call = false;
        break;
    case FOCUS_LOST:
        cancel_deferred_music_stop();
        bt_app_get_snapshot(&snapshot);
        if (snapshot.music_playing_state) {
            bt_app_music_pause();
            BT_FOCUS_LOGI("Bluetooth music paused by permanent audio focus loss");
        }
        bt_focus_set_music_audio(false);
        s_music_focus_held = false;
        s_music_paused_by_focus = false;
        s_music_resume_suppressed_by_call = false;
        BT_FOCUS_LOGI("Bluetooth music audio focus lost permanently");
        break;
    default:
        break;
    }
}

static void on_call_focus_changed(const audio_focus_change_info_t *info, void *user_data)
{
    BtAppSnapshot snapshot;

    (void)user_data;

    if (!s_initialized || info == NULL) {
        return;
    }

    switch (info->state) {
    case FOCUS_LOST_TRANSIENT:
    case FOCUS_LOST_TRANSIENT_MAY_DUCK:
        s_carplay_call_conflict = is_carplay_call_focus_owner();
        stop_local_ringtone_for_focus_loss();
        stop_call_audio_policies();
        break;
    case FOCUS_LOST:
        s_carplay_call_conflict = is_carplay_call_focus_owner();
        stop_local_ringtone_for_focus_loss();
        s_call_focus_held = false;
        s_music_focus_held = false;
        s_music_paused_by_focus = false;
        s_music_resume_suppressed_by_call = false;
        stop_call_audio_policies();
        bt_focus_set_music_audio(false);
        break;
    case FOCUS_GRANTED:
    case FOCUS_GRANTED_TRANSIENT:
    case FOCUS_GRANTED_TRANSIENT_MAY_DUCK:
        if (s_call_focus_held && is_bluetooth_call_focus_owner()) {
            /* CarPlay released the transient focus; restore BT call audio. */
            s_carplay_call_conflict = false;
            bt_app_get_snapshot(&snapshot);
            sync_call_ringtone_and_route(&snapshot);
            sync_call_audio_policies(&snapshot);
        }
        break;
    default:
        break;
    }
    BT_FOCUS_LOGI("Bluetooth call audio focus changed: state=%d", (int)info->state);
}

static void on_bluetooth_audio_focus_changed(const audio_focus_change_info_t *info, void *user_data)
{
    if (info == NULL) {
        return;
    }

    if (info->focus_type == FOCUS_CALL) {
        on_call_focus_changed(info, user_data);
    } else {
        on_music_focus_changed(info, user_data);
    }
}

static void on_bluetooth_video_focus_changed(app_id_t app_id, bool focused, void *user_data)
{
    app_id_t current;

    (void)app_id;
    (void)user_data;

    if (!s_initialized || focused || !s_prev_snapshot.in_call) {
        return;
    }

    current = videofocusmanager_get_current_app();
    if (current == APP_RVC) {
        BT_FOCUS_LOGI("Bluetooth call UI preempted by RVC");
        return;
    }

    s_call_triggered_bluetooth_open = false;
    s_call_prev_app = APP_NONE;
    BT_FOCUS_LOGI("Bluetooth call auto-open flag cleared by user navigation");
}

int bt_focus_adapter_init(void)
{
    if (s_initialized) {
        BT_FOCUS_LOGW("Already initialized");
        return 0;
    }

    memset(&s_prev_snapshot, 0, sizeof(s_prev_snapshot));
    s_has_prev_snapshot = false;
    s_music_focus_held = false;
    s_call_focus_held = false;
    s_music_paused_by_focus = false;
    s_music_resume_suppressed_by_call = false;
    s_call_triggered_bluetooth_open = false;
    s_music_stop_deferred = false;
    s_bluetooth_music_page_selected = false;
    s_music_policy_started = false;
    s_call_policy_started = false;
    s_local_ringtone_policy_started = false;
    s_local_ringtone_playing = false;
    s_carplay_call_conflict = false;
    s_call_video_open_pending = false;
    s_call_video_blocker = APP_NONE;
    s_call_prev_app = APP_NONE;
    s_initialized = true;

    audiofocusmanager_register_focus_callback(BT_AUDIO_FOCUS_ID, on_bluetooth_audio_focus_changed, NULL);
    videofocusmanager_register_focus_callback(APP_BLUETOOTH, on_bluetooth_video_focus_changed, NULL);

    BT_FOCUS_LOGI("Bluetooth focus adapter initialized");
    bt_focus_adapter_sync();
    return 0;
}

void bt_focus_adapter_deinit(void)
{
    if (!s_initialized) {
        return;
    }

    audiofocusmanager_unregister_focus_callback(BT_AUDIO_FOCUS_ID);
    if (s_local_ringtone_playing) {
        bt_app_stop_call_ringtone();
    }
    release_call_focus();
    release_music_focus();
    restore_bluetooth_video_priority();

    memset(&s_prev_snapshot, 0, sizeof(s_prev_snapshot));
    s_has_prev_snapshot = false;
    s_music_focus_held = false;
    s_call_focus_held = false;
    s_music_paused_by_focus = false;
    s_music_resume_suppressed_by_call = false;
    s_call_triggered_bluetooth_open = false;
    s_music_stop_deferred = false;
    s_bluetooth_music_page_selected = false;
    s_music_policy_started = false;
    s_call_policy_started = false;
    s_local_ringtone_policy_started = false;
    s_local_ringtone_playing = false;
    s_carplay_call_conflict = false;
    s_call_video_open_pending = false;
    s_call_video_blocker = APP_NONE;
    s_call_prev_app = APP_NONE;
    s_initialized = false;

    BT_FOCUS_LOGI("Bluetooth focus adapter deinitialized");
}

void bt_focus_adapter_set_bluetooth_music_page_selected(bool selected)
{
    if (s_bluetooth_music_page_selected == selected) {
        return;
    }

    s_bluetooth_music_page_selected = selected;
    if (!s_initialized) {
        return;
    }

    BT_FOCUS_LOGI("Bluetooth music page selected=%d", selected ? 1 : 0);
    bt_focus_adapter_sync();
}

void bt_focus_adapter_poll(void)
{
    BtAppSnapshot snapshot;
    app_id_t current;

    if (!s_initialized || !s_call_video_open_pending) {
        return;
    }

    bt_app_get_snapshot(&snapshot);
    if (!snapshot.in_call) {
        clear_pending_call_video_open();
        return;
    }

    current = videofocusmanager_get_current_app();
    if (current == s_call_video_blocker) {
        return;
    }

    BT_FOCUS_LOGI("Call UI blocker changed: %d -> %d, retry open",
                  (int)s_call_video_blocker, (int)current);
    try_open_bluetooth_for_call();
}

void bt_focus_adapter_sync(void)
{
    BtAppSnapshot snap;

    if (!s_initialized) {
        return;
    }

    bt_app_get_snapshot(&snap);

    if (!s_has_prev_snapshot) {
        memset(&s_prev_snapshot, 0, sizeof(s_prev_snapshot));
        s_has_prev_snapshot = true;
    }

    if (s_prev_snapshot.power_on && !snap.power_on) {
        handle_power_off();
        s_prev_snapshot = snap;
        return;
    }

    if (!s_prev_snapshot.in_call && snap.in_call) {
        s_carplay_call_conflict =
            has_connected_carplay_call() || is_carplay_call_focus_owner();
        BT_FOCUS_LOGI("Bluetooth call started, CarPlay call conflict=%d",
                      s_carplay_call_conflict ? 1 : 0);
        cancel_deferred_music_stop();
        if (s_music_focus_held &&
            s_prev_snapshot.music_playing_state &&
            !snap.music_playing_state) {
            s_music_paused_by_focus = true;
            s_music_resume_suppressed_by_call = true;
        }
        if (!s_carplay_call_conflict) {
            request_call_focus();
        } else {
            BT_FOCUS_LOGI("CarPlay owns call focus; Bluetooth call focus request skipped");
        }
        handle_call_video_started();
    } else if (s_prev_snapshot.in_call && !snap.in_call) {
        release_call_focus();
        handle_call_video_finished();
        s_carplay_call_conflict = false;
    }

    sync_call_ringtone_and_route(&snap);
    sync_call_audio_policies(&snap);

    if (snap.music_playing_state != s_prev_snapshot.music_playing_state) {
        if (snap.music_playing_state) {
            cancel_deferred_music_stop();
            if (is_bluetooth_music_focus_owner()) {
                /*
                 * Reassert the audio gate for a newly started A2DP stream.
                 * The vendor player may create a new streaming session.
                 */
                bt_focus_set_music_audio(true);
            } else if (!snap.in_call && is_bluetooth_music_page_active()) {
                if (s_music_focus_held) {
                    block_music_without_focus();
                } else {
                    (void)request_music_focus(false);
                }
            } else {
                block_music_without_focus();
            }
        } else {
            if (snap.in_call || s_music_paused_by_focus) {
                cancel_deferred_music_stop();
            } else {
                defer_music_stop();
            }
        }
    } else if (snap.music_playing_state &&
               !is_bluetooth_music_focus_owner()) {
        cancel_deferred_music_stop();
        if (!s_music_focus_held &&
            !snap.in_call &&
            is_bluetooth_music_page_active()) {
            (void)request_music_focus(false);
        } else {
            block_music_without_focus();
        }
    } else {
        settle_deferred_music_stop(&snap);
    }

    s_prev_snapshot = snap;
}
