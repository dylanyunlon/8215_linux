#ifndef BT_EVENT_QUEUE_H
#define BT_EVENT_QUEUE_H

#include "bt_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void bt_event_queue_reset(void);
bool bt_event_post(const BtEvent *event);
bool bt_event_pop(BtEvent *event);
size_t bt_event_queue_dropped_count(void);

#ifdef __cplusplus
}
#endif

#endif
