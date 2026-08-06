#include "tasktest.h"
#include "multitask.h"
#include <string.h>
namespace universal_utils {
    // User-defined structure
    struct Data {
        int a = 0;
        int b = 0;
    };
    TaskTest::TaskTest()
    {}

    void* TaskTest::exeAdd(void* context)
    {
        Data *param = (Data*)context;
        if (param == nullptr) {
            printf("param is null\n");
            return nullptr;
        }
        int a = param->a;
        int b = param->b;
        int sum = add(a, b);
        printf("sum:%d\n", sum);

        return (void*)sum;
    }

    int TaskTest::add(int a, int b)
    {
        printf("a=%d, b=%d\n", a, b);
        return a + b;
    }

    void* TaskTest::exeSubraction(void* context)
    {
        printf("exeSubraction enter\n");
        Data *param = (Data*)context;
        if (param == nullptr) {
            printf("param is null\n");
            return nullptr;
        }
        int a = param->a;
        int b = param->b;
        int sub = subraction(a, b);
        printf("sub:%d\n", sub);

        return (void*)sub;
    }

    int TaskTest::subraction(int a, int b)
    {
        printf("a=%d, b=%d\n", a, b);
        return a - b;
    }

    int TaskTest::taskFunc()
    {
        Data param;
        memset(&param, 0, sizeof(param));

        param.a = 10;
        param.b = 43;
        void *context = (void*)&param;
        m_multiTask->taskExecution(exeAdd, context);

        //taskProcess cb = std::bind(&MultiTask::exeSubraction, this, std::placeholders::_1);
        //m_multiTask->taskExecution(cb, context);

        return 0;
    }
}
