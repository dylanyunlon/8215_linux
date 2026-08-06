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

#include "updater/Updater.hpp"
#include "utils/macro.hpp"
#include "bootctrl/BootCtrl.hpp"
#include "utils/Version.hpp"

#include <thread>

namespace atcupdateservice {
namespace updater {

using namespace bootctrl;
using utils::LockGuard;
using utils::Mutex;

Updater::Updater()
    : m_ipc(new ipc::socket::SocketIPC()) {
}

void Updater::removeProto(const std::string &name) {
    LockGuard<Mutex> lg(m_mx);
    m_workerProtos.erase(name);
}

void Updater::registerProto(const std::string &name,
                            worker::IWorker::ptr proto) {
    LockGuard<Mutex> lg(m_mx);
    m_workerProtos[name + std::string("_Proto")] = proto;
    ATC_STREAM_LOGE() << "register proto : " << name << std::endl;
}

static LastUpdateStatus lastStatus() {
    std::string version = readVersion();

    if (access(CHECKPOINT_PATH, F_OK) == 0)
        return LAST_UNFINISHED;
    if (access(VERSION_OLD, F_OK) != 0) {
        return LAST_NO_UPDATE;
    } else {
        unlink(VERSION_OLD);
        slot_metadata_t slotinfo;
        //std::string suffix;
        memset(&slotinfo, 0, sizeof(slotinfo));
        if (getInactiveSlot(&slotinfo) < 0) {
            if (slotinfo.retry_count == 0) {
                return LAST_FAIL;
            }
        }

        return LAST_UPDATE_OK;
    }
}

void Updater::run() {
    // setup directory
    if (access(CHECKPOINT_PARENT, F_OK) != 0) {
        if (mkdir(CHECKPOINT_PARENT, 0644) < 0) {
            ATC_STREAM_LOGE() << "setup checkpoint base directory failed: " << strerror(errno) << std::endl;
        }
    }
    m_status = lastStatus();
    ATC_STREAM_LOGE() << "last status: " << m_status << std::endl;

    //m_monitor = monitor::MonitorMgr::getInstance();
}

int Updater::startService() {
    static std::once_flag once;
    try {
        std::call_once(once, [this](){
            int rt = 0;
            rt = bootctrl::markBootSuccessful();
            if (rt < 0) {
                throw std::logic_error("failed to call mark Boot Successful");
            }
            m_version = utils::readVersion();
        });
    } catch (std::exception &e) {
        return UPD_EBOOTCTL;
    }

    return 0;
}

bool Updater::checkUpdating() {
    LockGuard<Mutex> lg(m_mx);
    auto iter = m_workers.find("LocalWorker");
    if (iter == m_workers.end()) {
        return false;
    }
    worker::IWorker::ptr worker = iter->second;
    return worker->getStatus() != worker::IWorker::IDLE;
}

int Updater::beginUpdate(const std::string& pathname, const std::string& method) {
    LockGuard<Mutex> lg(m_mx);
    worker::IWorker::ptr worker;
    auto iter = m_workers.find(method);

    if(iter == m_workers.end()) {
        worker::IWorker::ptr proto;
        auto it = m_workerProtos.find(method + std::string("_Proto"));
        if (it == m_workerProtos.end()) {
            ATC_STREAM_LOGE() << "no such prototype" << std::endl;
            return UPD_EBADREQ;
        }
        proto = it->second;
        worker = proto->clone(method);
        worker -> start();
        //first time, start monitor
        //m_monitor -> start();
        m_workers.insert(std::make_pair(method, worker));
    } else {
        worker = iter->second;
    }
#ifdef BOARD_AVB_ENABLE
    //upgrade request would be denied in the first 3 second after atcupdateservice is booted!
    if (utils::getElapseTimeMs() <= 3 * MS_TO_SEC) {
        sendMessage(UpdateMessageType::ERROR, "atcupdateservice is setting up, upgrade denied!");
        return UPD_EBADREQ;
    }
#endif
    if (worker->getStatus() == worker::IWorker::IDLE) {
        worker -> notify("OnStart", UpdateMessage(UpdateMessage::Type::NORMAL, {pathname}));
        m_curWorker = worker;
    } else {
        return UPD_EBUSY;
    }

    return 0;
}

int Updater::cancel() {
    return UPD_ENOOP;
}

int Updater::resume() {
    utils::LockGuard<utils::Mutex> lg(m_mx);
    if (m_curWorker) {
        m_curWorker -> resume();
        return 0;
    } else {
        return UPD_EIDLE;
    }
}

int Updater::pause() {
    utils::LockGuard<utils::Mutex> lg(m_mx);
    if (m_curWorker) {
        m_curWorker -> pause();
        return 0;
    } else {
        return UPD_EIDLE;
    }
}

void Updater::join() {
    //m_monitor->join();
    for (auto &item : m_workers) {
        item.second->join();
    }
}

bool Updater::notifyAll(const std::string &action, bool emerge,
                        const std::vector<std::string> &contents) {
    UpdateMessage::Type type = emerge ? UpdateMessage::EMERGE : UpdateMessage::NORMAL;

    UpdateMessage msg(type, contents);
    for(auto& item : m_workers) {
        item.second->notify(action, msg);
    }
    return true;
}

bool Updater::notifyOne(const std::string &who, const std::string &action, bool emerge,
                        const std::vector<std::string> &contents) {
    UpdateMessage::Type type = emerge ? UpdateMessage::EMERGE : UpdateMessage::NORMAL;
    UpdateMessage msg(type, contents);
    auto iter = m_workers.find(who);

    if (iter == m_workers.end()) {
        ATC_STREAM_LOGW() << "no such worker!" << std::endl;
        return false;
    }
    iter->second->notify(action, msg);

    return true;
}

}
}