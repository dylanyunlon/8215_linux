#include "synchronizedtaskexecution.h"
#include <string.h>
#include <thread>

namespace universal_utils {

static const bool DBG = false;

#define SYNCHRONIZEDTASKEXECUTION_BASE_ERROR -1
#define SYNCHRONIZEDTASKEXECUTION_MEMORY_ERROR -2
#define SYNCHRONIZEDTASKEXECUTION_OK 0

    SynchronizedTaskExecution::SynchronizedTaskExecution()
    {
        initTaskExecution();
    }

    SynchronizedTaskExecution::~SynchronizedTaskExecution()
    {
        deinitTaskExecution();
    }

    int SynchronizedTaskExecution::initTaskExecution()
    {
        m_consumer = std::move(std::thread(&SynchronizedTaskExecution::consumeTask, this));

        m_task = new Task();
        if (m_task == nullptr) {
            printf("new error\n");
            return SYNCHRONIZEDTASKEXECUTION_MEMORY_ERROR;
        }
        memset(m_task, 0, sizeof(Task));

        return SYNCHRONIZEDTASKEXECUTION_OK;
    }

    int SynchronizedTaskExecution::deinitTaskExecution()
    {
        if (DBG) printf("deinitTaskExecution\n");
        m_isThreadRun = false;

        {
            std::unique_lock<std::mutex> lck(m_mutex);
            m_taskNum++;
            if (m_task != nullptr) {
                delete m_task;
                m_task = nullptr;
            }
            mTaskNotEmpty.notify_all();
        }

        if (m_consumer.joinable()) {
            m_consumer.join();
        }



        return SYNCHRONIZEDTASKEXECUTION_OK;
    }

    int SynchronizedTaskExecution::taskExecution(taskProcess callback, void* context)
    {
        if (m_task == nullptr) {
            printf("m_task is null\n");
            return SYNCHRONIZEDTASKEXECUTION_MEMORY_ERROR;
        }

        if (callback == nullptr) {
            printf("error:callback is null\n");
            return SYNCHRONIZEDTASKEXECUTION_MEMORY_ERROR;
        }

        m_orderLock.lock();

        if (DBG) printf("begin produceTask\n");
        produceTask(callback, context);
        if (DBG) printf("produce Task done\n");

        m_orderLock.unlock();

        return SYNCHRONIZEDTASKEXECUTION_OK;
    }

    int SynchronizedTaskExecution::produceTask(taskProcess callback, void* context)
    {
        int ret = SYNCHRONIZEDTASKEXECUTION_OK;

        std::unique_lock<std::mutex> lock(m_mutex);

        // begin new task
        if (DBG) printf("begin new task\n");
        if (m_task != nullptr) {
            m_task->callback = callback;
            m_task->context = context;
            m_taskNum++;
            // new task end
        } else {
            if (DBG) printf("do not have task to handle\n");
            ret = SYNCHRONIZEDTASKEXECUTION_MEMORY_ERROR;
        }

        mTaskNotEmpty.notify_all();   //notify consumer that they can consume products

        while (m_taskNum == 1) {  // There is one task being processed
            if (DBG) printf("producer is waiting for am empty slot\n");
            mTaskNotFull.wait(lock);
        }

        lock.unlock();

        return ret;
    }


    void SynchronizedTaskExecution::consumeTask()
    {
        while (m_isThreadRun == true) {
            if (DBG) printf("begin consume\n");
            consumeItem();
            if (DBG) printf("consume end\n");
        }
        return;
    }

    int SynchronizedTaskExecution::consumeItem()
    {
        int ret = SYNCHRONIZEDTASKEXECUTION_OK;

        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_taskNum == 0) {
            if (DBG) printf("Consumer is waiting for task\n");
            mTaskNotEmpty.wait(lock);//wait Task
        }
        if (DBG) printf("begin consume task\n");
        if (m_task != nullptr) {
            m_task->callback(m_task->context);
            m_taskNum--;
        } else {
            if (DBG) printf("do not have task to consume\n");
            ret = SYNCHRONIZEDTASKEXECUTION_MEMORY_ERROR;
        }

        mTaskNotFull.notify_all();
        lock.unlock();

        return ret;
    }
}