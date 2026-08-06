#ifndef ATC_UPDATE_CLIENT_HPP
#define ATC_UPDATE_CLIENT_HPP

#include <memory>
#include <functional>
#include <string>

#include "UpdateMessageType.hpp"

namespace atcupdateservice {
class ATCUpdateClient {
public:
    typedef std::function<void(UpdateMessageType, const std::string &)> MessageCb;
    typedef std::function<void(uint32_t)> ProgressCb;
    typedef std::shared_ptr<ATCUpdateClient> ptr;
    ~ATCUpdateClient() {}

    static ATCUpdateClient::ptr getInstance();
    bool registerProgressCb(ProgressCb Cb);
    bool registerMessageCb(MessageCb Cb);
    bool checkPackage(std::string path);
    static int currentSlot();
    int startService();
    // only support LocalWorker currently
    // LocalWorker means update from local device like udisk, internal storage, sdcard
    int beginUpdate(const std::string& pathname, const std::string &method = "LocalWorker");
    int cancelUpdate();
    int pauseUpdate();
    int resumeUpdate();
    int getProgress(unsigned &progress);

    LastUpdateStatus getLastStatus();
    std::string getSystemVersion();
    bool checkUpdating();

    //following api is NOT recommand to use
    bool startService(std::string &msg);
    bool beginUpdate(const std::string& pathname, std::string &msg,
                     const std::string &method = "LocalWorker");
    bool cancelUpdate(std::string &msg);
    bool pauseUpdate(std::string &msg);
    bool resumeUpdate(std::string &msg);
private:
    ATCUpdateClient() {}
};

}
#endif