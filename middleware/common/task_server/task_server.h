#ifndef __MW_TASK_SERVER_H__
#define __MW_TASK_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  纯 C 任务队列（生产者-消费者)
 *
 * 结构体内部不对外暴露（定义在 task_server.c），调用方只持有
 * mw_task_server_t*。
 *
 * 数据所有权：生产者 malloc 任务数据 -> mw_task_server_add() 入队 ->
 *             工作线程回调处理 -> 回调返回后由本模块 free(data)。
 *
 * 典型用法：
 *   static void on_task(void *data) { // 处理 data }
 *   mw_task_server_t *srv = mw_task_server_create(on_task);   // 建即运行
 *   mw_task_server_add(srv, my_malloced_msg);
 *   ...
 *   mw_task_server_destroy(srv);     // 排空剩余任务、join 工作线程并释放
 */

typedef struct mw_task_server mw_task_server_t;

/** 任务回调：在工作线程内被调用，处理完后 data 由本模块释放。 */
typedef void (*mw_task_cb_t)(void* data);

/** 创建并启动任务队列（分配实例 + 起工作线程）。cb 为 NULL 或失败时返回 NULL。
 */
mw_task_server_t* mw_task_server_create(mw_task_cb_t cb);

/**
 * @brief  入队一个堆上分配的任务消息。非阻塞；所有权转移给 server
 *         （回调返回后由 server 用 free() 释放 data）。
 * @return 0 成功；-1 失败（参数非法或分配节点失败；此时 data 仍归调用者）。
 * @note   destroy 之后不应再调用。
 */
int mw_task_server_add(mw_task_server_t* s, void* data);

/** 停止、排空并处理剩余任务、join 工作线程并释放实例。传 NULL 安全。 */
void mw_task_server_destroy(mw_task_server_t* s);

#ifdef __cplusplus
}
#endif

#endif /* __MW_TASK_SERVER_H__ */
