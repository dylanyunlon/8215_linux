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
#include <unistd.h>

#include <sstream>
#include <functional>
#include <thread>

#include "monitor/Monitor.hpp"
#include "utils/macro.hpp"
#include "updater/Updater.hpp"

namespace atcupdateservice {
namespace monitor {

IMonitorItem::IMonitorItem(const std::string &name)
    : m_name(name) {
}

const std::string &IMonitorItem::getName() const
{
    return m_name;
}

std::string IMonitorItem::dump() const {
    std::stringstream ss;
    ss << "< " << m_name << " , " << getKey() << " >";

    return ss.str();
}

// Monitor
IMonitorItem::ptr Monitor::getMonitor(unsigned key) {
    utils::RDLockGuard<utils::RWLock> lg(m_rw);

    return (m_monitors.find(key) == m_monitors.end()) ? nullptr : m_monitors[key];
}

void Monitor::start() {
    try {
        m_thread.reset(new std::thread(std::bind(&Monitor::run, this)));
    } catch(std::exception &e) {

    }
}

Monitor::Monitor()
    : m_thread(nullptr) {
}

Monitor::~Monitor() {
    if(m_thread && m_thread->joinable())
        join();
    m_thread.reset();
}

void Monitor::join() {
    if(m_thread->joinable())
        m_thread->join();
}

void Monitor::run() {
    bool ok = 0;
    update(ok);
    auto upd = updater::Updater::Upd::getInstance();

    while(1) {
        usleep(1000 * m_intervals);
        if(update(ok) == false) {
            ATCLOGE("failed to update loading");
        }
        if (ok == false) {
            ATC_STREAM_LOGI() << "try pause" << std::endl;
            upd -> pause();
        } else {
            upd -> resume();
        }
        //ATCLOGI("current cpu load is : %.1f", payload());
    }
}

bool Monitor::removeMonitor(unsigned key) {
    utils::WRLockGuard<utils::RWLock> lg(m_rw);

    if (m_monitors.find(key) != m_monitors.end()) {
        m_monitors.erase(key);
        return true;
    }

    return false;
}

std::string Monitor::dump() const {
    std::stringstream ss;
    utils::RDLockGuard<utils::RWLock> lg(m_rw);

    for (const auto &item : m_monitors) {
        ss << item.second->dump() << std::endl;
    }

    return ss.str();
}

bool Monitor::update(bool &ok) {
    utils::RDLockGuard<utils::RWLock> lg(m_rw);
    ok = true;
    for (auto &item : m_monitors) {
        if (item.second->update() == false) {
            return false;
        }
        if (item.second->loadingStatus() == false) {
            ok = false;
        }
    }

    return true;
}

double Monitor::payload() {
    double sum = 0.0;
    unsigned totWeight = 0;

    for (auto &item : m_monitors) {
        unsigned weight = 0;
        if (item.second->update() == false) {
            return 0.0;
        }
        weight = item.second->weight();
        sum += weight * item.second->currentLoading();
        totWeight += weight;
    }

    return sum/totWeight;
}

bool Monitor::notify(const std::string& actionName, const UpdateMessage &msg) {
    bool succeed = true;
    for(auto &item : m_monitors) {
        succeed = succeed & item.second->notify(actionName, msg);
    }
    return succeed;
}

} // end of monitor
} // end of namespace atcupdateservice