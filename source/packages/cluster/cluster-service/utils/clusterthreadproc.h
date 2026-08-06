#ifndef THREADPROC_H1
#define THREADPROC_H1
#include <clusterthread.h>
#include <mutex>
#include <condition_variable>
#include <functional>
class ThreadProc : public ClusterThread
{
public:
    ThreadProc(const std::function<void()> &fun) : m_funtion(fun) {m_run = true;}
    void threadRun() override {
        while (m_run) {
           std::unique_lock<std::mutex> lock(m_mutex);
           if (m_triggerCount == 0) {
                m_condition.wait(lock);
           }
           --m_triggerCount;
           lock.unlock();
           if (m_run) {
                m_funtion();
           }
        }
    }

    bool threadStop() {
        m_run = false;
        m_condition.notify_all();

        return ClusterThread::threadStop();
    }

    void triggerProc() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_triggerCount++;
        return m_condition.notify_all();
    }

private:
    std::function<void()> m_funtion;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    int m_triggerCount = 0;
};

#endif // CTHREADPROC_H
