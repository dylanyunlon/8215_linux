#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "bt_app.h"

#include "bt_a2dp.h"
#include "bt_avrcp.h"
#include "bt_core.h"
#include "bt_gap.h"
#include "bt_hfp.h"
#include "bt_pbap.h"
#include "bt_platform.h"

#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define BT_APP_CONFIG_PATH_LEN 260U
#define BT_APP_RETRY_INTERVAL_MS 5000U
#define BT_APP_SCAN_POWER_TIMEOUT_MS 15000U

typedef enum BtAppInitState {
    BT_APP_INIT_NOT_STARTED = 0,
    BT_APP_INIT_IN_PROGRESS,
    BT_APP_INIT_READY,
    BT_APP_INIT_FAILED
} BtAppInitState;

static pthread_mutex_t s_init_mutex = PTHREAD_MUTEX_INITIALIZER;
static BtAppInitState s_init_state = BT_APP_INIT_NOT_STARTED;
static bool s_core_initialized = false;
static char s_async_config_path[BT_APP_CONFIG_PATH_LEN];
static BtAppConfig s_async_config;
static uint64_t s_next_retry_ms;
static bool s_scan_after_power_on = false;
static uint64_t s_scan_after_power_on_deadline_ms = 0U;

static uint64_t monotonic_time_ms(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0U;
    }

    return ((uint64_t)ts.tv_sec * 1000U) + ((uint64_t)ts.tv_nsec / 1000000U);
}

static void copy_config_path(char *dst, size_t dst_size, const char *src)
{
    size_t len;

    if (dst == NULL || dst_size == 0U) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    len = strlen(src);
    if (len >= dst_size) {
        len = dst_size - 1U;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
}

static void set_init_state(BtAppInitState state)
{
    pthread_mutex_lock(&s_init_mutex);
    s_init_state = state;
    s_next_retry_ms = state == BT_APP_INIT_FAILED ?
                      monotonic_time_ms() + BT_APP_RETRY_INTERVAL_MS : 0U;
    pthread_mutex_unlock(&s_init_mutex);
}

static void set_core_initialized(bool initialized)
{
    pthread_mutex_lock(&s_init_mutex);
    s_core_initialized = initialized;
    pthread_mutex_unlock(&s_init_mutex);
}

static void bt_app_get_runtime_state(bool *core_initialized, BtAppInitState *state)
{
    pthread_mutex_lock(&s_init_mutex);
    if (core_initialized != NULL) {
        *core_initialized = s_core_initialized;
    }
    if (state != NULL) {
        *state = s_init_state;
    }
    pthread_mutex_unlock(&s_init_mutex);
}

static bool bt_app_ready(void)
{
    BtAppInitState state;

    bt_app_get_runtime_state(NULL, &state);
    return state == BT_APP_INIT_READY;
}

static bool bt_app_try_begin_init(void)
{
    bool should_begin = false;

    pthread_mutex_lock(&s_init_mutex);
    if (s_init_state == BT_APP_INIT_NOT_STARTED || s_init_state == BT_APP_INIT_FAILED) {
        s_init_state = BT_APP_INIT_IN_PROGRESS;
        s_next_retry_ms = 0U;
        should_begin = true;
    }
    pthread_mutex_unlock(&s_init_mutex);

    return should_begin;
}

static int bt_app_init_worker(const BtAppConfig *config)
{
    bool core_initialized;
    int ret;

    fprintf(stderr, "[bt-app] worker begin\n");
    bt_app_get_runtime_state(&core_initialized, NULL);
    if (!core_initialized) {
        fprintf(stderr, "[bt-app] bt_core_init begin\n");
        ret = bt_core_init(config);
        fprintf(stderr, "[bt-app] bt_core_init ret=%d\n", ret);
        if (ret != 0) {
            set_core_initialized(false);
            set_init_state(BT_APP_INIT_FAILED);
            return ret;
        }
        set_core_initialized(true);
    }

    fprintf(stderr, "[bt-app] bt_platform_init begin\n");
    ret = bt_platform_init();
    fprintf(stderr, "[bt-app] bt_platform_init ret=%d\n", ret);
    if (ret != 0) {
        bt_core_notify_status("Bluetooth platform init failed");
        set_init_state(BT_APP_INIT_FAILED);
        return ret;
    }

    (void)bt_gap_init();
    (void)bt_hfp_init();
    (void)bt_pbap_init();
    (void)bt_avrcp_init();
    (void)bt_a2dp_init();

    set_init_state(BT_APP_INIT_READY);
    return 0;
}

static void *bt_app_init_thread(void *arg)
{
    const BtAppConfig *config = (const BtAppConfig *)arg;

    (void)bt_app_init_worker(config);
    return NULL;
}

int bt_app_init(const BtAppConfig *config)
{
    BtAppInitState state;

    bt_app_get_runtime_state(NULL, &state);
    if (state == BT_APP_INIT_READY) {
        return 0;
    }
    if (state == BT_APP_INIT_IN_PROGRESS) {
        return -1;
    }
    if (!bt_app_try_begin_init()) {
        return -1;
    }

    return bt_app_init_worker(config);
}

void bt_app_init_async(const BtAppConfig *config)
{
    pthread_t thread;
    int ret;

    fprintf(stderr, "[bt-app] init_async request\n");
    pthread_mutex_lock(&s_init_mutex);
    if (s_init_state != BT_APP_INIT_NOT_STARTED && s_init_state != BT_APP_INIT_FAILED) {
        pthread_mutex_unlock(&s_init_mutex);
        return;
    }

    s_init_state = BT_APP_INIT_IN_PROGRESS;
    s_next_retry_ms = 0U;
    if (config != NULL) {
        copy_config_path(s_async_config_path, sizeof(s_async_config_path), config->config_path);
        s_async_config.config_path = s_async_config_path[0] != '\0' ? s_async_config_path : NULL;
    }
    pthread_mutex_unlock(&s_init_mutex);

    ret = pthread_create(&thread, NULL, bt_app_init_thread, &s_async_config);
    fprintf(stderr, "[bt-app] init_async pthread_create ret=%d\n", ret);
    if (ret != 0) {
        set_init_state(BT_APP_INIT_FAILED);
        return;
    }

    (void)pthread_detach(thread);
}

bool bt_app_is_ready(void)
{
    return bt_app_ready();
}

void bt_app_deinit(void)
{
    bool core_initialized;
    BtAppInitState state;

    bt_app_get_runtime_state(&core_initialized, &state);
    if (state == BT_APP_INIT_IN_PROGRESS) {
        return;
    }

    if (state == BT_APP_INIT_READY) {
        bt_a2dp_deinit();
        bt_avrcp_deinit();
        bt_pbap_deinit();
        bt_hfp_deinit();
        bt_gap_deinit();
        bt_platform_deinit();
    }
    if (core_initialized) {
        bt_core_deinit();
    }

    pthread_mutex_lock(&s_init_mutex);
    s_init_state = BT_APP_INIT_NOT_STARTED;
    s_core_initialized = false;
    s_next_retry_ms = 0U;
    pthread_mutex_unlock(&s_init_mutex);
}

void bt_app_poll(void)
{
    bool core_initialized;
    BtAppInitState state;
    uint64_t next_retry_ms;
    uint64_t now_ms;
    char auto_connect_address[96];

    bt_app_get_runtime_state(&core_initialized, &state);
    if (core_initialized) {
        bt_core_poll();
    }
    pthread_mutex_lock(&s_init_mutex);
    state = s_init_state;
    next_retry_ms = s_next_retry_ms;
    pthread_mutex_unlock(&s_init_mutex);
    now_ms = monotonic_time_ms();
    if (state == BT_APP_INIT_FAILED && next_retry_ms != 0U && now_ms >= next_retry_ms) {
        bt_app_init_async(NULL);
        return;
    }
    if (!core_initialized) {
        return;
    }
    if (state != BT_APP_INIT_READY) {
        return;
    }

    bt_platform_poll();
    if (s_scan_after_power_on) {
        BtAppSnapshot snapshot;

        bt_app_get_snapshot(&snapshot);
        if (snapshot.power_on) {
            s_scan_after_power_on = false;
            s_scan_after_power_on_deadline_ms = 0U;
            bt_gap_start_scan();
        } else if (s_scan_after_power_on_deadline_ms != 0U &&
                   now_ms >= s_scan_after_power_on_deadline_ms) {
            s_scan_after_power_on = false;
            s_scan_after_power_on_deadline_ms = 0U;
            bt_core_notify_status("Bluetooth power on timeout");
        }
    }
    if (bt_core_pending_auto_connect(auto_connect_address, sizeof(auto_connect_address))) {
        bt_hfp_connect_phone(auto_connect_address);
    }
}

void bt_app_set_ui_hooks(const BtAppUiHooks *hooks)
{
    bt_core_set_ui_hooks(hooks);
}

void bt_app_get_snapshot(BtAppSnapshot *snapshot)
{
    bt_core_get_snapshot(snapshot);
}

void bt_app_enter_main_page(void)
{
    bt_core_enter_main_page();
}

void bt_app_go_home(void)
{
    bt_core_go_home();
}

void bt_app_set_power(bool enabled)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_gap_set_power(enabled);
}

void bt_app_start_scan(void)
{
    BtAppSnapshot snapshot;

    if (!bt_app_ready()) {
        return;
    }
    if (s_scan_after_power_on) {
        bt_core_notify_status("Bluetooth powering on; scan pending");
        return;
    }
    bt_app_get_snapshot(&snapshot);
    if (!snapshot.power_on) {
        s_scan_after_power_on = true;
        s_scan_after_power_on_deadline_ms = monotonic_time_ms() + BT_APP_SCAN_POWER_TIMEOUT_MS;
        bt_core_notify_status("Bluetooth power off; powering on...");
        bt_gap_set_power(true);
        return;
    }
    bt_gap_start_scan();
}

void bt_app_stop_scan(void)
{
    if (!bt_app_ready()) {
        return;
    }
    s_scan_after_power_on = false;
    s_scan_after_power_on_deadline_ms = 0U;
    bt_gap_stop_scan();
}

void bt_app_pair_device(const char *address)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_gap_pair_device(address);
}

void bt_app_remove_device(const char *address)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_gap_remove_device(address);
}

void bt_app_connect_phone(const char *address)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_hfp_connect_phone(address);
}

void bt_app_disconnect_phone(const char *address)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_core_notify_manual_disconnect();
    bt_hfp_disconnect_phone(address);
}

void bt_app_dial(const char *number)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_hfp_dial(number);
}

void bt_app_redial(void)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_hfp_redial();
}

void bt_app_accept_call(void)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_hfp_accept_call();
}

void bt_app_hangup_call(void)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_hfp_hangup_call();
}

void bt_app_hold_call(BtCallAction action)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_hfp_hold_call(action);
}

void bt_app_switch_call_audio(bool phone_audio)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_hfp_switch_audio(phone_audio);
}

void bt_app_play_call_ringtone(void)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_hfp_play_ringtone();
}

void bt_app_stop_call_ringtone(void)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_hfp_stop_ringtone();
}

void bt_app_send_dtmf(const char *code)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_hfp_send_dtmf(code);
}

void bt_app_download_phonebook(void)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_pbap_download_phonebook();
}

void bt_app_download_call_records(void)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_pbap_download_call_records();
}

void bt_app_abort_pbap_download(void)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_pbap_abort_download();
}

bool bt_app_get_contact(size_t index, BtContact *out)
{
    return bt_core_get_contact(index, out);
}

bool bt_app_get_call_record(size_t index, BtCallRecordType filter, BtCallRecord *out)
{
    return bt_core_get_call_record(index, filter, out);
}

void bt_app_search_phonebook(const char *keyword)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_core_search_phonebook(keyword);
}

void bt_app_filter_call_records(BtCallRecordType type)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_core_filter_call_records(type);
}

void bt_app_music_play_pause(void)
{
    BtAppSnapshot snapshot;

    if (!bt_app_ready()) {
        return;
    }
    bt_core_get_snapshot(&snapshot);
    bt_avrcp_play_pause(snapshot.music_playing);
}

void bt_app_music_pause(void)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_avrcp_pause();
}

void bt_app_music_prev(void)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_avrcp_prev();
}

void bt_app_music_next(void)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_avrcp_next();
}

void bt_app_set_music_audio_enabled(bool enabled)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_a2dp_set_audio_enabled(enabled);
}

void bt_app_set_auto_connect(bool enabled)
{
    if (!bt_app_ready()) {
        return;
    }
    bt_core_set_auto_connect(enabled);
}

void bt_app_set_local_name(const char *name)
{
    if (!bt_app_ready()) {
        return;
    }
    if (bt_platform_set_local_name(name) != 0) {
        bt_core_notify_status("Set Bluetooth name failed");
    }
}

void bt_app_set_pairing_pin(const char *pin)
{
    if (!bt_app_ready()) {
        return;
    }
    if (bt_platform_set_pairing_pin(pin) != 0) {
        bt_core_notify_status("Set Bluetooth PIN failed");
    }
}
