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

#ifndef ATC_MONITOR_HPP
#define ATC_MONITOR_HPP

#include <memory>
#include <thread>
#include <unordered_map>
#include <map>

#include "utils/Singleton.hpp"
#include "utils/Key.hpp"
#include "utils/Lock.hpp"
#include "Observer.hpp"

namespace atcupdateservice {
namespace monitor {

class IMonitorItem {
public:
    typedef std::shared_ptr<IMonitorItem> ptr;
    IMonitorItem(const std::string &name = "IMointorItem");

    const std::string& getName() const;
    std::string dump() const;
    virtual unsigned getKey() const = 0;
    virtual bool loadingStatus() const = 0;
    virtual double currentLoading() const = 0;
    virtual bool update() = 0;
    virtual unsigned weight() const = 0;
    virtual bool notify(const std::string& actionName, const UpdateMessage &msg) = 0;

protected:
    std::string m_name;
};

class Monitor : public IObserver {
public:
    friend class utils::Singleton<Monitor>;
    ~Monitor();
    std::string dump() const;
    bool update(bool &ok);
    bool removeMonitor(unsigned key);
    IMonitorItem::ptr getMonitor(unsigned key);
    /*
     * payload desides the frequence of polling, NOT desides whether update is available
     * the higher the payload is , the less frequency of polling
     */
    double payload();
    /* this is supposed to be called by REGISTER_GUARD before main*/
    template<class T>
    bool registerMonitor() {
        unsigned key = hash::Key<T>::getKey();
        std::shared_ptr<T> monitor(new T());

        utils::WRLockGuard<utils::RWLock> lg(m_rw);

        if (m_monitors.find(key) == m_monitors.end()) {
            m_monitors[key] = monitor;
            return true;
        }

        return false;
    }
    bool notify(const std::string& actionName, const UpdateMessage &msg) override;
    void join();
    void start();

private:
    void run();
    Monitor();
    Monitor(const Monitor&) = delete;

    std::map<unsigned, IMonitorItem::ptr> m_monitors;
    unsigned m_intervals = 500;            // in ms
    mutable utils::RWLock m_rw;
    std::shared_ptr<std::thread> m_thread;
};

typedef utils::Singleton<Monitor> MonitorMgr;

namespace{

template<class T>
class MonitorRegisterHelper {
public:
    MonitorRegisterHelper() {
        Monitor *mgr = MonitorMgr::getInstance();
        mgr -> registerMonitor<T>();
    }
};
} // end of annoymous namespace

// Following macro must be used within the atcupdate namespace
#define REGISTER_MONITOR(CLASS)     MonitorRegisterHelper<CLASS> s__monitor_ ## CLASS ## not_impotant_suffix;

} // end of atcupdate namespace
}

#endif