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

#ifndef ATC_WORKER_HPP
#define ATC_WORKER_HPP

#include <thread>
#include <string>
#include <memory>
#include <unordered_map>
#include <queue>
#include <functional>
#include <atomic>

#include "Observer.hpp"
#include "utils/TSQueue.hpp"
#include "utils/Sleeper.hpp"
#include "worker/Action.hpp"

using namespace atcupdateservice::utils;
namespace atcupdateservice {
namespace worker {

struct WorkerEvent {
    UpdateMessage m_msg;
    std::string m_name = "Parser";
    unsigned long m_size = 0;
    Action::ptr m_action;
    bool operator<(const WorkerEvent &o) const {
        return m_msg.m_type < o.m_msg.m_type;
    }
    UpdateMessage::Type getType() {
        return m_msg.m_type;
    }
    bool emerge() {
        return (uint32_t)m_msg.m_type  == UpdateMessage::EMERGE;
    }
};

class IWorker : public std::enable_shared_from_this<IWorker> {
public:
    enum Status {
        IDLE,
        WORKING,
        CHECKING,
        ERROR,
        UMOUNTED,
        FINISHED,
        WAITING,
    };
    typedef std::shared_ptr<IWorker> ptr;
    IWorker(const std::string &name)
        : m_name(name),
          m_status{(uint32_t)Status::IDLE},
          m_resourceStatus(false){
    }

    uint32_t getStatus() {
        return m_status;
    }
    virtual ~IWorker() {
        join();
    }
    virtual bool notify(const std::string& actionName, const UpdateMessage &msg) = 0;
    virtual IWorker::ptr clone(const std::string &name) const = 0;
    virtual bool start();
    void join();
    virtual bool getResourceStatus() {
        return m_resourceStatus;
    }
    virtual void setResourceStatus(bool stat) {
        m_resourceStatus = stat;
    }

    const std::string &getWorkerName()   {
        return m_name;
    }
    virtual bool pause() = 0;
    virtual bool resume() = 0;
    void setStatus(Status status) {
        LockGuard<Mutex> lg(m_mx);
        m_status = (uint32_t)status;
    }

    void setPathAndStatus(const std::string &curPath, Status status) {
        LockGuard<Mutex> lg(m_mx);
        m_status = (uint32_t)status;
        m_curPath = curPath;
    }
protected:
    virtual void run() = 0;

    Mutex m_mx;
    std::shared_ptr<std::thread> m_thread;
    std::string m_name;
    std::atomic<uint32_t> m_status;
    std::string m_curPath;
    bool m_resourceStatus = false;
};

class LocalWorker final : public IWorker {
public:
    typedef std::shared_ptr<LocalWorker> ptr;
    LocalWorker(const std::string& name = "LocalWorker")
        : IWorker(name),
          m_sleeper(new utils::Sleeper()),
          m_pause{false} {
    }
    ~LocalWorker() {
    }
    IWorker::ptr clone(const std::string &name) const override;
    bool notify(const std::string& actionName, const UpdateMessage &msg) override;
    bool addAction(Action::ptr action, const std::string &name = "Parser", bool emerge = false);
    void handleUmounted(const std::string &action, const std::string &mountPath);
    LocalWorker::ptr self() {
        return std::dynamic_pointer_cast<LocalWorker>(shared_from_this());
    }
    bool pause() {
        m_pause = true;
        return true;
    }
    bool resume() {
        m_pause = false;
        m_sleeper -> wakeUp();
        return true;
    }
protected:
    void run()  override;
private:
    utils::PriorityTSQueue<WorkerEvent> m_que;
    utils::Sleeper::ptr m_sleeper;
    Parser::ptr m_parser;
    std::atomic_bool m_pause;
};

}
}

#endif