/**
 * @file awtk_stub.h
 * @brief Minimal AWTK API stub for CLI/headless testing.
 *
 * Replaces awtk.h when building without the AWTK GUI framework.
 * Provides pthread-based implementations of:
 *   - idle_queue()   → runs callback immediately on a worker thread
 *   - timer_add()    → spawns a detached thread with usleep
 *   - timer_remove() → sets a cancel flag (best-effort)
 *   - timer_manager() → returns a stub with get_elapsed_ms
 *
 * Build with: -DAWTK_STUB to activate this instead of real awtk.h
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#ifndef AWTK_STUB_H
#define AWTK_STUB_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Basic AWTK types
 *==========================================================================*/
typedef int ret_t;
#define RET_OK      0
#define RET_FAIL    1
#define RET_REMOVE  2
#define RET_REPEAT  3

#define TK_INVALID_ID  0
#define TRUE  1
#define FALSE 0

/*============================================================================
 * idle_info_t — context for idle callbacks
 *==========================================================================*/
typedef struct _idle_info_t {
    void* ctx;
} idle_info_t;

typedef ret_t (*idle_func_t)(const idle_info_t* idle);

/**
 * idle_queue — dispatch a callback.
 * In real AWTK this runs on the main loop; here we run it directly
 * in a short-lived detached thread (simulates async dispatch).
 */
typedef struct {
    idle_func_t func;
    void*       ctx;
} _idle_task_t;

static void* _idle_thread_func(void* arg) {
    _idle_task_t* task = (_idle_task_t*)arg;
    idle_info_t info;
    info.ctx = task->ctx;
    task->func(&info);
    free(task);
    return NULL;
}

static inline ret_t idle_queue(idle_func_t func, void* ctx) {
    _idle_task_t* task = (_idle_task_t*)calloc(1, sizeof(_idle_task_t));
    if (!task) return RET_FAIL;
    task->func = func;
    task->ctx  = ctx;

    pthread_t t;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&t, &attr, _idle_thread_func, task);
    pthread_attr_destroy(&attr);
    return RET_OK;
}

/*============================================================================
 * timer_info_t — context for timer callbacks
 *==========================================================================*/
typedef struct _timer_info_t {
    void* ctx;
} timer_info_t;

typedef ret_t (*timer_func_t)(const timer_info_t* timer);

/*============================================================================
 * Timer stub — backed by detached threads + usleep
 *
 * timer_add returns a small incrementing ID.
 * timer_remove sets a cancel flag (best-effort, timer may still fire).
 *==========================================================================*/
#define _STUB_MAX_TIMERS 64

typedef struct {
    uint32_t     id;
    volatile int cancelled;
    timer_func_t func;
    void*        ctx;
    uint32_t     interval_ms;
} _stub_timer_t;

static _stub_timer_t _stub_timers[_STUB_MAX_TIMERS];
static uint32_t      _stub_timer_next_id = 1;
static pthread_mutex_t _stub_timer_mutex = PTHREAD_MUTEX_INITIALIZER;

static void* _timer_thread_func(void* arg) {
    _stub_timer_t* st = (_stub_timer_t*)arg;
    uint32_t id = st->id;

    do {
        usleep(st->interval_ms * 1000);

        pthread_mutex_lock(&_stub_timer_mutex);
        int cancelled = st->cancelled;
        pthread_mutex_unlock(&_stub_timer_mutex);

        if (cancelled) break;

        timer_info_t info;
        info.ctx = st->ctx;
        ret_t r = st->func(&info);

        if (r == RET_REMOVE) break;
        /* RET_REPEAT → loop again */
    } while (1);

    /* Clear slot */
    pthread_mutex_lock(&_stub_timer_mutex);
    st->id = 0;
    st->func = NULL;
    pthread_mutex_unlock(&_stub_timer_mutex);

    return NULL;
}

static inline uint32_t timer_add(timer_func_t func, void* ctx, uint32_t interval_ms) {
    pthread_mutex_lock(&_stub_timer_mutex);

    /* Find free slot */
    int slot = -1;
    for (int i = 0; i < _STUB_MAX_TIMERS; i++) {
        if (_stub_timers[i].id == 0) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        pthread_mutex_unlock(&_stub_timer_mutex);
        return TK_INVALID_ID;
    }

    uint32_t id = _stub_timer_next_id++;
    if (_stub_timer_next_id == 0) _stub_timer_next_id = 1; /* avoid 0 */

    _stub_timers[slot].id          = id;
    _stub_timers[slot].cancelled   = 0;
    _stub_timers[slot].func        = func;
    _stub_timers[slot].ctx         = ctx;
    _stub_timers[slot].interval_ms = interval_ms;

    pthread_mutex_unlock(&_stub_timer_mutex);

    pthread_t t;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&t, &attr, _timer_thread_func, &_stub_timers[slot]);
    pthread_attr_destroy(&attr);

    return id;
}

static inline void timer_remove(uint32_t id) {
    if (id == TK_INVALID_ID) return;
    pthread_mutex_lock(&_stub_timer_mutex);
    for (int i = 0; i < _STUB_MAX_TIMERS; i++) {
        if (_stub_timers[i].id == id) {
            _stub_timers[i].cancelled = 1;
            break;
        }
    }
    pthread_mutex_unlock(&_stub_timer_mutex);
}

/*============================================================================
 * timer_manager stub — provides get_elapsed_ms
 *==========================================================================*/
typedef struct {
    uint64_t (*get_elapsed_ms)(void* self);
} _stub_timer_manager_t;

static struct timespec _stub_start_time;
static int _stub_time_inited = 0;

static uint64_t _stub_get_elapsed_ms(void* self) {
    (void)self;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    uint64_t ms = (uint64_t)(now.tv_sec - _stub_start_time.tv_sec) * 1000
                + (uint64_t)(now.tv_nsec - _stub_start_time.tv_nsec) / 1000000;
    return ms;
}

static _stub_timer_manager_t _stub_tm_instance = { _stub_get_elapsed_ms };

static inline _stub_timer_manager_t* timer_manager(void) {
    if (!_stub_time_inited) {
        clock_gettime(CLOCK_MONOTONIC, &_stub_start_time);
        _stub_time_inited = 1;
    }
    return &_stub_tm_instance;
}

/*============================================================================
 * Widget stubs (not used in CLI, but needed to compile music_app.h)
 *==========================================================================*/
typedef void* widget_t;

#ifdef __cplusplus
}
#endif

#endif /* AWTK_STUB_H */
