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

#ifndef ATC_TSQUEUE_HPP
#define ATC_TSQUEUE_HPP

#include <queue>
#include "utils/Lock.hpp"

namespace atcupdateservice {
namespace utils {

//Nonblock, thread safe
template <class T, class LockType = Mutex>
class PriorityTSQueue {
public:
    void push(const T &val) {
        LockGuard<LockType> lg(m_mx);
        m_que.push(val);
    }
    bool top(T &val) {
        LockGuard<LockType> lg(m_mx);

        if (m_que.empty()) {
            return false;
        }
        val = m_que.top();
        m_que.pop();
        return true;
    }
    unsigned size() const {
        LockGuard<LockType> lg(m_mx);
        return m_que.size();
    }
private:
    std::priority_queue<T> m_que;
    LockType m_mx;
};

//Nonblock, thread safe
template <class T, class LockType = Mutex>
class TSQueue {
public:
    void push(const T &val) {
        LockGuard<LockType> lg(m_mx);
        m_que.push(val);
    }
    bool front(T &val) {
        LockGuard<LockType> lg(m_mx);

        if (m_que.empty()) {
            return false;
        }
        val = m_que.front();
        m_que.pop();
        return true;
    }
    bool front(T *val) {
        LockGuard<LockType> lg(m_mx);

        if (m_que.empty()) {
            return false;
        }
        *val = m_que.front();
        m_que.pop();
        return true;
    }
    unsigned size() const {
        LockGuard<LockType> lg(m_mx);
        return m_que.size();
    }
private:
    std::queue<T> m_que;
    LockType m_mx;
};

}
}


#endif