#include <iostream>
#include <sstream>
#include "clusterthread.h"
#include <memory>
#include <functional>

ClusterThread::~ClusterThread()
{
    threadTerminated();
    threadStop();
}

void ClusterThread::setDeath()
{
    m_thread->detach();
}

bool ClusterThread::threadStart()
{
    m_run = true;
    if (nullptr == m_thread) {
        m_thread = new std::thread(std::bind(&ClusterThread::threadRun, this));
    }

    return (m_thread != nullptr);
}

bool ClusterThread::threadStop()
{
    if (m_thread) {
        if (m_thread->joinable()) {
            m_thread->join();
        }
        delete m_thread;
        m_thread = nullptr;
    }

    return true;
}

void ClusterThread::threadTerminated()
{
    m_run = false;
}

unsigned long long ClusterThread::getThreadId()
{
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string stid = oss.str();
    unsigned long long tid = std::stoull(stid);

    return tid;
}


