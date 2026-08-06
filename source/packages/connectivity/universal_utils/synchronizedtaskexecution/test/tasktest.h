#ifndef __TASKTEST__
#define __TASKTEST__
#include "multitask.h"
namespace universal_utils {
    class TaskTest {
    public:
        TaskTest();
        static void* exeAdd(void* context);
        static int add(int a, int b);
        int taskFunc();

        void* exeSubraction(void* context);
        int subraction(int a, int b);

    private:
        MultiTask *m_multiTask = new MultiTask();
    };
}

#endif
