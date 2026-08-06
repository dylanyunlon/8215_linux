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

#ifndef ATC_UPDATE_HPP
#define ATC_UPDATE_HPP

#include <memory>
#include <functional>
#include <string>

#include "ipc/socket/SocketProto.hpp"
#include "SocketClient.hpp"
#include "UpdateMessageType.hpp"

namespace atcupdateservice {
class ATCUpdate {
public:
    typedef std::function<void(UpdateMessageType, const std::string &)> MessageCb;
    typedef std::function<void(uint32_t)> ProgressCb;
    typedef std::shared_ptr<ATCUpdate> ptr;
    ~ATCUpdate() {}

    static ATCUpdate::ptr getInstance();
    int startService();
    // only support LocalWorker currently
    // LocalWorker means update from local device like udisk, internal storage, sdcard
    int beginUpdate(const std::string& pathname, const std::string &method = "LocalWorker");
    int cancelUpdate();
    int pauseUpdate();
    int resumeUpdate();
    int getProgress(unsigned &progress);

    bool registerProgressCb(ProgressCb Cb);
    bool registerMessageCb(MessageCb Cb);
    bool checkPackage(std::string path);

    LastUpdateStatus getLastStatus();
    std::string getSystemVersion();
    bool checkUpdating();

    /*
     * get current boot slot, 0 for a and 1 for b,-1 for failure
     */

    static int currentSlot();
    //following api is NOT recommand to use
    bool beginUpdate(const std::string& pathname, std::string &msg,
                     const std::string &method = "LocalWorker");
    bool cancelUpdate(std::string &msg);
    bool pauseUpdate(std::string &msg);
    bool resumeUpdate(std::string &msg);
    bool startService(std::string &msg);
private:
    ATCUpdate();
    SocketConnection::ptr m_conn;
};

}
#endif // end of ATCUpdate