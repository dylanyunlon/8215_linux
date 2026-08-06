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

#include "worker/ActionFactory.hpp"

namespace atcupdateservice {
namespace worker {   

ActionFactory::ptr ActionFactoryManager::getFactory(const std::string &name) {
    auto it = m_factories.find(name);
    if (it == m_factories.end()) {
        return nullptr;
    }
    return it->second;
}
bool ActionFactoryManager::registerFactory(const std::string &name,
                                            ActionFactory::ptr factory) {
    auto it = m_factories.find(name);
    if (it != m_factories.end()) {
        return false;
    }
    m_factories[name] = factory;
    return true;
}
bool ActionFactoryManager::deregisterFactory(const std::string &name) {
    auto it = m_factories.find(name);
    if (it == m_factories.end()) {
        return false;
    }
    m_factories.erase(name);

    return true;
}

REGISTER_FACTORY("Parser", ParserFactory);
REGISTER_FACTORY("OnError", OnErrorFactory);
REGISTER_FACTORY("OnFinished", OnFinishedFactory);
REGISTER_FACTORY("OnStart", OnStartFactory);

}
}
