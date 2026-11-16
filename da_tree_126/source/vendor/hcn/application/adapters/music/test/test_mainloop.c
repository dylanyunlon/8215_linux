#include "compat/awtk.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static pthread_t s_main;
static int s_idle_hits, s_repeat_hits, s_t_once, s_t_rep, s_t_removed_self, s_cross_thread_violation;
static uint32_t s_rep_id, s_victim_id;

static ret_t idle_cb(const idle_info_t *i) { (void)i; if (!pthread_equal(pthread_self(), s_main)) s_cross_thread_violation++; s_idle_hits++; return RET_REMOVE; }
static ret_t idle_repeat_cb(const idle_info_t *i) { (void)i; return (++s_repeat_hits < 3) ? RET_REPEAT : RET_REMOVE; }
static ret_t t_once_cb(const timer_info_t *t) { (void)t; if (!pthread_equal(pthread_self(), s_main)) s_cross_thread_violation++; s_t_once++; return RET_REMOVE; }
static ret_t t_rep_cb(const timer_info_t *t) { (void)t; s_t_rep++; return RET_REPEAT; }
static ret_t t_victim_cb(const timer_info_t *t) { (void)t; s_t_removed_self++; return RET_REPEAT; }
/* timer that removes another timer and adds a new one from inside its callback (re-entrancy) */
static ret_t t_reenter_cb(const timer_info_t *t) { (void)t; timer_remove(s_victim_id); timer_add(t_once_cb, NULL, 1); return RET_REMOVE; }
/* timer that removes ITSELF via timer_remove and also returns REPEAT: must stop */
static uint32_t s_self_id;
static ret_t t_selfrm_cb(const timer_info_t *t) { (void)t; s_t_removed_self += 1000; timer_remove(s_self_id); return RET_REPEAT; }

#define N_THREADS 8
#define N_PER 2000
static void *producer(void *a) { (void)a; for (int i = 0; i < N_PER; i++) idle_queue(idle_cb, NULL); return NULL; }

int main(void)
{
    s_main = pthread_self();
    pthread_t th[N_THREADS];
    for (int i = 0; i < N_THREADS; i++) pthread_create(&th[i], NULL, producer, NULL);

    idle_queue(idle_repeat_cb, NULL);
    timer_add(t_once_cb, NULL, 5);
    s_rep_id = timer_add(t_rep_cb, NULL, 10);
    s_victim_id = timer_add(t_victim_cb, NULL, 15);
    timer_add(t_reenter_cb, NULL, 20);
    s_self_id = timer_add(t_selfrm_cb, NULL, 25);
    assert(timer_remove(99999) != RET_OK);

    for (int i = 0; i < 60; i++) { music_mainloop_poll(); usleep(5000); }
    for (int i = 0; i < N_THREADS; i++) pthread_join(th[i], NULL);
    for (int i = 0; i < 5; i++) music_mainloop_poll();

    printf("idle hits=%d (want %d)  repeat-idle runs=%d (want 3)\n", s_idle_hits, N_THREADS * N_PER, s_repeat_hits);
    printf("timer once=%d (want 2: direct + re-entrant add)  repeating=%d (>=10)\n", s_t_once, s_t_rep);
    printf("victim fired=%d (removed by other timer, so small)  self-removed total=%d (want exactly 1000)\n", s_t_removed_self, s_t_removed_self);
    printf("cross-thread callbacks=%d (want 0)  pending=%d (want 0)\n", s_cross_thread_violation, music_mainloop_pending());
    assert(s_idle_hits == N_THREADS * N_PER);
    assert(s_repeat_hits == 3);
    assert(s_t_once == 2);
    assert(s_t_rep >= 10);
    assert(s_cross_thread_violation == 0);
    assert(music_mainloop_pending() == 0);
    timer_remove(s_rep_id);
    music_mainloop_poll();
    music_mainloop_deinit();
    puts("ALL PASS");
    return 0;
}
