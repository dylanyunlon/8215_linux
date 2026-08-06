#ifndef __SYNCHRONIZEDTASKEXECUTION_H__
#define __SYNCHRONIZEDTASKEXECUTION_H__

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "orderedlock.h"

namespace universal_utils {
    // c++11
    // using taskProcess = std::function<void*(void*)>;
    typedef void(*taskProcess)(void*);
    struct Task {
        taskProcess callback = nullptr;
        void *context = nullptr;
        int reseve = 0;
    };

    class SynchronizedTaskExecution
    {
    public:
        SynchronizedTaskExecution();
        SynchronizedTaskExecution(const SynchronizedTaskExecution& obj) = delete;
        SynchronizedTaskExecution(SynchronizedTaskExecution&&) = delete;
        SynchronizedTaskExecution& operator=(const SynchronizedTaskExecution& obj) = delete;

        virtual ~SynchronizedTaskExecution();
        int taskExecution(taskProcess callback, void* context);

    private:
        int initTaskExecution();
        int deinitTaskExecution();
        int produceTask(taskProcess callback, void* context);
        void consumeTask();
        int consumeItem();

        // consumer threads
        std::thread m_consumer;
        std::atomic_bool m_isThreadRun {true};

        // mutex locks for Condition Variables
        std::mutex m_mutex;
        // There is No Task to deal with, consumers need to wait
        std::condition_variable mTaskNotFull;
        // There is a task processing, producers need to wait
        std::condition_variable mTaskNotEmpty;

        // Multi-producer, Single consumer, avoiding multiple
        // producers waiting on the same condition
        std::mutex m_taskListLock;

        Task* m_task = nullptr;
        int m_taskNum = 0;

        OrderedLock m_orderLock;
    };
}
#endif
