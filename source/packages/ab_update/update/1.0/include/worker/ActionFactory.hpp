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

#ifndef ATC_ACTION_FACTORY_HPP
#define ATC_ACTION_FACTORY_HPP

#include "Action.hpp"
#include "utils/Lock.hpp"
#include "utils/Singleton.hpp"
#include "worker/Worker.hpp"

#include <memory>

namespace atcupdateservice {
namespace worker {

class ActionFactory {
public:
    typedef std::shared_ptr<ActionFactory> ptr;
    virtual Action::ptr create(const std::string &content,
            LocalWorker::ptr worker) const = 0;
};

class ParserFactory : public ActionFactory {
public:
    typedef std::shared_ptr<ParserFactory> ptr;
    virtual Action::ptr create(const std::string &content,
            LocalWorker::ptr worker) const override {
        return std::make_shared<Parser>(content, worker);
    }
};

class OnErrorFactory : public ActionFactory {
public:
    typedef std::shared_ptr<OnErrorFactory> ptr;
    virtual Action::ptr create(const std::string &content,
            LocalWorker::ptr worker) const override {
        return std::make_shared<OnError>(content, worker);
    }
};

class OnFinishedFactory : public ActionFactory {
public:
    typedef std::shared_ptr<OnFinishedFactory> ptr;
    virtual Action::ptr create(const std::string &content,
            LocalWorker::ptr worker) const override {
        return std::make_shared<OnFinished>(content, worker);
    }
};

class OnStartFactory : public ActionFactory {
public:
    typedef std::shared_ptr<OnStartFactory> ptr;
    virtual Action::ptr create(const std::string &content,
            LocalWorker::ptr worker) const override {
        return std::make_shared<OnStart>(content, worker);
    }
};

class ActionFactoryManager {
public:
    typedef std::shared_ptr<ActionFactoryManager> ptr;
    friend class utils::SingletonPtr<ActionFactoryManager>;
    ActionFactory::ptr getFactory(const std::string &name);
    bool registerFactory(const std::string &name, ActionFactory::ptr factory);
    bool deregisterFactory(const std::string &name);
private:
    ActionFactoryManager() {}
    utils::Mutex m_mx;
    std::map<std::string, ActionFactory::ptr> m_factories;
};

typedef utils::SingletonPtr<ActionFactoryManager> ActionFactoryMgr;

class ActionFactoryRegisterHelper {
public:
    ActionFactoryRegisterHelper(const std::string &name, ActionFactory::ptr factory) {
        auto mgr = ActionFactoryMgr::getInstance();
        mgr->registerFactory(name, factory);
    }
};

#define REGISTER_FACTORY(name, CLASS)                                                                       \
     static ActionFactoryRegisterHelper __ ## CLASS ## _abc ## __(name, std::make_shared<CLASS>())

}
}


#endif