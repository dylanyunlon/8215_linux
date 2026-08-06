#ifndef __MULTITASK__
#define __MULTITASK__

#include "synchronizedtaskexecution.h"
namespace universal_utils {
    class MultiTask
    {
    public:
        MultiTask();
        virtual ~MultiTask();
        int taskExecution(taskProcess task, void *context);

    private:
        SynchronizedTaskExecution *m_synchronizedTaskExecution = nullptr;
    };
}
#endif