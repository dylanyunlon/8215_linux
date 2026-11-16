#include "bt_platform.h"

#include "bt_event_queue.h"
#include "bt_types.h"

#include "bluetoothapi.h"
#include "bluetoothgapindications.h"
#include "bluetoothhandsfreeindications.h"
#include "bluetootha2dpindications.h"
#include "bluetoothavrcpindications.h"
#include "bluetoothpbapindications.h"
#include "bluetoothmessageextras.h"

#include <memory>
#include <string>
#include <atomic>
#include <mutex>
#include <cstring>
#include <vector>
#include <list>
#include <cstdio>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <dlfcn.h>

#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT NULL
#endif

#define BT_HAVE_ICONV 1

namespace {

struct BtSdkBackend;

std::unique_ptr<BtSdkBackend> s_backend;

enum PbapRequestKind {
    PBAP_REQUEST_NONE = 0,
    PBAP_REQUEST_PHONEBOOK,
    PBAP_REQUEST_CALL_RECORDS
};

BtSdkBackend *backend();
BtPbapDownloadType current_pbap_download_type();
void sync_hfp_call_list();
void refresh_pbap_records(int path);
void refresh_media_metadata_from_backend();
void post_cached_media_metadata_from_backend();
void post_current_local_info();
void sync_bonded_devices_from_backend();
void set_a2dp_audio_enabled(bool enabled);
bool is_music_playing_from_backend();
void update_music_playing_state(bool playing, const std::string &address);
void post_music_position(int position_ms, int duration_ms);
void request_initial_play_status_from_backend();
void request_play_status_from_backend();
void reset_music_playing_cache();
void reset_profile_connection_cache();
void set_sdk_power_on(bool powered_on);
void queue_user_confirm(const std::string &address);

uint64_t platform_monotonic_ms()
{
    using namespace std::chrono;

    return (uint64_t)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

void copy_string(char *dst, size_t dst_size, const std::string &src)
{
    bt_copy_string(dst, dst_size, src.c_str());
}

bool copy_valid_utf8_display(const std::string &src, std::string &out)
{
    std::vector<char> buffer(src.size() + 1U);

    if (!bt_copy_valid_utf8_display_bytes(buffer.data(), buffer.size(), src.data(), src.size())) {
        out.clear();
        return false;
    }
    out.assign(buffer.data());
    return true;
}

bool copy_local_name_for_sdk(const char *name, std::string &out)
{
    char buffer[BT_LOCAL_NAME_TEXT_LEN];

    if (name == NULL || name[0] == '\0') {
        out.clear();
        return false;
    }
    if (!bt_copy_valid_utf8_display_bytes(buffer, sizeof(buffer), name, std::strlen(name))) {
        out.clear();
        return false;
    }

    out.assign(buffer);
    return !out.empty();
}

#ifdef BT_HAVE_ICONV
typedef void *(*BtIconvOpenFn)(const char *, const char *);
typedef size_t (*BtIconvFn)(void *, char **, size_t *, char **, size_t *);
typedef int (*BtIconvCloseFn)(void *);

struct BtIconvApi {
    BtIconvOpenFn open;
    BtIconvFn convert;
    BtIconvCloseFn close;
    void *handle;
    bool attempted;
};

BtIconvApi &iconv_api()
{
    static BtIconvApi api = { NULL, NULL, NULL, NULL, false };
    const char *libs[] = { "libiconv.so", "libiconv.so.2", "libc.so.6" };

    if (api.attempted) {
        return api;
    }

    api.attempted = true;
    api.open = reinterpret_cast<BtIconvOpenFn>(dlsym(RTLD_DEFAULT, "iconv_open"));
    api.convert = reinterpret_cast<BtIconvFn>(dlsym(RTLD_DEFAULT, "iconv"));
    api.close = reinterpret_cast<BtIconvCloseFn>(dlsym(RTLD_DEFAULT, "iconv_close"));
    if (api.open != NULL && api.convert != NULL && api.close != NULL) {
        return api;
    }

    for (size_t i = 0U; i < sizeof(libs) / sizeof(libs[0]); ++i) {
        api.handle = dlopen(libs[i], RTLD_LAZY);
        if (api.handle == NULL) {
            continue;
        }
        api.open = reinterpret_cast<BtIconvOpenFn>(dlsym(api.handle, "iconv_open"));
        api.convert = reinterpret_cast<BtIconvFn>(dlsym(api.handle, "iconv"));
        api.close = reinterpret_cast<BtIconvCloseFn>(dlsym(api.handle, "iconv_close"));
        if (api.open != NULL && api.convert != NULL && api.close != NULL) {
            return api;
        }
        dlclose(api.handle);
        api.handle = NULL;
    }

    api.open = NULL;
    api.convert = NULL;
    api.close = NULL;
    return api;
}

bool convert_text_encoding(const std::string &src, const char *from_encoding, std::string &out)
{
    BtIconvApi &api = iconv_api();
    void *cd;
    std::string buffer;
    char *inbuf;
    size_t inleft;
    size_t outleft;
    char *outbuf;

    if (api.open == NULL || api.convert == NULL || api.close == NULL) {
        return false;
    }

    cd = api.open("UTF-8", from_encoding);
    if (cd == (void *)-1 || cd == NULL) {
        return false;
    }

    buffer.resize(src.size() * 4U + 8U);
    inbuf = const_cast<char *>(src.data());
    inleft = src.size();
    outbuf = &buffer[0];
    outleft = buffer.size();

    while (inleft > 0U) {
        size_t ret = api.convert(cd, &inbuf, &inleft, &outbuf, &outleft);
        if (ret != (size_t)-1) {
            continue;
        }
        if (errno == E2BIG) {
            size_t used = (size_t)(outbuf - &buffer[0]);
            buffer.resize(buffer.size() * 2U);
            outbuf = &buffer[0] + used;
            outleft = buffer.size() - used;
            continue;
        }
        api.close(cd);
        return false;
    }

    out.assign(buffer.data(), (size_t)(outbuf - &buffer[0]));
    api.close(cd);
    return !out.empty();
}
#endif

std::string sanitize_display_text(const std::string &src, const std::string &fallback)
{
    std::string out;
    std::string converted;

    if (copy_valid_utf8_display(src, out)) {
        return out;
    }

#ifdef BT_HAVE_ICONV
    if (convert_text_encoding(src, "GB18030", converted) && copy_valid_utf8_display(converted, out)) {
        return out;
    }
    if (convert_text_encoding(src, "GBK", converted) && copy_valid_utf8_display(converted, out)) {
        return out;
    }
#endif

    if (fallback.empty()) {
        return std::string();
    }
    if (copy_valid_utf8_display(fallback, out)) {
        return out;
    }
    return "Unknown";
}

std::string normalize_address(const std::string &address)
{
    BluetoothAddress bt_address(address);

    if (bt_address.isValid()) {
        return bt_address.toString();
    }
    return address;
}

std::string message_address(const universal_utils::CMessage &message)
{
    return normalize_address(message.getStringExtra(STRING_ADDRESS, ""));
}

bool post_event(const BtEvent &event)
{
    return bt_event_post(&event);
}

void post_status(const char *status)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_STATUS;
    bt_copy_string(event.text, sizeof(event.text), status);
    post_event(event);
}

void post_device_event(BtEventType type, const std::string &address, const std::string &name)
{
    BtEvent event;
    std::string text_address = normalize_address(address);

    memset(&event, 0, sizeof(event));
    event.type = type;
    copy_string(event.address, sizeof(event.address), text_address);
    copy_string(event.name, sizeof(event.name), sanitize_display_text(name, text_address));
    post_event(event);
}

void post_bool_event(BtEventType type, bool value, const std::string &address = std::string())
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.value = value;
    copy_string(event.address, sizeof(event.address), normalize_address(address));
    post_event(event);
}

void post_call_state(BtAppCallState state)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_STATE;
    event.state = state;
    post_event(event);
}

void post_ringtone_type(BtRingtoneType type)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_RINGTONE_TYPE;
    event.state = type;
    post_event(event);
}

void post_ringtone(void)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_RINGTONE;
    post_event(event);
}

void post_call_number(const std::string &number)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_NUMBER;
    copy_string(event.number, sizeof(event.number), number);
    post_event(event);
}

void post_media_info(const std::string &title, const std::string &artist, const std::string &album)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_MEDIA_INFO;
    copy_string(event.media_title, sizeof(event.media_title), sanitize_display_text(title, "Unknown"));
    copy_string(event.media_artist, sizeof(event.media_artist), sanitize_display_text(artist, "Unknown"));
    copy_string(event.media_album, sizeof(event.media_album), sanitize_display_text(album, "Unknown"));
    post_event(event);
}

BtPbapDownloadType pbap_download_type_from_request(PbapRequestKind request)
{
    switch (request) {
    case PBAP_REQUEST_PHONEBOOK:
        return BT_PBAP_DOWNLOAD_PHONEBOOK;
    case PBAP_REQUEST_CALL_RECORDS:
        return BT_PBAP_DOWNLOAD_CALL_RECORDS;
    case PBAP_REQUEST_NONE:
    default:
        return BT_PBAP_DOWNLOAD_NONE;
    }
}

void post_pbap_state(BtPbapState state, const char *status, int current_index = -1, int total_count = -1)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_PBAP_STATE;
    event.state = state;
    event.pbap_download_type = current_pbap_download_type();
    event.pbap_current_index = current_index;
    event.pbap_total_count = total_count;
    bt_copy_string(event.text, sizeof(event.text), status);
    post_event(event);
}

void post_local_info(const std::string &name, const std::string &pin)
{
    BtEvent event;

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_LOCAL_INFO;
    copy_string(event.local_name, sizeof(event.local_name), sanitize_display_text(name, ""));
    copy_string(event.pairing_pin, sizeof(event.pairing_pin), sanitize_display_text(pin, ""));
    post_event(event);
}

class GapCallback : public IBluetoothCallBack {
public:
    int onIndication(const universal_utils::CMessage &message) override
    {
        switch (message.what) {
        case GAP_POWERON_IND:
            fprintf(stderr, "[bt-sdk] power on\n");
            set_sdk_power_on(true);
            post_bool_event(BT_EVENT_POWER_STATE, true);
            break;
        case GAP_POWEROFF_IND:
            fprintf(stderr, "[bt-sdk] power off\n");
            set_sdk_power_on(false);
            post_bool_event(BT_EVENT_POWER_STATE, false);
            break;
        case GAP_DISCOVERY_START_IND:
            fprintf(stderr, "[bt-sdk] discovery started\n");
            post_bool_event(BT_EVENT_SCAN_STATE, true);
            break;
        case GAP_DISCOVERY_STOP_IND:
            fprintf(stderr, "[bt-sdk] discovery stopped\n");
            post_bool_event(BT_EVENT_SCAN_STATE, false);
            break;
        case GAP_DISCOVERY_RESULT_IND:
        case GAP_DISCOVERY_UPDATE_IND: {
            std::string address = message_address(message);
            std::string name = message.getStringExtra(STRING_NAME, "");
            fprintf(stderr, "[bt-sdk] device %s (%s)\n", address.c_str(), name.c_str());
            post_device_event(BT_EVENT_DEVICE_FOUND, address, name);
            break;
        }
        case GAP_BONDING_RESULT_IND:
            if (message.arg1 == 0) {
                post_device_event(BT_EVENT_DEVICE_PAIRED, message_address(message),
                                  message.getStringExtra(STRING_NAME, ""));
            } else {
                post_status("Bluetooth pair failed");
            }
            break;
        case GAP_BOND_REMOVED_IND:
            post_device_event(BT_EVENT_DEVICE_REMOVED, message_address(message), "");
            break;
        case GAP_SECURITY_USER_CONFIRM_IND: {
            queue_user_confirm(message_address(message));
            break;
        }
        case GAP_LINK_STATE_IND: {
            int error_code = message.getIntExtra(INT_ERROR_CODE, 0);
            if (error_code == HCI_ERR_CONNECTION_TIMEOUT || error_code == HCI_ERR_LMP_RESPONSE_TIMEOUT) {
                post_status("Bluetooth link lost");
            }
            break;
        }
        default:
            break;
        }
        return 0;
    }
};

class HfpCallback : public IBluetoothCallBack {
public:
    int onIndication(const universal_utils::CMessage &message) override
    {
        switch (message.what) {
        case HF_IND_DEVICE_CONNECTED:
            if (message.arg1 == 0) {
                post_bool_event(BT_EVENT_PHONE_CONNECTED, true, message_address(message));
            } else {
                post_status("Phone audio connect failed");
            }
            break;
        case HF_IND_DEVICE_DISCONNECTED:
            post_bool_event(BT_EVENT_PHONE_DISCONNECTED, false, message_address(message));
            break;
        case HF_IND_DEVICE_CONNECTING:
            post_status("Phone audio connecting");
            break;
        case HF_IND_CALLSTATE:
            handle_call_state(message.getIntExtra(INT_HF_CALLSTATE, HF_CALLSTATE_IDLE));
            break;
        case HF_IND_RINGTONE_TYPE:
            post_ringtone_type(
                message.getIntExtra(INT_HF_RINGTONE_TYPE, HF_RINGTONE_TYPE_LOCAL) ==
                        HF_RINGTONE_TYPE_INBAND
                    ? BT_RINGTONE_TYPE_INBAND
                    : BT_RINGTONE_TYPE_LOCAL);
            break;
        case HF_IND_RINGTONE:
            post_ringtone();
            break;
        case HF_IND_CALLNUMBER:
            post_call_number(message.getStringExtra(STRING_HF_CALLNUMBER, ""));
            break;
        case HF_IND_CALLAUDIOTRANSFER: {
            int towards = message.getIntExtra(STRING_HF_AUDIOTOWARDS, HF_AUDIOTOWARDS_HF);
            if (towards != HF_AUDIOTOWARDS_AG && towards != HF_AUDIOTOWARDS_HF) {
                towards = HF_AUDIOTOWARDS_HF;
            }
            post_bool_event(BT_EVENT_CALL_AUDIO_STATE, towards == HF_AUDIOTOWARDS_AG, message_address(message));
            break;
        }
        case HF_IND_CALLCHANGE:
            sync_hfp_call_list();
            break;
        case HF_IND_ATRESULT:
            if (message.getIntExtra(INT_HF_RESULT_CODE, 0) == HF_AT_RESULT_ERROR_NO_ANSWER) {
                post_status("The phone did not answer");
            }
            break;
        default:
            break;
        }
        return 0;
    }

private:
    static void handle_call_state(int call_state)
    {
        switch (call_state) {
        case HF_CALLSTATE_INCOMING:
            post_call_state(BT_APP_CALL_STATE_INCOMING);
            break;
        case HF_CALLSTATE_OUTGOING:
            post_call_state(BT_APP_CALL_STATE_OUTGOING);
            break;
        case HF_CALLSTATE_ALERTING:
            post_call_state(BT_APP_CALL_STATE_ALERTING);
            break;
        case HF_CALLSTATE_SPEAKING:
            post_call_state(BT_APP_CALL_STATE_SPEAKING);
            break;
        case HF_CALLSTATE_HELD:
        case HF_CALLSTATE_HELD_BY_RESPONSE_AND_HOLD:
            post_call_state(BT_APP_CALL_STATE_HELD);
            break;
        case HF_CALLSTATE_WAITING:
            post_call_state(BT_APP_CALL_STATE_WAITING);
            break;
        case HF_CALLSTATE_IDLE:
        default:
            post_call_state(BT_APP_CALL_STATE_IDLE);
            break;
        }
    }
};

class A2dpCallback : public IBluetoothCallBack {
public:
    int onIndication(const universal_utils::CMessage &message) override
    {
        switch (message.what) {
        case A2DP_IND_DEVICE_CONNECTED:
            if (message.arg1 == 0) {
                post_bool_event(BT_EVENT_MEDIA_CONNECTED, true, message_address(message));
                post_bool_event(BT_EVENT_A2DP_CONNECTED, true, message_address(message));
            } else {
                post_status("A2DP connect failed");
            }
            break;
        case A2DP_IND_DEVICE_DISCONNECTED:
            reset_music_playing_cache();
            post_bool_event(BT_EVENT_A2DP_DISCONNECTED, false, message_address(message));
            break;
        case A2DP_IND_PLAY_START:
            update_music_playing_state(true, message_address(message));
            break;
        case A2DP_IND_PLAY_SUSPEND:
            update_music_playing_state(false, message_address(message));
            break;
        default:
            break;
        }
        return 0;
    }
};

class AvrcpCallback : public IBluetoothCallBack {
public:
    int onIndication(const universal_utils::CMessage &message) override
    {
        switch (message.what) {
        case AVRCP_IND_DEVICE_CONNECTED:
            if (message.arg1 == 0) {
                post_bool_event(BT_EVENT_MEDIA_CONNECTED, true, message_address(message));
                post_bool_event(BT_EVENT_AVRCP_CONNECTED, true, message_address(message));
                request_initial_play_status_from_backend();
                if (is_music_playing_from_backend()) {
                    refresh_media_metadata_from_backend();
                }
            } else {
                post_status("AVRCP connect failed");
            }
            break;
        case AVRCP_IND_DEVICE_DISCONNECTED:
            reset_music_playing_cache();
            post_bool_event(BT_EVENT_AVRCP_DISCONNECTED, false, message_address(message));
            break;
        case AVRCP_IND_PLAYBACK_DATA_UPDATE: {
            int play_state = message.getIntExtra(BYTE_PLAYBACK_STATUS, STOPPED);
            update_music_playing_state(play_state == PLAYING, message_address(message));
            post_music_position(message.getIntExtra(INT_PLAYING_TIME, -1),
                                message.getIntExtra(INT_TOTAL_TIME, -1));
            break;
        }
        case AVRCP_IND_PLAY_STATUS_CHANGED: {
            int play_state = message.getIntExtra(BYTE_PLAYBACK_STATUS, STOPPED);
            update_music_playing_state(play_state == PLAYING, message_address(message));
            break;
        }
        case AVRCP_IND_SONG_POSITION_CHANGED:
            post_music_position(message.getIntExtra(INT_PLAYING_TIME, -1), -1);
            break;
        case AVRCP_IND_MEDIA_DATA_CHANGED:
            if (is_music_playing_from_backend()) {
                post_cached_media_metadata_from_backend();
            } else {
                request_play_status_from_backend();
            }
            break;
        default:
            break;
        }
        return 0;
    }
};

class PbapCallback : public IBluetoothCallBack {
public:
    int onIndication(const universal_utils::CMessage &message) override
    {
        switch (message.what) {
        case PBAP_IND_DOWNLOAD_START:
            post_pbap_state(BT_PBAP_STATE_DOWNLOADING, "PBAP download start", 0, 0);
            break;
        case PBAP_IND_DOWNLOAD_SIZE:
            post_pbap_state(BT_PBAP_STATE_DOWNLOADING,
                            "PBAP download size updated",
                            0,
                            message.getIntExtra(INT_PBAP_PHONEBOOK_SIZE, 0));
            break;
        case PBAP_IND_DOWNLOAD_FINISH:
            post_pbap_state(BT_PBAP_STATE_FINISHED, "PBAP download finish");
            refresh_pbap_records(message.getIntExtra(INT_PBAP_DOWNLOAD_PATH, PBMGR_UNKNOW));
            break;
        case PBAP_IND_DOWNLOAD_STOP:
            post_pbap_state(BT_PBAP_STATE_STOPPED, "PBAP download stopped");
            break;
        case PBAP_IND_DOWNLOAD_ERROR:
            post_pbap_state(BT_PBAP_STATE_ERROR, "PBAP download error");
            break;
        case PBAP_IND_DOWNLOAD_ONESTEP:
            post_pbap_state(BT_PBAP_STATE_DOWNLOADING,
                            "PBAP downloading",
                            message.getIntExtra(INT_PBAP_DOWNLOAD_INDEX, 0),
                            -1);
            break;
        default:
            break;
        }
        return 0;
    }
};

struct BtSdkBackend {
    IBluetoothClient *client = NULL;
    IBluetoothLocalDevice *local_device = NULL;
    IBluetoothHandsfree *hfp = NULL;
    IBluetoothPBAP *pbap = NULL;
    IBluetoothAvrcp *avrcp = NULL;
    IBluetoothA2dp *a2dp = NULL;
    std::unique_ptr<GapCallback> gap_cb;
    std::unique_ptr<HfpCallback> hfp_cb;
    std::unique_ptr<PbapCallback> pbap_cb;
    std::unique_ptr<AvrcpCallback> avrcp_cb;
    std::unique_ptr<A2dpCallback> a2dp_cb;
    bool last_phone_connected = false;
    bool last_media_connected = false;
    bool last_a2dp_connected = false;
    bool last_avrcp_connected = false;
    bool last_music_playing = false;
    bool initial_play_status_pending = false;
    int last_reported_position_sec = -1;
    int last_reported_duration_ms = -1;
    bool sdk_power_on = false;
    std::atomic<bool> pending_power_on_sync{false};
    std::atomic<bool> pending_power_state_retry{false};
    uint64_t next_power_state_retry_ms = 0U;
    int power_state_retry_count = 0;
    std::mutex pending_confirm_mutex;
    std::string pending_user_confirm_address;
    int last_pbap_path = PBMGR_UNKNOW;
    PbapRequestKind last_pbap_request = PBAP_REQUEST_NONE;
    std::string last_phone_address;
    std::string last_media_address;

    ~BtSdkBackend()
    {
        deinit();
    }

    int init()
    {
        fprintf(stderr, "[bt-sdk] getBluetoothClient begin\n");
        client = getBluetoothClient();
        fprintf(stderr, "[bt-sdk] getBluetoothClient ret=%p\n", (void *)client);
        if (client == NULL) {
            post_status("getBluetoothClient failed");
            return -1;
        }

        gap_cb.reset(new GapCallback());
        hfp_cb.reset(new HfpCallback());
        pbap_cb.reset(new PbapCallback());
        avrcp_cb.reset(new AvrcpCallback());
        a2dp_cb.reset(new A2dpCallback());

        fprintf(stderr, "[bt-sdk] register gap callback begin\n");
        int gap_ret = client->registerCallBack(*gap_cb);
        fprintf(stderr, "[bt-sdk] register gap callback ret=%d\n", gap_ret);
        if (gap_ret != 0) {
            post_status("Bluetooth callback registration failed");
        }
        (void)client->getLocalDevice(&local_device);
        fprintf(stderr, "[bt-sdk] local_device=%p\n", (void *)local_device);
        init_profile(HANDSFREEPROFILENAME, &hfp);
        init_profile(PBAPPROFILENAME, &pbap);
        init_profile(AVRCPPROFILENAME, &avrcp);
        init_profile(A2DPPROFILENAME, &a2dp);
        fprintf(stderr, "[bt-sdk] profiles hfp=%p pbap=%p avrcp=%p a2dp=%p\n",
                (void *)hfp, (void *)pbap, (void *)avrcp, (void *)a2dp);

        if (hfp != NULL) {
            (void)hfp->registerCallBack(*hfp_cb);
        }
        if (pbap != NULL) {
            (void)pbap->registerCallBack(*pbap_cb);
        }
        if (avrcp != NULL) {
            (void)avrcp->registerCallBack(*avrcp_cb);
        }
        if (a2dp != NULL) {
            (void)a2dp->registerCallBack(*a2dp_cb);
        }

        fprintf(stderr, "[bt-sdk] init_local_device begin\n");
        init_local_device();
        fprintf(stderr, "[bt-sdk] sync_bonded_devices begin\n");
        sync_bonded_devices();
        fprintf(stderr, "[bt-sdk] sync_bonded_devices done\n");
        if (sdk_power_on) {
            sync_profile_state();
        }
        fprintf(stderr, "[bt-sdk] init done\n");
        post_status("Bluetooth SDK initialized");
        return 0;
    }

    void deinit()
    {
        if (client != NULL && gap_cb != NULL) {
            (void)client->deregisterCallBack(*gap_cb);
        }
        if (hfp != NULL && hfp_cb != NULL) {
            (void)hfp->deregisterCallBack(*hfp_cb);
        }
        if (pbap != NULL && pbap_cb != NULL) {
            (void)pbap->deregisterCallBack(*pbap_cb);
        }
        if (avrcp != NULL && avrcp_cb != NULL) {
            (void)avrcp->deregisterCallBack(*avrcp_cb);
        }
        if (a2dp != NULL && a2dp_cb != NULL) {
            (void)a2dp->deregisterCallBack(*a2dp_cb);
        }

        gap_cb.reset();
        hfp_cb.reset();
        pbap_cb.reset();
        avrcp_cb.reset();
        a2dp_cb.reset();
        local_device = NULL;
        hfp = NULL;
        pbap = NULL;
        avrcp = NULL;
        a2dp = NULL;

        if (client != NULL) {
            releaseBluetoothClient();
            client = NULL;
        }
    }

    void poll()
    {
        std::string confirm_address;

        refresh_power_state_if_due();
        if (pending_power_on_sync.exchange(false)) {
            if (local_device != NULL) {
                (void)local_device->setScanMode(SCAN_MODE_PAGE_ON_INQUIRY_ON_LOW);
            }
            post_current_local_info();
            sync_bonded_devices();
            sync_profile_state();
        }

        {
            std::lock_guard<std::mutex> lock(pending_confirm_mutex);
            confirm_address.swap(pending_user_confirm_address);
        }
        if (!confirm_address.empty()) {
            BluetoothAddress address(confirm_address);
            if (!address.isValid()) {
                post_status("Bluetooth pairing address invalid");
            } else {
                BluetoothRemoteDevice remote(address);
                if (remote.secureUserConfirm(true) != BLUETOOTH_OK) {
                    post_status("Bluetooth pairing confirmation failed");
                }
            }
        }
    }

    int set_power(bool enabled)
    {
        E_LOCAL_DEVICE_STATE state = GAP_STATE_INVALID;
        int state_ret;
        int ret;

        if (local_device == NULL) {
            return -1;
        }
        state_ret = local_device->getBluetoothState(state);
        fprintf(stderr, "[bt-sdk] set power state ret=%d state=%d\n", state_ret, (int)state);
        if (state_ret < 0 || state == GAP_STATE_INVALID) {
            schedule_power_state_retry();
            post_status("Bluetooth service not ready");
            return -1;
        }
        if (state == GAP_STATE_POWERSWTICHING) {
            post_status("Bluetooth power transition in progress");
            return -1;
        }
        if (enabled && (state == GAP_STATE_POWERON || state == GAP_STATE_DISCOVERYING)) {
            return 0;
        }
        if (!enabled && state == GAP_STATE_POWEROFF) {
            return 0;
        }
        ret = enabled ? local_device->open() : local_device->close();
        fprintf(stderr, "[bt-sdk] set power %d ret=%d\n", enabled ? 1 : 0, ret);
        if (ret == 0) {
            schedule_power_state_retry();
        } else {
            post_status("Bluetooth power change failed");
        }
        return ret;
    }

    int scan()
    {
        int ret;

        if (local_device == NULL) {
            return -1;
        }
        ret = local_device->discovery(15);
        fprintf(stderr, "[bt-sdk] discovery ret=%d\n", ret);
        return ret;
    }

    int cancel_scan()
    {
        int ret;

        if (local_device == NULL) {
            return -1;
        }
        ret = local_device->cancelDiscovery();
        fprintf(stderr, "[bt-sdk] cancel discovery ret=%d\n", ret);
        return ret;
    }

    int pair(const char *address)
    {
        std::unique_ptr<BluetoothRemoteDevice> remote = make_remote(address);
        int ret;

        stop_scan();
        ret = remote != NULL ? remote->bond() : -1;
        fprintf(stderr, "[bt-sdk] bond %s ret=%d\n", address != NULL ? address : "", ret);
        return ret;
    }

    int remove(const char *address)
    {
        std::unique_ptr<BluetoothRemoteDevice> remote = make_remote(address);

        if (remote != NULL) {
            return remote->removeBond();
        }
        if (local_device != NULL && address != NULL) {
            BluetoothAddress bt_address(address);
            return local_device->deleteTrust(bt_address);
        }
        return -1;
    }

    int connect(const char *address)
    {
        std::unique_ptr<BluetoothRemoteDevice> remote = make_remote(address);

        stop_scan();
        return remote != NULL ? remote->connect() : -1;
    }

    int disconnect(const char *address)
    {
        std::unique_ptr<BluetoothRemoteDevice> remote = make_remote(address);

        return remote != NULL ? remote->disconnect() : -1;
    }

    void refreshMediaMetadata()
    {
        refresh_media_metadata();
    }

    void postCachedMediaMetadata()
    {
        post_cached_media_metadata();
    }

    void refreshBondedDevices()
    {
        sync_bonded_devices();
    }

private:
    template <typename T>
    void init_profile(const char *name, T **out)
    {
        IBluetoothProfile *profile = NULL;

        if (client != NULL && client->getProfile(name, &profile) >= 0) {
            *out = dynamic_cast<T *>(profile);
        }
    }

    void init_local_device()
    {
        if (local_device == NULL) {
            post_status("Bluetooth local device unavailable");
            return;
        }

        E_LOCAL_DEVICE_STATE power_state = GAP_STATE_INVALID;
        int state_ret = local_device->getBluetoothState(power_state);
        if (state_ret >= 0) {
            sdk_power_on = power_state == GAP_STATE_POWERON || power_state == GAP_STATE_DISCOVERYING;
            fprintf(stderr, "[bt-sdk] power state=%d\n", (int)power_state);
            post_bool_event(BT_EVENT_POWER_STATE,
                            sdk_power_on);
            if (!sdk_power_on &&
                (power_state == GAP_STATE_POWEROFF || power_state == GAP_STATE_INVALID)) {
                schedule_power_state_retry();
            }
        } else {
            fprintf(stderr, "[bt-sdk] getBluetoothState ret=%d\n", state_ret);
            schedule_power_state_retry();
        }
        {
            post_current_local_info();
        }
        (void)local_device->setScanMode(SCAN_MODE_PAGE_ON_INQUIRY_ON_LOW);
    }

    void schedule_power_state_retry()
    {
        pending_power_state_retry.store(true);
        next_power_state_retry_ms = 0U;
        power_state_retry_count = 0;
    }

    void refresh_power_state_if_due()
    {
        E_LOCAL_DEVICE_STATE state = GAP_STATE_INVALID;
        uint64_t now_ms;
        int state_ret;

        if (!pending_power_state_retry.load() || local_device == NULL) {
            return;
        }
        now_ms = platform_monotonic_ms();
        if (next_power_state_retry_ms == 0U) {
            next_power_state_retry_ms = now_ms + 250U;
            return;
        }
        if (now_ms < next_power_state_retry_ms) {
            return;
        }

        state_ret = local_device->getBluetoothState(state);
        if (state_ret >= 0 &&
            (state == GAP_STATE_POWERON || state == GAP_STATE_DISCOVERYING)) {
            fprintf(stderr, "[bt-sdk] power state retry=%d state=%d\n",
                    power_state_retry_count, (int)state);
            set_sdk_power_on(true);
            post_bool_event(BT_EVENT_POWER_STATE, true);
            pending_power_state_retry.store(false);
            next_power_state_retry_ms = 0U;
            power_state_retry_count = 0;
            return;
        }
        if (state_ret >= 0 && state == GAP_STATE_POWEROFF) {
            fprintf(stderr, "[bt-sdk] power state retry=%d state=%d\n",
                    power_state_retry_count, (int)state);
            post_bool_event(BT_EVENT_POWER_STATE, false);
            pending_power_state_retry.store(false);
            next_power_state_retry_ms = 0U;
            power_state_retry_count = 0;
            return;
        }

        ++power_state_retry_count;
        fprintf(stderr, "[bt-sdk] power state retry=%d state=%d ret=%d\n",
                power_state_retry_count, (int)state, state_ret);
        if (power_state_retry_count >= 15) {
            pending_power_state_retry.store(false);
            next_power_state_retry_ms = 0U;
            post_status("Bluetooth service not ready");
            return;
        }
        next_power_state_retry_ms = now_ms + 1000U;
    }

    void stop_scan()
    {
        E_LOCAL_DEVICE_STATE state = GAP_STATE_INVALID;

        if (local_device != NULL &&
            local_device->getBluetoothState(state) >= 0 &&
            state == GAP_STATE_DISCOVERYING) {
            (void)local_device->cancelDiscovery();
        }
    }

    std::unique_ptr<BluetoothRemoteDevice> make_remote(const char *address)
    {
        if (address == NULL || address[0] == '\0') {
            return std::unique_ptr<BluetoothRemoteDevice>();
        }
        BluetoothAddress bt_address(address);
        if (!bt_address.isValid()) {
            return std::unique_ptr<BluetoothRemoteDevice>();
        }
        return std::unique_ptr<BluetoothRemoteDevice>(new BluetoothRemoteDevice(bt_address));
    }

    void sync_bonded_devices()
    {
        std::vector<BluetoothRemoteDevice> devices;

        if (local_device == NULL || local_device->getBondedDevices(devices) < 0) {
            return;
        }

        for (size_t i = 0; i < devices.size(); ++i) {
            post_device_event(BT_EVENT_DEVICE_BONDED,
                              devices[i].getAddress().toString(),
                              devices[i].getName());
        }
    }

    void sync_profile_state()
    {
        sync_phone_state();
        sync_media_state();
    }

    void sync_phone_state()
    {
        E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;
        BluetoothAddress address;

        if (hfp == NULL || hfp->getState(state) < 0) {
            return;
        }

        if (state == BLUETOOTH_PROFILE_CONNECTED) {
            std::string text_address;
            if (hfp->getConnectedDevice(address) >= 0 && address.isValid()) {
                text_address = address.toString();
            }
            if (!last_phone_connected || last_phone_address != text_address) {
                post_bool_event(BT_EVENT_PHONE_CONNECTED, true, text_address);
            }
            last_phone_connected = true;
            last_phone_address = text_address;
        } else if (last_phone_connected) {
            post_bool_event(BT_EVENT_PHONE_DISCONNECTED, false, last_phone_address);
            last_phone_connected = false;
            last_phone_address.clear();
        }
    }

    void sync_media_state()
    {
        E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;
        BluetoothAddress address;
        bool media_connected = false;
        bool a2dp_connected = false;
        bool avrcp_connected = false;

        if (a2dp != NULL && a2dp->getState(state) >= 0 && state == BLUETOOTH_PROFILE_CONNECTED) {
            a2dp_connected = true;
            media_connected = true;
            (void)a2dp->getConnectedDevice(address);
        }
        if (!media_connected && avrcp != NULL && avrcp->getState(state) >= 0 && state == BLUETOOTH_PROFILE_CONNECTED) {
            media_connected = true;
            (void)avrcp->getConnectedDevice(address);
        }
        if (avrcp != NULL && avrcp->getState(state) >= 0 && state == BLUETOOTH_PROFILE_CONNECTED) {
            avrcp_connected = true;
        }

        if (a2dp_connected != last_a2dp_connected) {
            post_bool_event(a2dp_connected ? BT_EVENT_A2DP_CONNECTED : BT_EVENT_A2DP_DISCONNECTED,
                            a2dp_connected,
                            address.isValid() ? address.toString() : std::string());
            last_a2dp_connected = a2dp_connected;
        }
        if (avrcp_connected != last_avrcp_connected) {
            post_bool_event(avrcp_connected ? BT_EVENT_AVRCP_CONNECTED : BT_EVENT_AVRCP_DISCONNECTED,
                            avrcp_connected,
                            address.isValid() ? address.toString() : std::string());
            last_avrcp_connected = avrcp_connected;
        }

        if (media_connected) {
            std::string text_address = address.isValid() ? address.toString() : std::string();
            if (!last_media_connected || last_media_address != text_address) {
                post_bool_event(BT_EVENT_MEDIA_CONNECTED, true, text_address);
            }
            last_media_connected = true;
            last_media_address = text_address;
        } else if (last_media_connected) {
            post_bool_event(BT_EVENT_MEDIA_DISCONNECTED, false, last_media_address);
            last_media_connected = false;
            last_media_address.clear();
        }
    }

    void post_cached_media_metadata()
    {
        E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;
        std::string title;
        std::string artist;
        std::string album;

        if (avrcp == NULL) {
            return;
        }
        if (avrcp->getState(state) < 0 || state != BLUETOOTH_PROFILE_CONNECTED) {
            return;
        }
        if (avrcp->getMediaTitle(title) < 0 || title.empty()) {
            title = "Unknown";
        }
        if (avrcp->getMediaArtist(artist) < 0 || artist.empty()) {
            artist = "Unknown";
        }
        if (avrcp->getMediaAlbum(album) < 0 || album.empty()) {
            album = "Unknown";
        }
        post_media_info(title, artist, album);
    }

    void refresh_media_metadata()
    {
        E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;

        if (avrcp == NULL) {
            return;
        }
        if (avrcp->getState(state) < 0 || state != BLUETOOTH_PROFILE_CONNECTED) {
            return;
        }
        if (!last_music_playing) {
            return;
        }

        (void)avrcp->updatePlayStatus();
        (void)avrcp->updateMediaInfo();
        post_cached_media_metadata();
    }
};

BtSdkBackend *backend()
{
    return s_backend.get();
}

bool is_music_playing_from_backend()
{
    BtSdkBackend *sdk = backend();

    return sdk != NULL && sdk->last_music_playing;
}

void post_music_position(int position_ms, int duration_ms)
{
    BtSdkBackend *sdk = backend();
    BtEvent event;
    int position_sec;
    bool duration_changed;

    if (sdk == NULL || position_ms < 0) {
        return;
    }

    position_sec = position_ms / 1000;
    duration_changed = duration_ms >= 0 && sdk->last_reported_duration_ms != duration_ms;
    if (sdk->last_reported_position_sec == position_sec && !duration_changed) {
        return;
    }

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_MUSIC_POSITION;
    std::snprintf(event.media_position, sizeof(event.media_position), "%02d:%02d",
                  position_sec / 60, position_sec % 60);
    if (duration_ms >= 0) {
        std::snprintf(event.media_duration, sizeof(event.media_duration), "%02d:%02d",
                      (duration_ms / 1000) / 60, (duration_ms / 1000) % 60);
    }

    if (post_event(event)) {
        sdk->last_reported_position_sec = position_sec;
        if (duration_ms >= 0) {
            sdk->last_reported_duration_ms = duration_ms;
        }
    }
}

void update_music_playing_state(bool playing, const std::string &address)
{
    BtSdkBackend *sdk = backend();
    bool was_playing = false;
    bool changed = true;

    if (sdk != NULL) {
        was_playing = sdk->last_music_playing;
        changed = playing != was_playing;
        sdk->last_music_playing = playing;
        sdk->initial_play_status_pending = false;
    }

    if (!changed) {
        return;
    }

    post_bool_event(BT_EVENT_MUSIC_PLAYING, playing, address);

    if (playing && !was_playing) {
        refresh_media_metadata_from_backend();
    }
}

void request_initial_play_status_from_backend()
{
    BtSdkBackend *sdk = backend();

    if (sdk == NULL || sdk->avrcp == NULL) {
        return;
    }

    sdk->initial_play_status_pending = true;
    if (sdk->avrcp->updatePlayStatus() < 0) {
        sdk->initial_play_status_pending = false;
    }
}

void request_play_status_from_backend()
{
    BtSdkBackend *sdk = backend();
    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;

    if (sdk == NULL || sdk->avrcp == NULL) {
        return;
    }
    if (sdk->avrcp->getState(state) < 0 || state != BLUETOOTH_PROFILE_CONNECTED) {
        return;
    }

    (void)sdk->avrcp->updatePlayStatus();
}

void reset_music_playing_cache()
{
    BtSdkBackend *sdk = backend();

    if (sdk == NULL) {
        return;
    }

    sdk->last_music_playing = false;
    sdk->initial_play_status_pending = false;
    sdk->last_reported_position_sec = -1;
    sdk->last_reported_duration_ms = -1;
}

BtPbapDownloadType current_pbap_download_type()
{
    return backend() != NULL ?
           pbap_download_type_from_request(backend()->last_pbap_request) :
           BT_PBAP_DOWNLOAD_NONE;
}

void set_a2dp_audio_enabled(bool enabled)
{
    if (backend() != NULL && backend()->a2dp != NULL) {
        backend()->a2dp->enableAudio(enabled);
    }
}

void post_current_local_info()
{
    std::string pin;

    if (backend() == NULL || backend()->local_device == NULL) {
        return;
    }

    (void)backend()->local_device->getPinCode(pin);
    post_local_info(backend()->local_device->getName(), pin);
}

void sync_bonded_devices_from_backend()
{
    if (backend() != NULL) {
        backend()->refreshBondedDevices();
    }
}

void reset_profile_connection_cache()
{
    if (backend() == NULL) {
        return;
    }

    backend()->last_phone_connected = false;
    backend()->last_media_connected = false;
    backend()->last_a2dp_connected = false;
    backend()->last_avrcp_connected = false;
    backend()->last_music_playing = false;
    backend()->initial_play_status_pending = false;
    backend()->last_reported_position_sec = -1;
    backend()->last_reported_duration_ms = -1;
    backend()->last_phone_address.clear();
    backend()->last_media_address.clear();
}

void set_sdk_power_on(bool powered_on)
{
    if (backend() == NULL) {
        return;
    }

    backend()->sdk_power_on = powered_on;
    if (powered_on) {
        backend()->pending_power_on_sync.store(true);
    } else {
        reset_profile_connection_cache();
    }
}

void queue_user_confirm(const std::string &address)
{
    if (backend() == NULL || address.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(backend()->pending_confirm_mutex);
    backend()->pending_user_confirm_address = address;
}

BtAppCallState map_hfp_call_state(int state)
{
    switch (state) {
    case HF_CALLSTATE_INCOMING:
        return BT_APP_CALL_STATE_INCOMING;
    case HF_CALLSTATE_OUTGOING:
        return BT_APP_CALL_STATE_OUTGOING;
    case HF_CALLSTATE_ALERTING:
        return BT_APP_CALL_STATE_ALERTING;
    case HF_CALLSTATE_SPEAKING:
        return BT_APP_CALL_STATE_SPEAKING;
    case HF_CALLSTATE_HELD:
    case HF_CALLSTATE_HELD_BY_RESPONSE_AND_HOLD:
        return BT_APP_CALL_STATE_HELD;
    case HF_CALLSTATE_WAITING:
        return BT_APP_CALL_STATE_WAITING;
    case HF_CALLSTATE_IDLE:
    default:
        return BT_APP_CALL_STATE_IDLE;
    }
}

void sync_hfp_call_list()
{
    std::vector<BluetoothHfClientCall> calls;

    if (backend() == NULL || backend()->hfp == NULL || backend()->hfp->getCall(calls) < 0) {
        return;
    }

    if (calls.empty()) {
        post_call_state(BT_APP_CALL_STATE_IDLE);
        return;
    }

    bool has_multiparty = calls.size() > 1U;
    const BluetoothHfClientCall &call = calls.front();
    if (!call.getNumber().empty()) {
        post_call_number(call.getNumber());
    }
    post_call_state(has_multiparty ? BT_APP_CALL_STATE_MULTI : map_hfp_call_state(call.getState()));
}

std::string record_name(const PBRecord &record)
{
    std::string name = record.getFormattedName();

    if (name.empty()) {
        name = record.getFirstName();
    }
    if (name.empty()) {
        name = record.getGivenName();
    }
    if (name.empty()) {
        name = record.getNumber();
    }
    return name.empty() ? "Unknown" : name;
}

bool post_contact_record(const std::string &name, const std::string &number)
{
    BtEvent event;
    std::string text_number;

    if (number.empty()) {
        return false;
    }
    text_number = sanitize_display_text(number, "");
    if (text_number.empty()) {
        return false;
    }

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_PHONEBOOK_RECORD;
    copy_string(event.contact.number, sizeof(event.contact.number), text_number);
    copy_string(event.contact.name, sizeof(event.contact.name),
                sanitize_display_text(name.empty() ? number : name, event.contact.number));
    return post_event(event);
}

BtCallRecordType record_type_from_path(int path, const PBRecord &record)
{
    int type = record.getType();

    if (type == PBMGR_INCOMING_CALLS_HISTORY || path == PBMGR_INCOMING_CALLS_HISTORY) {
        return BT_CALL_RECORD_INCOMING;
    }
    if (type == PBMGR_OUTGOING_CALLS_HISTORY || path == PBMGR_OUTGOING_CALLS_HISTORY) {
        return BT_CALL_RECORD_OUTGOING;
    }
    if (type == PBMGR_MISSED_CALLS_HISTORY || path == PBMGR_MISSED_CALLS_HISTORY) {
        return BT_CALL_RECORD_MISSED;
    }
    return BT_CALL_RECORD_ALL;
}

bool post_phonebook_records(int path, int *remaining)
{
    int count = 0;
    int request_count;
    bool truncated = false;
    std::list<PBRecord> records;

    if (remaining == NULL) {
        return false;
    }
    if (backend() == NULL || backend()->pbap == NULL ||
        backend()->pbap->getRecordCount((E_PBType)path, count) < 0 || count <= 0) {
        return false;
    }
    if (*remaining <= 0) {
        return true;
    }

    request_count = count;
    if (request_count > *remaining) {
        request_count = *remaining;
        truncated = true;
    }

    if (backend()->pbap->getRecord((E_PBType)path, 0, request_count, records) < 0) {
        return truncated;
    }

    for (std::list<PBRecord>::const_iterator it = records.begin(); it != records.end(); ++it) {
        std::list<PBRecord::PBTel> tel_list;
        std::string name = record_name(*it);

        if (*remaining <= 0) {
            truncated = true;
            break;
        }
        if (it->getTelList(tel_list) >= 0 && !tel_list.empty()) {
            for (std::list<PBRecord::PBTel>::const_iterator tel = tel_list.begin(); tel != tel_list.end(); ++tel) {
                if (*remaining <= 0) {
                    truncated = true;
                    break;
                }
                if (post_contact_record(name, tel->m_telNumber)) {
                    (*remaining)--;
                }
            }
        } else {
            if (post_contact_record(name, it->getNumber())) {
                (*remaining)--;
            }
        }
    }

    return truncated;
}

bool post_call_records(int path, int *remaining)
{
    int count = 0;
    int request_count;
    bool truncated = false;
    std::list<PBRecord> records;
    BtEvent event;

    if (remaining == NULL) {
        return false;
    }
    if (backend() == NULL || backend()->pbap == NULL ||
        backend()->pbap->getRecordCount((E_PBType)path, count) < 0 || count <= 0) {
        return false;
    }
    if (*remaining <= 0) {
        return true;
    }

    request_count = count;
    if (request_count > *remaining) {
        request_count = *remaining;
        truncated = true;
    }

    if (backend()->pbap->getRecord((E_PBType)path, 0, request_count, records, PB_ORDERY_BY_TIME, PB_SORT_DESC) < 0) {
        return truncated;
    }

    for (std::list<PBRecord>::const_iterator it = records.begin(); it != records.end(); ++it) {
        if (*remaining <= 0) {
            truncated = true;
            break;
        }
        memset(&event, 0, sizeof(event));
        event.type = BT_EVENT_CALL_RECORD;
        copy_string(event.call_record.number, sizeof(event.call_record.number),
                    sanitize_display_text(it->getNumber(), ""));
        copy_string(event.call_record.name, sizeof(event.call_record.name),
                    sanitize_display_text(record_name(*it), event.call_record.number));
        copy_string(event.call_record.time, sizeof(event.call_record.time),
                    sanitize_display_text(it->getCallTime(), ""));
        event.call_record.type = record_type_from_path(path, *it);
        if (post_event(event)) {
            (*remaining)--;
        }
    }

    return truncated;
}

bool is_call_record_pbap_path(int path)
{
    if (path == PBMGR_UNKNOW) {
        return false;
    }
    return path == PBMGR_COMBINED_CALLED_HISTORY ||
           (path & PBMGR_INCOMING_CALLS_HISTORY) ||
           (path & PBMGR_OUTGOING_CALLS_HISTORY) ||
           (path & PBMGR_MISSED_CALLS_HISTORY);
}

bool is_phonebook_pbap_path(int path)
{
    if (path == PBMGR_UNKNOW) {
        return false;
    }
    return (path & PBMGR_PHONEBOOK) || (path & PBMGR_SIM_PHONEBOOK);
}

bool post_call_records_for_path(int path, int *remaining)
{
    bool truncated = false;

    if (path == PBMGR_COMBINED_CALLED_HISTORY) {
        return post_call_records(PBMGR_COMBINED_CALLED_HISTORY, remaining);
    }

    truncated = post_call_records(PBMGR_INCOMING_CALLS_HISTORY, remaining) || truncated;
    truncated = post_call_records(PBMGR_OUTGOING_CALLS_HISTORY, remaining) || truncated;
    truncated = post_call_records(PBMGR_MISSED_CALLS_HISTORY, remaining) || truncated;
    return truncated;
}

void refresh_pbap_records(int path)
{
    BtEvent event;
    int requested_path = PBMGR_UNKNOW;
    PbapRequestKind requested_kind = PBAP_REQUEST_NONE;
    int phonebook_remaining = (int)BT_PHONEBOOK_MAX_RECORDS;
    int call_record_remaining = (int)BT_CALL_RECORD_MAX_RECORDS;
    bool truncated = false;

    if (backend() != NULL) {
        requested_path = backend()->last_pbap_path;
        requested_kind = backend()->last_pbap_request;
    }

    if (path == PBMGR_UNKNOW) {
        path = requested_path;
    }

    if (requested_kind == PBAP_REQUEST_CALL_RECORDS ||
        (requested_kind == PBAP_REQUEST_NONE &&
         (is_call_record_pbap_path(requested_path) || is_call_record_pbap_path(path)))) {
        memset(&event, 0, sizeof(event));
        event.type = BT_EVENT_CALL_RECORDS_RESET;
        post_event(event);
        truncated = post_call_records_for_path(path, &call_record_remaining);
        memset(&event, 0, sizeof(event));
        event.type = BT_EVENT_CALL_RECORDS_COMPLETE;
        post_event(event);
        if (truncated) {
            post_status("Call records truncated to 200 records");
        }
        return;
    }

    if (requested_kind == PBAP_REQUEST_PHONEBOOK ||
        (requested_kind == PBAP_REQUEST_NONE &&
         (is_phonebook_pbap_path(requested_path) || is_phonebook_pbap_path(path)))) {
        memset(&event, 0, sizeof(event));
        event.type = BT_EVENT_PHONEBOOK_RESET;
        post_event(event);
        truncated = post_phonebook_records(PBMGR_PHONEBOOK, &phonebook_remaining);
        truncated = post_phonebook_records(PBMGR_SIM_PHONEBOOK, &phonebook_remaining) || truncated;
        memset(&event, 0, sizeof(event));
        event.type = BT_EVENT_PHONEBOOK_COMPLETE;
        post_event(event);
        if (truncated) {
            post_status("Phonebook truncated to 500 records");
        }
        return;
    }

    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_RECORDS_RESET;
    post_event(event);
    truncated = post_call_records_for_path(path, &call_record_remaining);
    memset(&event, 0, sizeof(event));
    event.type = BT_EVENT_CALL_RECORDS_COMPLETE;
    post_event(event);
    if (truncated) {
        post_status("Call records truncated to 200 records");
    }
}

void refresh_media_metadata_from_backend()
{
    if (backend() != NULL) {
        backend()->refreshMediaMetadata();
    }
}

void post_cached_media_metadata_from_backend()
{
    if (backend() != NULL) {
        backend()->postCachedMediaMetadata();
    }
}

} // namespace

extern "C" int bt_platform_init(void)
{
    int ret;

    fprintf(stderr, "[bt-sdk] platform init begin\n");
    s_backend.reset(new BtSdkBackend());
    ret = s_backend->init();
    fprintf(stderr, "[bt-sdk] platform init ret=%d\n", ret);
    if (ret != 0) {
        s_backend.reset();
    }
    return ret;
}

extern "C" void bt_platform_deinit(void)
{
    s_backend.reset();
}

extern "C" void bt_platform_poll(void)
{
    if (backend() != NULL) {
        backend()->poll();
    }
}

extern "C" int bt_platform_gap_set_power(bool enabled)
{
    return backend() != NULL ? backend()->set_power(enabled) : -1;
}

extern "C" int bt_platform_gap_start_scan(void)
{
    return backend() != NULL ? backend()->scan() : -1;
}

extern "C" int bt_platform_gap_stop_scan(void)
{
    return backend() != NULL ? backend()->cancel_scan() : -1;
}

extern "C" int bt_platform_gap_pair(const char *address)
{
    return backend() != NULL ? backend()->pair(address) : -1;
}

extern "C" int bt_platform_gap_remove(const char *address)
{
    return backend() != NULL ? backend()->remove(address) : -1;
}

extern "C" int bt_platform_hfp_connect(const char *address)
{
    return backend() != NULL ? backend()->connect(address) : -1;
}

extern "C" int bt_platform_hfp_disconnect(const char *address)
{
    return backend() != NULL ? backend()->disconnect(address) : -1;
}

extern "C" int bt_platform_hfp_dial(const char *number)
{
    std::string dial_number;
    int ret;

    if (backend() == NULL || backend()->hfp == NULL || number == NULL || number[0] == '\0') {
        return -1;
    }

    dial_number = number;
    ret = backend()->hfp->dialNumber(dial_number);
    if (ret == 0) {
        post_call_number(dial_number);
    }
    return ret;
}

extern "C" int bt_platform_hfp_redial(void)
{
    return backend() != NULL && backend()->hfp != NULL ? backend()->hfp->redial() : -1;
}

extern "C" int bt_platform_hfp_accept(void)
{
    return backend() != NULL && backend()->hfp != NULL ? backend()->hfp->acceptIncommingCall() : -1;
}

extern "C" int bt_platform_hfp_hangup(void)
{
    return backend() != NULL && backend()->hfp != NULL ? backend()->hfp->terminatePhoneCall() : -1;
}

extern "C" int bt_platform_hfp_hold(BtCallAction action)
{
    E_HF_CALL_ACTION sdk_action = HF_CALL_ACTION_CHLD_2;

    if (backend() == NULL || backend()->hfp == NULL) {
        return -1;
    }

    switch (action) {
    case BT_CALL_ACTION_SWITCH:
        sdk_action = HF_CALL_ACTION_CHLD_2;
        break;
    case BT_CALL_ACTION_MERGE:
        sdk_action = HF_CALL_ACTION_CHLD_3;
        break;
    case BT_CALL_ACTION_TERMINATE_AND_ACCEPT:
        sdk_action = HF_CALL_ACTION_CHLD_1;
        break;
    case BT_CALL_ACTION_HOLD:
    default:
        sdk_action = HF_CALL_ACTION_CHLD_2;
        break;
    }

    return backend()->hfp->callHold(sdk_action);
}

extern "C" int bt_platform_hfp_switch_audio(bool phone_audio)
{
    if (backend() == NULL || backend()->hfp == NULL) {
        return -1;
    }
    return backend()->hfp->swithAudioTransferTowards(phone_audio ? HF_AUDIOTOWARDS_AG : HF_AUDIOTOWARDS_HF);
}

extern "C" int bt_platform_hfp_play_ringtone(void)
{
    return backend() != NULL && backend()->hfp != NULL ? backend()->hfp->playRingTone() : -1;
}

extern "C" int bt_platform_hfp_stop_ringtone(void)
{
    return backend() != NULL && backend()->hfp != NULL ? backend()->hfp->stopRingTone() : -1;
}

extern "C" int bt_platform_hfp_send_dtmf(const char *code)
{
    if (backend() == NULL || backend()->hfp == NULL || code == NULL) {
        return -1;
    }
    return backend()->hfp->sendDTMFCode(std::string(code));
}

extern "C" int bt_platform_pbap_download_phonebook(void)
{
    if (backend() == NULL || backend()->pbap == NULL) {
        return -1;
    }

    backend()->last_pbap_path = PBMGR_PHONEBOOK | PBMGR_SIM_PHONEBOOK;
    backend()->last_pbap_request = PBAP_REQUEST_PHONEBOOK;
    return backend()->pbap->download((E_PBType)(PBMGR_PHONEBOOK | PBMGR_SIM_PHONEBOOK));
}

extern "C" int bt_platform_pbap_download_call_records(void)
{
    int history_path;

    if (backend() == NULL || backend()->pbap == NULL) {
        return -1;
    }

    history_path = PBMGR_INCOMING_CALLS_HISTORY |
                   PBMGR_OUTGOING_CALLS_HISTORY |
                   PBMGR_MISSED_CALLS_HISTORY;
    backend()->last_pbap_path = history_path;
    backend()->last_pbap_request = PBAP_REQUEST_CALL_RECORDS;
    return backend()->pbap->download((E_PBType)history_path);
}

extern "C" int bt_platform_pbap_abort(void)
{
    return backend() != NULL && backend()->pbap != NULL ? backend()->pbap->stopDownload() : -1;
}

extern "C" int bt_platform_avrcp_play_pause(bool currently_playing)
{
    if (backend() == NULL || backend()->avrcp == NULL) {
        return -1;
    }
    return currently_playing ? backend()->avrcp->pause() : backend()->avrcp->play();
}

extern "C" int bt_platform_avrcp_pause(void)
{
    if (backend() == NULL || backend()->avrcp == NULL) {
        return -1;
    }
    return backend()->avrcp->pause();
}

extern "C" int bt_platform_avrcp_prev(void)
{
    return backend() != NULL && backend()->avrcp != NULL ? backend()->avrcp->prev() : -1;
}

extern "C" int bt_platform_avrcp_next(void)
{
    return backend() != NULL && backend()->avrcp != NULL ? backend()->avrcp->next() : -1;
}

extern "C" int bt_platform_a2dp_init(void)
{
    return backend() != NULL && backend()->a2dp != NULL ? 0 : -1;
}

extern "C" int bt_platform_a2dp_set_audio_enabled(bool enabled)
{
    set_a2dp_audio_enabled(enabled);
    return backend() != NULL && backend()->a2dp != NULL ? 0 : -1;
}

extern "C" int bt_platform_set_local_name(const char *name)
{
    std::string safe_name;

    if (backend() == NULL || backend()->local_device == NULL ||
        !copy_local_name_for_sdk(name, safe_name)) {
        return -1;
    }
    int ret = backend()->local_device->setName(safe_name);
    if (ret == 0) {
        std::string pin;
        (void)backend()->local_device->getPinCode(pin);
        post_local_info(std::string(name), pin);
    }
    return ret;
}

extern "C" int bt_platform_set_pairing_pin(const char *pin)
{
    if (backend() == NULL || backend()->local_device == NULL || pin == NULL || pin[0] == '\0') {
        return -1;
    }
    int ret = backend()->local_device->setPinCode(std::string(pin));
    if (ret == 0) {
        post_local_info(backend()->local_device->getName(), std::string(pin));
    }
    return ret;
}
