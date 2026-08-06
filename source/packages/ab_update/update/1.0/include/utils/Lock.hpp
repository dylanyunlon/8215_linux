/*
copyright (c) 2020 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef ATC_LOCK_HPP
#define ATC_LOCK_HPP

#include <pthread.h>
#include <semaphore.h>

#include "macro.hpp"

namespace atcupdateservice {
namespace utils {

class Mutex {
public:
    Mutex() {
       // ATC_STREAM_LOGE() << "mutex init!" << std::endl;
        int rt = pthread_mutex_init(&m_mx, NULL);
        ATCLOGE_IF(rt < 0, "mutex init failure! errno = %d, error = %s", errno, strerror(errno));
    }
    bool lock() {
        //ATC_STREAM_LOGE() << "mutex locked!" << std::endl;
        int rt = pthread_mutex_lock(&m_mx);
        ATCLOGE_IF(rt < 0, "mutex lock failure! errno = %d, error = %s", errno, strerror(errno));
        return rt == 0;
    }
    bool tryLock() {
       // ATC_STREAM_LOGE() << "mutex trylocked!" << std::endl;
        int rt = pthread_mutex_trylock(&m_mx);
        ATCLOGE_IF(rt < 0, "mutex trylock failure! errno = %d, error = %s", errno, strerror(errno));
        return rt == 0;
    }
    bool unlock() {
      //  ATC_STREAM_LOGE() << "mutex unlocked!" << std::endl;
        int rt = pthread_mutex_unlock(&m_mx);
        ATCLOGE_IF(rt < 0, "mutex unlock failure! errno = %d, error = %s", errno, strerror(errno));
        return rt == 0;
    }
    ~Mutex() {
       /// ATC_STREAM_LOGE() << "mutex destroyed!" << std::endl;
        int rt = pthread_mutex_destroy(&m_mx);
        ATCLOGE_IF(rt < 0, "mutex destroy failure! errno = %d, error = %s", errno, strerror(errno));
    }
private:
    pthread_mutex_t m_mx;
};

class RWLock {
public:
    RWLock() {
        int rt = pthread_rwlock_init(&m_rw, NULL);
        ATCLOGE_IF(rt < 0, "rwmutex init failure! errno = %d, error = %s", errno, strerror(errno));
    }
    bool rdlock() {
        int rt = pthread_rwlock_rdlock(&m_rw);
        ATCLOGE_IF(rt < 0, "rwmutex rwlock failure! errno = %d, error = %s", errno, strerror(errno));
        return rt == 0;
    }
    bool wrlock() {
        int rt = pthread_rwlock_wrlock(&m_rw);
        ATCLOGE_IF(rt < 0, "rwmutex wrlock failure! errno = %d, error = %s", errno, strerror(errno));
        return rt == 0;
    }
    bool tryRDLock() {
        int rt = pthread_rwlock_tryrdlock(&m_rw);
        ATCLOGE_IF(rt < 0, "rwmutex tryrdlock failure! errno = %d, error = %s", errno, strerror(errno));
        return rt == 0;
    }
    bool tryWRLock() {
        int rt = pthread_rwlock_trywrlock(&m_rw);
        ATCLOGE_IF(rt < 0, "rwmutex trywrlock failure! errno = %d, error = %s", errno, strerror(errno));
        return rt == 0;
    }
    bool unlock() {
        int rt = pthread_rwlock_unlock(&m_rw);
        ATCLOGE_IF(rt < 0, "rwmutex unlock failure! errno = %d, error = %s", errno, strerror(errno));
        return rt == 0;
    }
    ~RWLock() {
        int rt = pthread_rwlock_destroy(&m_rw);
        ATCLOGE_IF(rt < 0, "rwmutex destroy failure! errno = %d, error = %s", errno, strerror(errno));
    }
private:
    pthread_rwlock_t m_rw;
};

class Semaphore {
};

#define DEFINE_LOCKGUARD(CLASS, LOCK1, LOCK2)   \
template <class T>                              \
class CLASS {                                   \
public:                                         \
    CLASS(T &mx)                            \
        : m_mx(mx), m_locked(false){            \
        m_mx.LOCK2();                           \
        m_locked = true;                        \
    }                                           \
    void LOCK1() {                              \
        if (!m_locked) {                        \
            m_mx.LOCK2();                       \
            m_locked = true;                    \
        }                                       \
    }                                           \
    void unlock() {                             \
        if (m_locked) {                         \
            m_locked = false;                   \
            m_mx.unlock();                      \
        }                                       \
    }                                           \
    ~CLASS() {                                  \
        if (m_locked)                           \
            m_mx.unlock();                      \
    }                                           \
private:                                        \
    T &m_mx;                                    \
    bool m_locked;                              \
};

DEFINE_LOCKGUARD(LockGuard, lock,  lock)
DEFINE_LOCKGUARD(RDLockGuard, lock, rdlock)
DEFINE_LOCKGUARD(WRLockGuard, lock, wrlock)

}
}

#endif