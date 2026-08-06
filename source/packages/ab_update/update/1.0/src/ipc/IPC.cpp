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

#include <fstream>

#include "ipc/IPC.hpp"
#include "updater/Updater.hpp"

namespace atcupdateservice {
namespace ipc {

int ATCUpdateServiceRPC::startService() {
    auto updater = updater::Updater::Upd::getInstance();

    return updater -> startService();
}

int ATCUpdateServiceRPC::beginUpdate(const std::string& pathname, const std::string &method ) {
    auto updater = updater::Updater::Upd::getInstance();

    return updater -> beginUpdate(pathname, method);
}

/* not support currently! */
int ATCUpdateServiceRPC::cancelUpdate() {
    auto updater = updater::Updater::Upd::getInstance();

    return updater -> cancel();
}

/* not support currently! */
int ATCUpdateServiceRPC::pauseUpdate() {
    auto updater = updater::Updater::Upd::getInstance();

    return updater -> pause();
}

/* not support currently! */
int ATCUpdateServiceRPC::resumeUpdate() {
    auto updater = updater::Updater::Upd::getInstance();

    return updater -> resume();
}

int ATCUpdateServiceRPC::getProgress(unsigned &progress) {
    auto updater = updater::Updater::Upd::getInstance();

    progress = updater -> getProgress();
    if (progress <= 0 || progress >= 100) {
        return UPD_EIDLE;
    }
    return 0;
}

ATCUpdateServiceRPC::ptr ATCUpdateServiceRPC::getInstance() {
    ATCUpdateServiceRPC::ptr rpc
        = utils::SingletonPtr<ATCUpdateServiceRPC>::getInstance();
    return rpc;
}

LastUpdateStatus ATCUpdateServiceRPC::getLastStatus() {
    auto updater = updater::Updater::Upd::getInstance();
    return updater->getLastStatus();
}

bool ATCUpdateServiceRPC::checkUpdating() {
    auto updater = updater::Updater::Upd::getInstance();
    return updater->checkUpdating();
}

std::string ATCUpdateServiceRPC::getSystemVersion() {
    auto updater = updater::Updater::Upd::getInstance();
    return updater->getSystemVersion();
}

}
}