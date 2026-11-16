/**
 * @file music_mainloop.c
 * @brief idle_queue / timer_add / timer_remove for the LVGL main loop.
 *        See include/compat/awtk.h for the why.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "compat/awtk.h"

#include <pthread.h>
#include <time.h>

typedef struct idle_node {
    idle_func_t       func;
    void             *ctx;
    struct idle_node *next;
} idle_node_t;

typedef struct timer_node {
    uint32_t           id;
    timer_func_t       func;
    void              *ctx;
    uint32_t           interval;
    uint64_t           due;
    bool               running;
    bool               dead;
    struct timer_node *next;
} timer_node_t;

static pthread_mutex_t s_mu = PTHREAD_MUTEX_INITIALIZER;
static idle_node_t    *s_idle_head, *s_idle_tail;
static timer_node_t   *s_timers;
static uint32_t        s_next_id = 1;

static uint64_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000u + (uint64_t)ts.tv_nsec / 1000000u;
}

static void idle_push_locked(idle_node_t *n)
{
    n->next = NULL;
    if (s_idle_tail) s_idle_tail->next = n; else s_idle_head = n;
    s_idle_tail = n;
}

ret_t idle_queue(idle_func_t func, void *ctx)
{
    idle_node_t *n;
    if (func == NULL) return RET_FAIL;
    n = (idle_node_t *)calloc(1, sizeof(*n));
    if (n == NULL) return RET_FAIL;
    n->func = func;
    n->ctx  = ctx;
    pthread_mutex_lock(&s_mu);
    idle_push_locked(n);
    pthread_mutex_unlock(&s_mu);
    return RET_OK;
}

uint32_t timer_add(timer_func_t func, void *ctx, uint32_t interval_ms)
{
    timer_node_t *t;
    uint32_t id;
    if (func == NULL) return TK_INVALID_ID;
    t = (timer_node_t *)calloc(1, sizeof(*t));
    if (t == NULL) return TK_INVALID_ID;
    t->func     = func;
    t->ctx      = ctx;
    t->interval = interval_ms ? interval_ms : 1;
    t->due      = now_ms() + t->interval;
    pthread_mutex_lock(&s_mu);
    id = s_next_id++;
    if (s_next_id == TK_INVALID_ID) s_next_id = 1;
    t->id   = id;
    t->next = s_timers;
    s_timers = t;
    pthread_mutex_unlock(&s_mu);
    return id;
}

ret_t timer_remove(uint32_t id)
{
    timer_node_t *t;
    ret_t ret = RET_FAIL;
    pthread_mutex_lock(&s_mu);
    for (t = s_timers; t != NULL; t = t->next) {
        if (t->id == id && !t->dead) { t->dead = true; ret = RET_OK; break; }
    }
    pthread_mutex_unlock(&s_mu);
    return ret;   /* node is freed lazily by the poll sweep: safe even from inside its own callback */
}

static void sweep_dead_timers_locked(void)
{
    timer_node_t **pp = &s_timers;
    while (*pp != NULL) {
        timer_node_t *t = *pp;
        if (t->dead && !t->running) { *pp = t->next; free(t); }
        else pp = &t->next;
    }
}

void music_mainloop_poll(void)
{
    idle_node_t *list, *n, *next;
    idle_node_t *again_head = NULL, *again_tail = NULL;
    uint64_t t0;

    /* 1) idles: take a snapshot so callbacks that queue more work don't starve the frame */
    pthread_mutex_lock(&s_mu);
    list = s_idle_head;
    s_idle_head = s_idle_tail = NULL;
    pthread_mutex_unlock(&s_mu);

    for (n = list; n != NULL; n = next) {
        idle_info_t info;
        ret_t ret;
        next = n->next;
        info.ctx = n->ctx;
        ret = n->func(&info);
        if (ret == RET_REPEAT) {            /* AWTK idle semantics: keep until RET_REMOVE */
            n->next = NULL;
            if (again_tail) again_tail->next = n; else again_head = n;
            again_tail = n;
        } else {
            free(n);
        }
    }
    if (again_head != NULL) {
        pthread_mutex_lock(&s_mu);
        again_tail->next = s_idle_head;      /* repeating idles run before newly queued ones */
        s_idle_head = again_head;
        if (s_idle_tail == NULL) s_idle_tail = again_tail;
        pthread_mutex_unlock(&s_mu);
    }

    /* 2) timers: never hold the lock while calling back (callbacks add/remove timers) */
    t0 = now_ms();
    for (;;) {
        timer_node_t *t;
        timer_func_t func = NULL;
        timer_info_t ti;

        pthread_mutex_lock(&s_mu);
        for (t = s_timers; t != NULL; t = t->next) {
            if (!t->dead && !t->running && t->due <= t0) break;
        }
        if (t == NULL) {
            sweep_dead_timers_locked();
            pthread_mutex_unlock(&s_mu);
            break;
        }
        t->running = true;
        func = t->func;
        ti.ctx = t->ctx;
        ti.id  = t->id;
        pthread_mutex_unlock(&s_mu);

        {
            ret_t ret = func(&ti);
            pthread_mutex_lock(&s_mu);
            t->running = false;
            if (ret == RET_REPEAT && !t->dead) t->due = t0 + t->interval;  /* > t0: no re-fire this poll */
            else t->dead = true;
            pthread_mutex_unlock(&s_mu);
        }
    }
}

int music_mainloop_pending(void)
{
    int n = 0;
    idle_node_t *p;
    pthread_mutex_lock(&s_mu);
    for (p = s_idle_head; p != NULL; p = p->next) n++;
    pthread_mutex_unlock(&s_mu);
    return n;
}

void music_mainloop_deinit(void)
{
    idle_node_t *i, *in;
    timer_node_t *t, *tn;
    pthread_mutex_lock(&s_mu);
    for (i = s_idle_head; i != NULL; i = in) { in = i->next; free(i); }
    s_idle_head = s_idle_tail = NULL;
    for (t = s_timers; t != NULL; t = tn) { tn = t->next; free(t); }
    s_timers = NULL;
    pthread_mutex_unlock(&s_mu);
}
