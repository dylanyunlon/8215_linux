#ifndef CLUSTERTHREAD_H
#define CLUSTERTHREAD_H

#include <thread>
#include <mutex>
#include <atomic>
class ClusterThread
{
public:
    virtual ~ClusterThread();
    void setDeath();
    bool threadStart();
    bool threadStop();
    void threadTerminated();
    unsigned long long getThreadId();

protected:
    std::thread *m_thread = nullptr;
    virtual void threadRun() = 0;
    std::atomic_bool m_run {false};
};

#endif // CLUSTERTHREAD_H
