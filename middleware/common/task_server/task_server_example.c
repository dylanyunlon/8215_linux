/** task_server_example.c -- 用法示例（OSAL 化）。非库源码，不纳入 SRCS。 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "osal.h"
#include "task_server.h"

/* ---- 1. 定义你的任务消息（须堆分配，处理完由 task_server 自动 free）---- */
typedef struct {
    uint16_t msg_id;
    uint16_t len;
    uint8_t payload[32];
} comm_msg_t;

/* ---- 2. 工作线程回调：消费一条消息 ---- */
static void on_task(void* data) {
    comm_msg_t* msg = (comm_msg_t*)data;
    /* 在工作线程上下文里处理：解析 / 转发 / 写硬件等。
       真实工程里此处可用 mw_log_info 替代 printf。*/
    printf("[worker] recv msg_id=%u len=%u payload=\"%s\"\n", msg->msg_id,
           msg->len, (char*)msg->payload);
    /* 回调返回后 task_server 会自动 free(msg) -- 切勿在此再 free */
}

/* ---- 3. 生产者上下文：携带 server 与完成信号量 ---- */
typedef struct {
    mw_task_server_t* srv;
    osal_sem_t* done;
} prod_ctx_t;

/* ---- 4. 生产者线程：构造消息并投递 ---- */
static void* producer(void* arg) {
    prod_ctx_t* c = (prod_ctx_t*)arg;
    int i;

    for (i = 0; i < 5; i++) {
        comm_msg_t* msg = (comm_msg_t*)malloc(sizeof(*msg)); /* 必须堆分配 */
        if (msg == NULL) continue;
        msg->msg_id = (uint16_t)(1000 + i);
        msg->len = (uint16_t)snprintf((char*)msg->payload, sizeof(msg->payload),
                                      "hello-%d", i);

        /* 入队后 msg 所有权转移给 server，不要再访问/释放它 */
        if (mw_task_server_add(c->srv, msg) != 0) {
            /* 入队失败（如分配节点失败）：msg 仍归本线程，需自行释放 */
            free(msg);
        }
        osal_delay_ms(100); /* 模拟生产节奏（替代 usleep）*/
    }
    osal_sem_post(c->done); /* 通知主线程生产完成（替代 pthread_join）*/
    return NULL;
}

int main(void) {
    mw_task_server_t* srv;
    osal_sem_t* done;
    prod_ctx_t ctx;

    /* 5. 创建并启动任务队列（建即运行） */
    srv = mw_task_server_create(on_task);
    if (srv == NULL) {
        fprintf(stderr, "create failed\n");
        return 1;
    }

    /* 6. 启动生产者线程投递任务（detach，经 done 信号量同步）*/
    done = osal_sem_create(0, 1);
    if (done == NULL) {
        mw_task_server_destroy(srv);
        return 1;
    }
    ctx.srv = srv;
    ctx.done = done;
    if (osal_thread_create(NULL, "producer", producer, &ctx, 0, 0) != OSAL_OK) {
        fprintf(stderr, "producer create failed\n");
        osal_sem_delete(done);
        mw_task_server_destroy(srv);
        return 1;
    }
    osal_sem_wait(done); /* 等生产者投递完（替代 pthread_join）*/

    /* 7. 销毁：排空并处理剩余任务、等工作线程退出、释放实例 */
    mw_task_server_destroy(srv);
    osal_sem_delete(done);

    printf("done\n");
    return 0;
}
