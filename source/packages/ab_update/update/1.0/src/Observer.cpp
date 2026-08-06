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

#include "Observer.hpp"

namespace atcupdateservice {

bool ObserverManager::notify(const std::string &name, const std::string& action, const UpdateMessage &msg) {

    utils::LockGuard<utils::Mutex> lg(m_mx);

    auto it = m_observers.find(name);
    if (it == m_observers.end()) {
        return false;
    }
    return it->second->notify(action, msg);
}

bool ObserverManager::registerObserver(const std::string &name, IObserver::ptr observer) {
    utils::LockGuard<utils::Mutex> lg(m_mx);
    auto it = m_observers.find(name);
    if (it == m_observers.end()) {
        m_observers.insert(std::make_pair(name, observer));
        return true;
    }
    return false;
}

bool ObserverManager::notifyAll(const std::string& action, const UpdateMessage &msg) {
     utils::LockGuard<utils::Mutex> lg(m_mx);

    for (auto &item : m_observers) {
        item.second->notify(action, msg);
    }
    return true;
}

bool ObserverManager::removeObserver(const std::string &name) {
    utils::LockGuard<utils::Mutex> lg(m_mx);
    auto it = m_observers.find(name);

    if (it != m_observers.end()) {
        m_observers.erase(name);
        return true;
    }
    return false;
}

}
