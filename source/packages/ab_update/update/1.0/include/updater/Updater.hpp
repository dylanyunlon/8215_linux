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

#ifndef ATC_UPDATER_HPP
#define ATC_UPDATER_HPP

#include <memory>
#include <vector>

//#include "monitor/Monitor.hpp"
#include "Observer.hpp"
#include "utils/Lock.hpp"
#include "worker/Worker.hpp"
#include "UpdateMessageType.hpp"
#include "utils/macro.hpp"
#include "ipc/IPC.hpp"
#include "ipc/socket/SocketRPC.hpp"

namespace atcupdateservice {
namespace updater {

// Updater must be located in heap memory
class Updater : public std::enable_shared_from_this<Updater> {
public:
    friend class ATCUpdateSerivceRPC;
    friend class utils::SingletonPtr<Updater>;
    typedef std::shared_ptr<Updater> ptr;
    typedef utils::SingletonPtr<Updater> Upd;
    ~Updater() {
        join();
    }
    void join();
    void run();
    void removeProto(const std::string &name);
    void registerProto(const std::string &name, worker::IWorker::ptr proto);

    int startService();
    int beginUpdate(const std::string& pathname, const std::string &method = "LocalWorker");

    bool checkUpdating();
    std::string getSystemVersion() const {
        return m_version;
    }
    LastUpdateStatus getLastStatus() const {
        return m_status;
    }

    int resume();
    int pause();
    int cancel();
    uint32_t getProgress() {
        return m_progress;
    }
    bool notifyAll(const std::string &action, bool emerge,
                   const std::vector<std::string> &contents);
    bool notifyOne(const std::string &who, const std::string &action, bool emerge,
                   const std::vector<std::string> &contents);

    void setProgress(uint32_t progress) {
        if (progress > 100) {
            m_progress = 100;
        } else  {
            m_progress = progress;
        }
    }

    void setAndSendProgress(uint32_t progress) {
        setProgress(progress);
        sendProgress();
    }

    void sendProgress() {
        m_ipc -> sendProgress(m_progress);
    }

    void sendMessage(UpdateMessageType type, const std::string &msg) {
        m_ipc -> sendMessage(type, msg);
    }
private:
    Updater();
    uint32_t m_progress = 0;
    utils::Mutex m_mx;

    //monitor::Monitor *m_monitor = nullptr;
    std::unordered_map<std::string, worker::IWorker::ptr> m_workerProtos;
    std::unordered_map<std::string, worker::IWorker::ptr> m_workers;
    // there could be only one active work at once
    worker::IWorker::ptr m_curWorker;
    ipc::IPCBase::ptr m_ipc;
    LastUpdateStatus m_status = LAST_NO_UPDATE;
    std::string m_version;
};

}
}

#endif
