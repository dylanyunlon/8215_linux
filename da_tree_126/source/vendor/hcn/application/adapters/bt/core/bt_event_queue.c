#include "bt_event_queue.h"

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BT_EVENT_QUEUE_INITIAL_CAPACITY 256U
#define BT_EVENT_QUEUE_MAX_CAPACITY 16384U

static pthread_mutex_t s_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static BtEvent *s_queue = NULL;
static size_t s_capacity = 0;
static size_t s_head = 0;
static size_t s_tail = 0;
static size_t s_count = 0;
static size_t s_dropped_count = 0;

static void free_queue_locked(void)
{
    free(s_queue);
    s_queue = NULL;
    s_capacity = 0;
    s_head = 0;
    s_tail = 0;
    s_count = 0;
}

/* Grow the ring buffer, linearizing existing entries into the new array so the
 * wrap-around segment is reordered to a contiguous [0, count) range. Returns
 * false on allocation failure or when the hard cap is reached. */
static bool grow_queue_locked(void)
{
    size_t new_capacity;
    BtEvent *new_queue;
    size_t i;

    if (s_capacity == 0U) {
        new_capacity = BT_EVENT_QUEUE_INITIAL_CAPACITY;
    } else if (s_capacity >= BT_EVENT_QUEUE_MAX_CAPACITY) {
        return false;
    } else {
        new_capacity = s_capacity * 2U;
        if (new_capacity > BT_EVENT_QUEUE_MAX_CAPACITY) {
            new_capacity = BT_EVENT_QUEUE_MAX_CAPACITY;
        }
    }
    if (new_capacity > SIZE_MAX / sizeof(BtEvent)) {
        return false;
    }

    new_queue = (BtEvent *)malloc(new_capacity * sizeof(BtEvent));
    if (new_queue == NULL) {
        return false;
    }

    for (i = 0U; i < s_count; ++i) {
        new_queue[i] = s_queue[(s_head + i) % s_capacity];
    }

    free(s_queue);
    s_queue = new_queue;
    s_capacity = new_capacity;
    s_head = 0U;
    s_tail = s_count;
    return true;
}

void bt_event_queue_reset(void)
{
    pthread_mutex_lock(&s_queue_mutex);
    free_queue_locked();
    s_dropped_count = 0;
    pthread_mutex_unlock(&s_queue_mutex);
}

bool bt_event_post(const BtEvent *event)
{
    bool ok = false;

    if (event == NULL) {
        return false;
    }

    pthread_mutex_lock(&s_queue_mutex);

    /* Lazily allocate on first use; an allocation failure here falls through to
     * the full-queue handling below (which will also fail to grow and drop). */
    if (s_capacity == 0U) {
        (void)grow_queue_locked();
    }

    if (s_count < s_capacity) {
        s_queue[s_tail] = *event;
        s_tail = (s_tail + 1U) % s_capacity;
        s_count++;
        ok = true;
    } else if (grow_queue_locked()) {
        s_queue[s_tail] = *event;
        s_tail = (s_tail + 1U) % s_capacity;
        s_count++;
        ok = true;
    } else if (event->type == BT_EVENT_PBAP_STATE && s_capacity > 0U) {
        /* Cap reached or out of memory: keep the latest PBAP state by evicting
         * the oldest event, preserving the original overflow behavior. */
        s_head = (s_head + 1U) % s_capacity;
        s_count--;
        s_dropped_count++;
        s_queue[s_tail] = *event;
        s_tail = (s_tail + 1U) % s_capacity;
        s_count++;
        ok = true;
    } else {
        s_dropped_count++;
    }

    pthread_mutex_unlock(&s_queue_mutex);

    return ok;
}

bool bt_event_pop(BtEvent *event)
{
    bool ok = false;

    if (event == NULL) {
        return false;
    }

    pthread_mutex_lock(&s_queue_mutex);
    if (s_count > 0U) {
        *event = s_queue[s_head];
        s_head = (s_head + 1U) % s_capacity;
        s_count--;
        ok = true;
    }
    pthread_mutex_unlock(&s_queue_mutex);

    return ok;
}

size_t bt_event_queue_dropped_count(void)
{
    size_t dropped_count;

    pthread_mutex_lock(&s_queue_mutex);
    dropped_count = s_dropped_count;
    pthread_mutex_unlock(&s_queue_mutex);

    return dropped_count;
}
