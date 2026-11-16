#ifndef CP_EVENT_QUEUE_H
#define CP_EVENT_QUEUE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CP_EVENT_DEVICE_ID_MAX_LEN 128
#define CP_EVENT_DEVICE_NAME_MAX_LEN 256

typedef enum CpEventType {
    CP_EVENT_NONE = 0,
    CP_EVENT_DEVICE_BOND_STATE_CHANGED,
    CP_EVENT_CONNECTION_STATE_CHANGED,
    CP_EVENT_STREAM_STATE_CHANGED,
    CP_EVENT_ENABLE_STATE_CHANGED,
    CP_EVENT_AVAILABLE_DEVICE_ADDED,
    CP_EVENT_FIRST_CONNECT_REQUEST,
    CP_EVENT_PASSIVE_CONNECT_REQUEST,
    CP_EVENT_REQUEST_ACCESSORY_UI,
    CP_EVENT_CALLBACK_REGISTERED,
    CP_EVENT_SERVICE_DIED,
    CP_EVENT_BLUETOOTH_SCAN_MODE_CHANGED,
    CP_EVENT_CONNECTION_FAIL_REASON,
    CP_EVENT_DEVICE_NAME_UPDATE,
} CpEventType;

typedef struct CpEvent {
    CpEventType type;
    char device_id[CP_EVENT_DEVICE_ID_MAX_LEN];
    char device_name[CP_EVENT_DEVICE_NAME_MAX_LEN];
    int value;
    int value2;
    int value3;
} CpEvent;

void cp_event_queue_reset(void);
bool cp_event_post(const CpEvent *event);
bool cp_event_pop(CpEvent *event);
size_t cp_event_queue_dropped_count(void);

#ifdef __cplusplus
}
#endif

#endif
