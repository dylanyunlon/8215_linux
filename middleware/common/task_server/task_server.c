/**
 * task_server.c -- 纯 C 任务队列（生产者-消费者），OSAL 化改造版。
 *
 */

#include <stdlib.h>
#include <stdbool.h>

#include "osal.h"
#include "mw_log.h"
#include "mw_lock.h"
#include "task_server.h"

/** 待处理任务节点（单向链表） */
struct task_node {
    void* data;
    struct task_node* next;
};

struct mw_task_server {
    osal_thread_t tid;   ///< 工作线程句柄（detach）
    bool running;        ///< 工作线程运行标志（mutex 保护）
    mw_task_cb_t cb;     ///< 任务回调
    osal_mutex_t mutex;  ///< 保护 head/tail/running
    osal_sem_t* cond;    ///< 新任务/停止唤醒（计数信号量，替代 cond）
    osal_sem_t* done;    ///< 工作线程退出通知（替代 pthread_join）
    struct task_node* head;
    struct task_node* tail;
};

/* ---- 工作线程 ---- */
static void* task_worker(void* arg) {
    mw_task_server_t* s = (mw_task_server_t*)arg;

    for (;;) {
        struct task_node* local;

        /* 等待：running 且队列为空时阻塞；被唤醒后 splice 整条 buffer 到本地。
         * cond_wait 语义 = 释放互斥 -> 等信号 -> 重取互斥；计数信号量 post
         * 不丢失。*/
        osal_mutex_lock(&s->mutex);
        while (s->running && s->head == NULL) {
            osal_mutex_unlock(&s->mutex);
            osal_sem_wait(s->cond);
            osal_mutex_lock(&s->mutex);
        }

        local = s->head;
        s->head = s->tail = NULL;
        osal_mutex_unlock(&s->mutex);

        /* 不加锁处理本地链表（回调可能耗时，避免长期持锁）*/
        while (local) {
            struct task_node* next = local->next;
            if (s->cb) s->cb(local->data);
            free(local->data);
            free(local);
            local = next;
        }

        /* 退出判定：已停止且 buffer 为空 -> 退出；否则继续（含排空迟到任务）*/
        osal_mutex_lock(&s->mutex);
        if (!s->running && s->head == NULL) {
            osal_mutex_unlock(&s->mutex);
            break;
        }
        osal_mutex_unlock(&s->mutex);
    }

    osal_sem_post(
        s->done); /* 通知 destroy：工作线程已退出（替代 pthread_join）*/
    return NULL;
}

mw_task_server_t* mw_task_server_create(mw_task_cb_t cb) {
    mw_task_server_t* s;

    if (cb == NULL) return NULL;

    s = (mw_task_server_t*)calloc(1, sizeof(*s));
    if (s == NULL) return NULL;
    s->cb = cb;
    s->running = true;

    if (osal_mutex_init(&s->mutex, false) != OSAL_OK) {
        free(s);
        return NULL;
    }
    s->cond = osal_sem_create(0, 1); /* 初值 0：无任务时阻塞 */
    if (s->cond == NULL) {
        osal_mutex_destroy(&s->mutex);
        free(s);
        return NULL;
    }
    s->done = osal_sem_create(0, 1); /* 初值 0：destroy 等待退出 */
    if (s->done == NULL) {
        osal_sem_delete(s->cond);
        osal_mutex_destroy(&s->mutex);
        free(s);
        return NULL;
    }
    if (osal_thread_create(&s->tid, "mw_task_srv", task_worker, s, 0, 0) !=
        OSAL_OK) {
        mw_log_error("mw_task_server: create thread failed\n");
        osal_sem_delete(s->done);
        osal_sem_delete(s->cond);
        osal_mutex_destroy(&s->mutex);
        free(s);
        return NULL;
    }
    return s;
}

int mw_task_server_add(mw_task_server_t* s, void* data) {
    struct task_node* node;

    if (s == NULL || data == NULL) return -1;

    node = (struct task_node*)malloc(sizeof(*node));
    if (node == NULL) {
        mw_log_error("mw_task_server: malloc node failed\n");
        return -1; /* data 仍归调用者 */
    }
    node->data = data;
    node->next = NULL;

    {
        MW_MUTEX_GUARD(&s->mutex);
        if (s->tail)
            s->tail->next = node;
        else
            s->head = node;
        s->tail = node;
    }
    /* 解锁后再 post（合法且省一次上下文切换）*/
    osal_sem_post(s->cond);
    return 0;
}

void mw_task_server_destroy(mw_task_server_t* s) {
    if (s == NULL) return;

    {
        MW_MUTEX_GUARD(&s->mutex);
        s->running = false;
        osal_sem_post(s->cond); /* 唤醒可能在等待的工作线程 */
    }
    osal_sem_wait(s->done); /* 等工作线程退出（替代 pthread_join）*/

    /* 排空残留（正常停机后应为空）：仅 free，不回调 */
    {
        struct task_node* n = s->head;
        while (n) {
            struct task_node* next = n->next;
            free(n->data);
            free(n);
            n = next;
        }
    }

    osal_sem_delete(s->done);
    osal_sem_delete(s->cond);
    osal_mutex_destroy(&s->mutex);
    free(s);
}
