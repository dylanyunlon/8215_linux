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

#ifndef ATC_OBSERVER_HPP
#define ATC_OBSERVER_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "utils/Lock.hpp"

namespace atcupdateservice {

struct UpdateMessage {
    enum Type {
        NORMAL,
        EMERGE,
    };
    Type m_type;
    std::vector<std::string> m_contents;
    void addContent(const std::string &content) {
        m_contents.push_back(content);
    }
    UpdateMessage(Type type = Type::NORMAL,
        const std::vector<std::string> contents = std::vector<std::string>())
        : m_type(type), m_contents(contents) {
    }
};

class IObserver {
public:
    typedef std::shared_ptr<IObserver> ptr;
    virtual bool notify(const std::string& actionName, const UpdateMessage &msg) = 0;
    virtual ~IObserver() {}
};

class ObserverManager {
public:
    typedef std::shared_ptr<ObserverManager> ptr;
    bool notify(const std::string &name, const std::string& action, const UpdateMessage &msg);
    bool notifyAll(const std::string& action, const UpdateMessage &msg);
    bool registerObserver(const std::string &msg, IObserver::ptr observer);

    bool removeObserver(const std::string &name);
private:
    utils::Mutex m_mx;
    std::unordered_map<std::string, IObserver::ptr> m_observers;
};

}

#endif