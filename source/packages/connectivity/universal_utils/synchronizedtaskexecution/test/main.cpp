#include "tasktest.h"
using namespace universal_utils;

void myThread(TaskTest* mt)
{
    if (mt == nullptr) {
        return;
    }

    for (int i = 0; i < 10000; i++) {
        mt->taskFunc();
        printf("i=%d\n", i);
    }
}

int main()
{
    TaskTest mt;
    std::thread th1(myThread, &mt);
    std::thread th2(myThread, &mt);
    th1.join();
    th2.join();

    //printf("threadid:%d\n", std::this_thread::get_id());

    return 0;
}