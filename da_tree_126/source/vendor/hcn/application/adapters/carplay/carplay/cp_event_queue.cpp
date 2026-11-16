#include "cp_event_queue.h"

#include <pthread.h>
#include <string.h>

#define CP_EVENT_QUEUE_CAPACITY 64

static pthread_mutex_t s_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static CpEvent s_queue[CP_EVENT_QUEUE_CAPACITY];
static size_t s_head = 0;
static size_t s_tail = 0;
static size_t s_count = 0;
static size_t s_dropped_count = 0;

void cp_event_queue_reset(void)
{
    pthread_mutex_lock(&s_queue_mutex);
    s_head = 0;
    s_tail = 0;
    s_count = 0;
    s_dropped_count = 0;
    for (size_t i = 0; i < CP_EVENT_QUEUE_CAPACITY; ++i) {
        memset(&s_queue[i], 0, sizeof(CpEvent));
    }
    pthread_mutex_unlock(&s_queue_mutex);
}

bool cp_event_post(const CpEvent *event)
{
    bool ok = false;

    if (event == NULL) {
        return false;
    }

    pthread_mutex_lock(&s_queue_mutex);
    if (s_count < CP_EVENT_QUEUE_CAPACITY) {
        s_queue[s_tail] = *event;
        s_tail = (s_tail + 1U) % CP_EVENT_QUEUE_CAPACITY;
        s_count++;
        ok = true;
    } else {
        s_dropped_count++;
    }
    pthread_mutex_unlock(&s_queue_mutex);

    return ok;
}

bool cp_event_pop(CpEvent *event)
{
    bool ok = false;

    if (event == NULL) {
        return false;
    }

    pthread_mutex_lock(&s_queue_mutex);
    if (s_count > 0U) {
        *event = s_queue[s_head];
        s_head = (s_head + 1U) % CP_EVENT_QUEUE_CAPACITY;
        s_count--;
        ok = true;
    }
    pthread_mutex_unlock(&s_queue_mutex);

    return ok;
}

size_t cp_event_queue_dropped_count(void)
{
    size_t dropped_count;

    pthread_mutex_lock(&s_queue_mutex);
    dropped_count = s_dropped_count;
    pthread_mutex_unlock(&s_queue_mutex);

    return dropped_count;
}
