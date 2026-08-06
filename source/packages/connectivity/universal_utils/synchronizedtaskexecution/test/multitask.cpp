#include <functional>
#include <string.h>

#include "multitask.h"

namespace universal_utils {
    MultiTask::MultiTask()
        : m_synchronizedTaskExecution(new SynchronizedTaskExecution())
    {
        printf("MultiTask\n");
    }

    MultiTask::~MultiTask()
    {
        if (m_synchronizedTaskExecution) {
            delete m_synchronizedTaskExecution;
            m_synchronizedTaskExecution = nullptr;
        }
    }

    int MultiTask::taskExecution(taskProcess task, void* context)
    {
        int ret = 0;

        if (m_synchronizedTaskExecution) {
            if (task) {
                ret = m_synchronizedTaskExecution->taskExecution(task ,context);
                if (ret != 0) {
                    printf("error:m_synchronizedTaskExecution exe fail\n");
                }
            } else {
                printf("error:task is null\n");
                ret = -1;
            }
        } else {
            printf("error:m_synchronizedTaskExecution is null\n");
            ret = -1;
        }

        return ret;
    }
}