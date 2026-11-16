#include "carplayapp_if.h"
#include "carplayapp.h"
#include "carplaysetting_ui.h"
#include "carplayapp_ui.h"
#include "cp_event_queue.h"

#include "app_common.h"
#include "videofocusmanager.h"

#include "accessorycall.h"
#include "carplaycommon.h"
#include "iaccessorycommon.h"

#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

using vendor::autochips::carlink::AccessoryCall;
using vendor::autochips::carplay::CarplayTransport;
using vendor::autochips::carplay::CarplayDeviceURLIdentifier;
using vendor::autochips::carplay::CarplayScreenIndex;
using vendor::autochips::carplay::CarplayEnableState;
using vendor::autochips::carplay::CarplayBondState;
using vendor::autochips::carplay::CarplayCall;
using vendor::autochips::carplay::CarplayConnectionState;
using vendor::autochips::carplay::CarplayConnectFailReason;
using vendor::autochips::carplay::CarplayDisconnectReason;
using vendor::autochips::carplay::CarplayStreamState;

static pthread_mutex_t s_video_stop_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_video_stop_cond = PTHREAD_COND_INITIALIZER;
static bool s_video_stopped = true;

/* 方案 A: RVC 借屏期间 CP 不出栈，仅释放 surface。
 * - STREAM_STATE_STOPPED 时若 RVC 在栈中，置 true，跳过 close_app
 * - STREAM_STATE_STARTED 时若为 true，清标记并 return（CP 仍是栈顶）
 * - CP 真正退出路径（断连/服务死/OEM UI 请求）必须清标记 */
static bool s_cp_paused_by_rvc = false;

void carplayapp_init_screen_value(CarplayApp* app)
{
    app->screen_width = 1024;
    app->screen_height = 600;
    lv_log("screen width[%d]  screen height[%d]", app->screen_width, app->screen_height);
}

void carplayapp_init_client(CarplayApp* app)
{
    vendor::autochips::carplay::CarplayStatus ret;

    fprintf(stderr, "[cp] init begin\n");
    app->accessory_info = vendor::autochips::carlink::getAccessoryInfoClient();
    fprintf(stderr, "[cp] accessory_info=%p\n", (void *)app->accessory_info);
    if (app->accessory_info == NULL) {
        lv_log("failed to get IAccessoryInfo");
        return;
    }

    /* CarPlay depends on the local Bluetooth identity burned in metazone.
     * An unprovisioned metazone returns an empty device id, and the vendor
     * CarPlay client does not tolerate that during initialization. */
    const std::string device_id = app->accessory_info->getDeviceID();
    fprintf(stderr, "[cp] device_id length=%u\n", (unsigned int)device_id.size());
    if (device_id.empty()) {
        lv_log("carplay disabled: metazone BT MAC is not provisioned, restore factory metazone.bin");
        return;
    }

    fprintf(stderr, "[cp] getCarplayClient begin\n");
    app->carplay_client = vendor::autochips::carplay::service::getCarplayClient();
    fprintf(stderr, "[cp] carplay_client=%p\n", (void *)app->carplay_client);
    if (app->carplay_client == NULL) {
        lv_log("failed to get ICarplayClient");
    } else {
        fprintf(stderr, "[cp] getCarplayManager begin\n");
        ret = app->carplay_client->getCarplayManager(&app->carplay_manager);
        fprintf(stderr, "[cp] carplay_manager=%p ret=%d\n", (void *)app->carplay_manager, (int)ret);
        if (ret != CarplayStatus::CARPLAY_OK || app->carplay_manager == NULL) {
            lv_log("failed to get ICarplayManager");
        }

        int type = 0;
        type |= (int)AccessoryFunctionType::FUNCTION_TYPE_WIFI;
        type |= (int)AccessoryFunctionType::FUNCTION_TYPE_BT_GAP;
        type |= (int)AccessoryFunctionType::FUNCTION_TYPE_BT_AVRCP;
        type |= (int)AccessoryFunctionType::FUNCTION_TYPE_BT_HFP;
        type |= (int)AccessoryFunctionType::FUNCTION_TYPE_BT_A2DP;

        app->accessory_callback = new AccessoryInfoCallback(app);
        fprintf(stderr, "[cp] accessory_callback=%p\n", (void *)app->accessory_callback);
        if (app->accessory_callback == NULL) {
            lv_log("failed to new AccessoryInfoCallback");
        } else {
            ((AccessoryInfoCallback*)app->accessory_callback)->setExpectedEnableType(type);
            fprintf(stderr, "[cp] enableAsync begin\n");
            app->accessory_info->enableAsync(type, *(AccessoryInfoCallback*)app->accessory_callback);
            fprintf(stderr, "[cp] enableAsync done\n");
            /* registerCallbackAsync will be invoked in onAccessoryReady
             * once all enable bits of `type` have reported ACCESSORY_OK. */

            app->carplay_callback = new CarplayCallback(app);
            fprintf(stderr, "[cp] carplay_callback=%p\n", (void *)app->carplay_callback);
            if (app->carplay_callback == NULL) {
                lv_log("failed to new CarplayCallback");
            } else {
                fprintf(stderr, "[cp] registerCallback begin\n");
                app->carplay_client->registerCallback(*(CarplayCallback*)app->carplay_callback);
                fprintf(stderr, "[cp] registerCallback done\n");
            }
        }
    }
    fprintf(stderr, "[cp] init done\n");
}

CarplayApp* carplayapp_get_instance()
{
    static CarplayApp* instance = NULL;
    static pthread_mutex_t instance_mtx = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&instance_mtx);
    if (instance == NULL) {
        instance = (CarplayApp*)malloc(sizeof(CarplayApp));
        if (instance != NULL) {
            memset(instance, 0, sizeof(CarplayApp));
            /* malloc/memset 不会构造 C++ 对象，用 placement new 在堆上构造 */
            instance->connecting_mtx = new std::mutex();
            instance->connecting_devices = new std::set<std::string>();
        }
        carplayapp_init_screen_value(instance);
        carplayapp_init_client(instance);
    }
    pthread_mutex_unlock(&instance_mtx);

    return instance;
}

void carplayapp_destroy(CarplayApp* app)
{
    if (app == NULL) return;

    if (app->carplay_client != NULL) {
        vendor::autochips::carplay::service::releaseCarplayClient();
        app->carplay_client = NULL;
    }

    if (app->accessory_info != NULL) {
        vendor::autochips::carlink::releaseAccessoryInfoClient();
        app->accessory_info = NULL;
    }

    if (app->carplay_callback != NULL) {
        delete (CarplayCallback*)app->carplay_callback;
        app->carplay_callback = NULL;
    }

    if (app->accessory_callback != NULL) {
        delete (AccessoryInfoCallback*)app->accessory_callback;
        app->accessory_callback = NULL;
    }

    if (app->surface != NULL) {
        videofocusmanager_close_app(APP_CARPLAY);
        lv_log("IAtcSurface_hide");
        int err = IAtcSurface_hide(app->surface);
        if (err < 0) {
            lv_log("IAtcSurface_hide error: %d", err);
        }
        lv_log("IAtcSurface_release");
        IAtcSurface_release(app->surface);
        app->surface = NULL;
    }

    delete app->connecting_mtx;
    app->connecting_mtx = NULL;
    delete app->connecting_devices;
    app->connecting_devices = NULL;

    free(app);
}

void carplayapp_start_video(CarplayApp* app)
{
    std::vector<CarplayDevice> carplay_devices;

    if (app == NULL || app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
        return;
    }

    app->carplay_manager->getConnectedDevices(carplay_devices);
    if (carplay_devices.empty()) {
        lv_log("%s no connected CarPlay device", __func__);
        return;
    }

    if (app->surface == NULL) {
        app->surface = atc_createsurface(ATCSURF_TYPE_DEFAULT, app->screen_width, app->screen_height, ATC_PIX_FMT_NV12M_PRIVATE1);
        if (app->surface != NULL) {
            lv_log("carplayapp_start_video: surface=%p created, startVideo", app->surface);
            IAtcSurface_setWindow(app->surface, 0, 0, app->screen_width, app->screen_height);
            IAtcSurface_setLayerZOrder(app->surface, 3);
            IAtcSurface_show(app->surface);
            app->carplay_manager->startVideo(carplay_devices[0].getDeviceId(), CarplayScreenIndex::SCREEN_INDEX_MAIN, (void *)app->surface);
        } else {
            lv_log("carplayapp_start_video: failed to create surface");
        }
    } else {
        lv_log("carplayapp_start_video: surface=%p already exists, skip create", app->surface);
    }
}

bool carplayapp_is_show_limit(CarplayApp* app)
{
    bool ret = false;
    IAccessoryInfo* info;

    if (app == NULL || app->accessory_info == NULL) {
        lv_log("%s accessory_info is NULL", __func__);
        return false;
    }

    info = app->accessory_info;

    if (info->isRvcOn()) {
        lv_log("in rvc state");
        ret = true;
    } else {
        std::vector<AccessoryCall> accessorycalls = info->getCurrentCalls();
        size_t i;
        int found_call = 0;
        for (i = 0; i < accessorycalls.size(); i++) {
            if (accessorycalls[i].getCallState() == AccessoryCallState::ACCESSORY_CALL_STATE_OFFHOOK ||
                accessorycalls[i].getCallState() == AccessoryCallState::ACCESSORY_CALL_STATE_RINGING) {
                found_call = 1;
                break;
            }
        }

        if (found_call) {
            lv_log("in call state");
            ret = true;
        }
    }

    return ret;
}

void carplayapp_request_ui(CarplayApp* app)
{
    if (app == NULL || app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
    } else {
        std::vector<CarplayDevice> carplay_devices;
        app->carplay_manager->getConnectedDevices(carplay_devices);
        if (carplay_devices.size() > 0) {
            app->carplay_manager->requestDeviceUI(carplay_devices[0].getDeviceId(), CarplayDeviceURLIdentifier::URL_ID_EMPTY, "");
        }
    }
}

void carplayapp_on_ui_touched(CarplayApp* app, int press, int point_x, int point_y)
{
    std::vector<CarplayDevice> carplay_devices;
    if (app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
        return;
    }

    app->carplay_manager->getConnectedDevices(carplay_devices);
    if (carplay_devices.size() > 0) {
        app->carplay_manager->updateTouchScreen(carplay_devices[0].getDeviceId(), (bool)press, point_x, point_y);
    }
}

bool carplayapp_is_connected(CarplayApp* app)
{
    bool result = false;
    std::vector<CarplayDevice> carplay_devices;
    if (app == NULL || app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
    } else {
        app->carplay_manager->getConnectedDevices(carplay_devices);
        if (carplay_devices.size() > 0) {
            result = true;
        }
    }

    return result;
}

bool carplayapp_has_connected_call(CarplayApp* app)
{
    std::vector<CarplayDevice> carplay_devices;

    if (app == NULL || app->carplay_manager == NULL) {
        return false;
    }

    app->carplay_manager->getConnectedDevices(carplay_devices);
    for (auto& device : carplay_devices) {
        std::vector<CarplayCall> calls;
        CarplayStatus status =
            app->carplay_manager->getCurrentCalls(device.getDeviceId(), calls);
        if (status != CarplayStatus::CARPLAY_OK) {
            continue;
        }

        for (const auto& call : calls) {
            if (call.mStatus == CarplayCall::CALL_STATUS_ACTIVE ||
                call.mStatus == CarplayCall::CALL_STATUS_HELD) {
                lv_log("connected CarPlay call found, status[%d]", call.mStatus);
                return true;
            }
        }
    }

    return false;
}

bool carplayapp_id_is_connected(CarplayApp* app, const char *id)
{
    if (app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
        return false;
    } else {
        CarplayDevice device(id);
        return device.isConnected();
    }
}

bool carplayapp_id_is_connecting(CarplayApp* app, const char *id)
{
    if (app == NULL || id == NULL || app->connecting_mtx == NULL || app->connecting_devices == NULL) {
        return false;
    }
    std::lock_guard<std::mutex> lk(*app->connecting_mtx);
    return app->connecting_devices->count(std::string(id)) > 0;
}

void carplayapp_enable(CarplayApp* app)
{
    if (app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
    } else {
        app->carplay_manager->enable();
    }
}

void carplayapp_disable(CarplayApp* app)
{
    if (app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
    } else {
        app->carplay_manager->disable();
    }
}

void carplayapp_enable_discoverable(CarplayApp* app)
{
    if (app->accessory_info == NULL) {
        lv_log("%s accessory_info is NULL", __func__);
    } else {
        AccessoryBluetoothScanMode mode = AccessoryBluetoothScanMode::ACCESSORY_SCAN_MODE_CONNECTABLE_DISCOVERABLE;
        app->accessory_info->setScanMode(mode);
    }
}

void carplayapp_disable_discoverable(CarplayApp* app)
{
    if (app->accessory_info == NULL) {
        lv_log("%s accessory_info is NULL", __func__);
    } else {
        AccessoryBluetoothScanMode mode = AccessoryBluetoothScanMode::ACCESSORY_SCAN_MODE_NONE;
        app->accessory_info->setScanMode(mode);
    }
}

void carplayapp_fresh(CarplayApp* app)
{
    if (app->accessory_info == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
    } else {
        app->accessory_info->cancelDiscovery();
        app->accessory_info->startDiscovery();
    }
}

void carplayapp_connect(CarplayApp* app, const char* device_id)
{
    if (app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
        return;
    }
    
    if (carplayapp_id_is_connecting(app, device_id)) {
        lv_log("%s device %s is already connecting", __func__, device_id);
        return;
    }
    if (carplayapp_id_is_connected(app, device_id)) {
        lv_log("%s device %s is already connected", __func__, device_id);
        return;
    }
    {
        std::lock_guard<std::mutex> lk(*app->connecting_mtx);
        app->connecting_devices->insert(std::string(device_id));
    }
    app->carplay_manager->connect(device_id, CarplayTransport::TRANSPORT_NONE);
}

void carplayapp_disconnect(CarplayApp* app, const char* device_id)
{
    if (app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
    } else {
        app->carplay_manager->disconnect(device_id);
    }
}

void carplayapp_forget(CarplayApp* app, const char* device_id)
{
    if (app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
    } else {
        app->carplay_manager->forget(device_id);
    }
}

void carplayapp_confirmFirstConnect(CarplayApp* app, const char* device_id)
{
    if (app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
    } else {
        app->carplay_manager->confirmFirstConnect(device_id);
    }
}

void carplayapp_stop_video(CarplayApp* app)
{
    if (app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
    } else {
        std::vector<CarplayDevice> carplay_devices;
        app->carplay_manager->getConnectedDevices(carplay_devices);
        if (carplay_devices.size() > 0) {
            app->carplay_manager->stopVideo(carplay_devices[0].getDeviceId(), CarplayScreenIndex::SCREEN_INDEX_MAIN, true);
        }
    }
}

bool carplayapp_is_enabled(CarplayApp* app)
{
    bool result = false;
    if (app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
    } else {
        CarplayEnableState state = CarplayEnableState::STATE_OFF;
        app->carplay_manager->getEnableState(state);
        if (state == CarplayEnableState::STATE_ON) {
            result = true;
        }
    }

    return result;
}
bool carplayapp_is_discoverable(CarplayApp* app)
{
    bool result = false;
    if (app->accessory_info == NULL) {
        lv_log("%s accessory_info is NULL", __func__);
    } else {
        AccessoryBluetoothScanMode mode = AccessoryBluetoothScanMode::ACCESSORY_SCAN_MODE_NONE;
        app->accessory_info->getScanMode(mode);
        if (mode == AccessoryBluetoothScanMode::ACCESSORY_SCAN_MODE_CONNECTABLE_DISCOVERABLE) {
            result = true;
        }
    }

    return result;
}


void carplayapp_update_list(CarplayApp* app)
{
    if (app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
    } else {
        std::vector<CarplayDevice> carplay_devices;
        app->carplay_manager->getBondedDevices(carplay_devices);
        for (auto& device : carplay_devices) {
            carplaysetting_add_bonded_device(device.getName().c_str(), device.getDeviceId().c_str());
        }
    }
}

/* Release the CarPlay surface. Must be called only from the UI thread
 * (carplayapp_poll -> handle_cp_event) to avoid races with
 * carplayapp_start_video / carplayapp_destroy which also run on the UI thread. */
void carplayapp_release_surface(CarplayApp *app)
{
    if (app == NULL) {
        return;
    }

    if (app->surface != NULL) {
        lv_log("carplayapp_release_surface: release surface=%p", app->surface);
        IAtcSurface_setLayerZOrder(app->surface, 0);
        lv_log("IAtcSurface_hide");
        int err = IAtcSurface_hide(app->surface);
        if (err < 0) {
            lv_log("IAtcSurface_hide error: %d", err);
        }
        lv_log("IAtcSurface_release");
        IAtcSurface_release(app->surface);
        app->surface = NULL;
        lv_log("carplayapp_release_surface: surface released, app->surface=NULL");
    } else {
        lv_log("carplayapp_release_surface: surface already NULL");
    }
}

static void handle_cp_event(CarplayApp *app, CpEvent *event)
{
    switch (event->type) {
    case CP_EVENT_DEVICE_BOND_STATE_CHANGED:
        if (event->value == (int)CarplayBondState::BOND_BONDED) {
            carplaysetting_remove_available_device(event->device_id);
            carplaysetting_add_bonded_device(event->device_name, event->device_id);
            carplaysetting_update_bonded_device_status(event->device_id, 1);
        } else if (event->value == (int)CarplayBondState::BOND_NONE) {
            carplaysetting_remove_bonded_device(event->device_id);
        }
        break;
    case CP_EVENT_CONNECTION_STATE_CHANGED:
        carplaysetting_update_bonded_device_status(event->device_id, event->value);
        if (event->value == (int)CarplayConnectionState::CONNECTION_STATE_DISCONNECTED) {
            if (!carplayapp_is_connected(app)) {
                s_cp_paused_by_rvc = false;
                videofocusmanager_close_app(APP_CARPLAY);
                carplayapp_release_surface(app);
            }
            carplaysetting_update_available_device_status(event->device_id);
        }
        break;
    case CP_EVENT_STREAM_STATE_CHANGED:
        if (event->value2 == (int)CarplayStreamState::STREAM_STATE_STARTED) {
            if (s_cp_paused_by_rvc) {
                /* 方案 A: RVC 借屏期间 CP 未出栈，stream 恢复时 CP 已是栈顶，
                 * 无需重复 open_app。RVC 退出时由栈恢复自动回到 CP 屏幕。 */
                lv_log("%s CP paused by RVC, skip open_app", __func__);
                s_cp_paused_by_rvc = false;
            } else {
                lv_log("%s videofocusmanager_open_app(APP_CARPLAY)", __func__);
                videofocusmanager_open_app(APP_CARPLAY);
            }
        } else if (event->value2 == (int)CarplayStreamState::STREAM_STATE_STOPPED) {
            if (videofocusmanager_is_in_stack(APP_RVC)) {
                /* 方案 A: RVC 在栈中（CP 在栈底），CP 不出栈，仅释放 surface。
                 * RVC 退出后栈顶自然恢复 CP，再由 STARTED 事件或 focus 回调重建 surface。 */
                lv_log("%s RVC in stack, pause CP instead of close", __func__);
                s_cp_paused_by_rvc = true;
                carplayapp_release_surface(app);
            } else {
                videofocusmanager_close_app(APP_CARPLAY);
                carplayapp_release_surface(app);
            }
        }
        break;
    case CP_EVENT_ENABLE_STATE_CHANGED:
        if (event->value == (int)CarplayEnableState::STATE_ON) {
            carplaysetting_switch_on_off(1);
        } else if (event->value == (int)CarplayEnableState::STATE_OFF) {
            carplaysetting_switch_on_off(0);
        }
        break;
    case CP_EVENT_AVAILABLE_DEVICE_ADDED:
        carplaysetting_add_available_device(event->device_name, event->device_id);
        break;
    case CP_EVENT_FIRST_CONNECT_REQUEST:
        if (carplayapp_is_connected(app)) {
            carplayapp_show_passive_connect_popup(event->device_name, event->device_id);
        } else {
            carplayapp_show_first_connect_popup(event->device_name, event->device_id);
        }
        break;
    case CP_EVENT_PASSIVE_CONNECT_REQUEST:
        carplayapp_show_passive_connect_popup(event->device_name, event->device_id);
        break;
    case CP_EVENT_REQUEST_ACCESSORY_UI:
        s_cp_paused_by_rvc = false;
        videofocusmanager_close_app(APP_CARPLAY);
        break;
    case CP_EVENT_CALLBACK_REGISTERED:
        videofocusmanager_open_app(APP_CARPLAY);
        break;
    case CP_EVENT_SERVICE_DIED:
        s_cp_paused_by_rvc = false;
        videofocusmanager_close_app(APP_CARPLAY);
        carplayapp_release_surface(app);
        break;
    case CP_EVENT_BLUETOOTH_SCAN_MODE_CHANGED:
        if (event->value == (int)AccessoryBluetoothScanMode::ACCESSORY_SCAN_MODE_CONNECTABLE_DISCOVERABLE) {
            carplaysetting_discoverable_on_of(1);
        } else {
            carplaysetting_discoverable_on_of(0);
        }
        break;
    case CP_EVENT_CONNECTION_FAIL_REASON:
        {
            const char *reason_text = "Unknown error";
            switch (event->value) {
            case (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_USB:
                reason_text = "CONNECT_FAIL_REASON_USB"; break;
            case (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_BT:
                reason_text = "CONNECT_FAIL_REASON_BT"; break;
            case (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_IAP:
                reason_text = "CONNECT_FAIL_REASON_IAP"; break;
            case (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_SESSION:
                reason_text = "CONNECT_FAIL_REASON_SESSION"; break;
            case (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_BT_DISABLED:
                reason_text = "CONNECT_FAIL_REASON_BT_DISABLED"; break;
            case (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_AP_DISABLED:
                reason_text = "CONNECT_FAIL_REASON_AP_DISABLED"; break;
            case (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_AP_AND_BT_DISABLED:
                reason_text = "CONNECT_FAIL_REASON_AP_AND_BT_DISABLED"; break;
            case (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_WIFI_INFORMATION_INVALIND:
                reason_text = "CONNECT_FAIL_REASON_WIFI_INFORMATION_INVALIND"; break;
            case (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_DEVICE_REJECT:
                reason_text = "CONNECT_FAIL_REASON_DEVICE_REJECT"; break;
            case (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_CARPLAY_DISABLED:
                reason_text = "CONNECT_FAIL_REASON_CARPLAY_DISABLED"; break;
            case (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_BT_STOPPED:
                reason_text = "CONNECT_FAIL_REASON_BT_STOPPED"; break;
            case (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_AP_SECURITY_TYPE_ERROR:
                reason_text = "CONNECT_FAIL_REASON_AP_SECURITY_TYPE_ERROR"; break;
            case (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_ILLEGAL_SESSION:
                reason_text = "CONNECT_FAIL_REASON_ILLEGAL_SESSION"; break;
            case (int)CarplayDisconnectReason::DISCONNECT_REASON_NET_IFACE_CLIENT_DISCONNECTED:
                reason_text = "DISCONNECT_REASON_NET_IFACE_CLIENT_DISCONNECTED"; break;
            default: break;
            }
            carplayapp_show_connect_fail_popup(reason_text);
        }
        break;
    case CP_EVENT_DEVICE_NAME_UPDATE:
        carplaysetting_update_bonded_device_name(event->device_id, event->device_name);
        break;
    default:
        break;
    }
}

void carplayapp_poll(void)
{
    CarplayApp *app = carplayapp_get_instance();
    if (app == NULL) {
        return;
    }

    static size_t s_last_dropped_count = 0;
    size_t dropped_count = cp_event_queue_dropped_count();
    if (dropped_count != s_last_dropped_count) {
        printf("[CP_APP][W] dropped events: %u\r\n", (unsigned int)dropped_count);
        s_last_dropped_count = dropped_count;
    }

    CpEvent event;
    int handled = 0;

    while (handled < 32 && cp_event_pop(&event)) {
        handle_cp_event(app, &event);
        handled++;
    }
}

void carplayapp_wait_for_video_stopped(CarplayApp* app)
{
    if (app == NULL || app->carplay_manager == NULL) {
        lv_log("%s carplay_manager is NULL", __func__);
        return;
    }

    std::vector<CarplayDevice> carplay_devices;
    app->carplay_manager->getConnectedDevices(carplay_devices);
    if (carplay_devices.size() > 0) {
        CarplayStreamState stream_state = CarplayStreamState::STREAM_STATE_STOPPED;
        app->carplay_manager->getStreamState(carplay_devices[0].getDeviceId(),
                    CarplayStreamID::STREAM_ID_MAIN_SCREEN,
                    CarplayAudioType::AUDIO_TYPE_DEFAULT, stream_state);
        if (stream_state == CarplayStreamState::STREAM_STATE_STARTED) {
            // condition to wait for video stopped
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 1;
            pthread_mutex_lock(&s_video_stop_mtx);
            int rc = 0;
            while (!s_video_stopped && rc != ETIMEDOUT) {
                rc = pthread_cond_timedwait(&s_video_stop_cond, &s_video_stop_mtx, &ts);
            }
            if (s_video_stopped) {
                lv_log("%s video stopped signaled", __func__);
            } else {
                lv_log("%s video stop wait timed out", __func__);
            }
            pthread_mutex_unlock(&s_video_stop_mtx);
        } else {
            lv_log("%s video is stopped", __func__);
        }
    } else {
        lv_log("%s carplay is not connected", __func__);
    }
}

void carplayapp_signal_video_stopped(void)
{
    pthread_mutex_lock(&s_video_stop_mtx);
    s_video_stopped = true;
    pthread_cond_signal(&s_video_stop_cond);
    pthread_mutex_unlock(&s_video_stop_mtx);
}

void carplayapp_reset_video_stopped(void)
{
    pthread_mutex_lock(&s_video_stop_mtx);
    s_video_stopped = false;
    pthread_mutex_unlock(&s_video_stop_mtx);
}
