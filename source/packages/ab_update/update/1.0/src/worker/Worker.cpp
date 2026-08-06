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
#include "worker/Worker.hpp"
#include "utils/macro.hpp"
#include "worker/ActionFactory.hpp"
#include "worker/Action.hpp"

#define MAX_INTERVAL     16000

namespace atcupdateservice {
namespace worker {

bool IWorker::start() {
    IWorker::ptr self = shared_from_this();

    try {
        m_thread.reset(new std::thread(std::bind(&IWorker::run, self)));
    } catch (std::exception &e) {
        ATCLOGE("exception catch, what() : %s\n", e.what());
        m_thread.reset();
        return false;
    }
    return true;
}

bool LocalWorker::notify(const std::string& actionName, const UpdateMessage &msg) {
    WorkerEvent event;
    event.m_msg = msg;
    ActionFactoryManager::ptr mgr = ActionFactoryMgr::getInstance();
    ActionFactory::ptr factory = mgr->getFactory(actionName);
    Action::ptr action;

    if (msg.m_contents.empty()) {
        return false;
    }
    if (factory == nullptr) {
        ATC_STREAM_LOGE() << "factory is null" << std::endl;
        return false;
    }

    action = factory->create(msg.m_contents[0], self());
    if (action == nullptr) {
        return false;
    }

    event.m_action = action;
    m_que.push(event);
    if (m_sleeper) {
        m_sleeper -> wakeUp();
        return true;
    }

    return false;
}

bool LocalWorker::addAction(Action::ptr action, const std::string &name, bool emerge) {
    WorkerEvent event;

    if (action == nullptr) {
        return false;
    }
    event.m_name = name;
    event.m_action = action;
    event.m_msg.m_type = emerge ? UpdateMessage::EMERGE : UpdateMessage::NORMAL;
    m_que.push(event);
    if (m_sleeper) {
        m_sleeper -> wakeUp();
        return true;
    }

    return true;
}

void IWorker::join() {
    if (m_thread && m_thread->joinable()) {
        m_thread -> join();
    }
}

IWorker::ptr LocalWorker::clone(const std::string &name) const {
    return std::make_shared<LocalWorker>(name);
}

void LocalWorker::run() {
    unsigned interval = 100;
    auto upd = updater::Updater::Upd::getInstance();

    ATC_STREAM_LOGI() << "LocalWorker Activated!" << std::endl;
    while (1) {
        WorkerEvent event;
        while (m_que.top(event)) {
            try {
                if(m_pause && !event.emerge()) {
                    updater::Updater::Upd::getInstance()->sendMessage(UpdateMessageType::PAUSE, "update had paused!");
                    if (interval < MAX_INTERVAL/2) {
                        interval *= 2;
                    }
                    m_que.push(event);
                    m_sleeper->sleepFor(interval);
                    continue;
                }
                interval = 100;
                if(event.m_action) {
                    event.m_action->run();
                } else {
                    ATC_STREAM_LOGE() << "no action!" << std::endl;
                }
            } catch (std::exception &e) {
                ATC_STREAM_LOGE() << "catch exception in action : " << event.m_name
                                  << " what : " << e.what() << std::endl;
                Action::ptr onError = std::make_shared<OnError>(e.what(), self());
                addAction(onError, "OnError", true);
                continue;
            }
        }
        if (interval < MAX_INTERVAL/2) {
            interval *= 2;
        } else if (interval < MAX_INTERVAL) {
            interval += MAX_INTERVAL/10;
        }
        //only if there is no task will here be reached!
        setPathAndStatus("", IWorker::IDLE);
        m_sleeper->sleepFor(interval);
    }
}

}
}