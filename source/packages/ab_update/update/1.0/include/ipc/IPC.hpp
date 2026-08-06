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

#ifndef ATC_IPC_BASE_HPP
#define ATC_IPC_BASE_HPP

#include "utils/Singleton.hpp"
#include "Observer.hpp"
#include "UpdateMessageType.hpp"

namespace atcupdateservice {
namespace ipc {

/* this is provided for Updater */
class IPCBase {
public:
    typedef std::shared_ptr<IPCBase> ptr;
    virtual bool sendMessage(UpdateMessageType action, const std::string &msg) = 0;
    virtual bool sendProgress(unsigned progress) = 0;
};

/* provide for concrete IPC method */
class ATCUpdateServiceRPC {
public:
    typedef std::shared_ptr<ATCUpdateServiceRPC> ptr;
    friend class utils::SingletonPtr<ATCUpdateServiceRPC>;

    static ATCUpdateServiceRPC::ptr getInstance();
    int startService();
    int beginUpdate(const std::string &pathname, const std::string &method = "LocalWorker");
    int cancelUpdate();
    int pauseUpdate();
    int resumeUpdate();
    int getProgress(unsigned &progress);
    LastUpdateStatus getLastStatus();
    std::string getSystemVersion();
    bool checkUpdating();
private:
    ATCUpdateServiceRPC() {}
};

}
}

#endif