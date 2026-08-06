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

#ifndef ATC_ACTION_HPP
#define ATC_ACTION_HPP

#include "utils/macro.hpp"

#include <map>
#include "utils/Util.hpp"
#include "UpdateMessageType.hpp"
#include "utils/Image.hpp"
#include "utils/File.hpp"

namespace atcupdateservice {
namespace worker {

class LocalWorker;

class Action {
public:
    typedef std::shared_ptr<Action> ptr;
    Action(std::shared_ptr<LocalWorker> worker)
        : m_worker(worker){
    }
    void sendMessage(UpdateMessageType type, const std::string &msg);
    void setProgress(uint32_t progress);
    void setAndSendProgress(uint32_t progress);
    virtual bool run() = 0;
    virtual ~Action() {}
protected:
    std::shared_ptr<LocalWorker> m_worker;
};

class Parser :public Action, public std::enable_shared_from_this<Parser> {
public:
    typedef std::shared_ptr<Parser> ptr;
    Parser(const std::string &iosPath, std::shared_ptr<LocalWorker> worker)
        : Action(worker), m_curIdx(0), m_iosPath(iosPath), m_firstTime(true){
    }
    ~Parser() {
        if (m_buf != nullptr) {
            ATC_STREAM_LOGE() << "free m_buf" << std::endl;
            free(m_buf);
            m_buf = nullptr;
        }
        ATC_STREAM_LOGE() << "Parser Destroyed!" << std::endl;
    }
    bool run() override;
    virtual bool doCheck(utils::CheckPoint &cp);
    void setMd5(const std::string &md5) {
        m_md5Sum = md5;
    }
    int findImage(const std::string &img);
    void setParts(const std::vector<utils::PartInfo::ptr>& parts) {
        m_parts = parts;
    }
    void setBuffer(char *buf) {
        m_buf = buf;
    }
protected:
    bool validImage(const std::string &image);

    uint64_t m_payLoad = 0;
    uint64_t m_totLoad = 0;
    std::vector<utils::PartInfo::ptr> m_parts;
    //Image::ptr m_image;
    int m_curIdx = 0;
    std::string m_iosPath;
    //TODO : delete it
    bool m_firstTime;
    std::string m_md5Sum;
    utils::Image::ptr m_image;
    utils::File::ptr m_part;
    utils::File::ptr m_part_otherslot;
    char *m_buf = nullptr;
};
class ParserDiff :public Parser {
public:
    typedef std::shared_ptr<ParserDiff> ptr;
    ParserDiff(const std::string &iosPath, std::shared_ptr<LocalWorker> worker)
        : Parser(iosPath, worker) {
    }
    ~ParserDiff() {
        ATC_STREAM_LOGE() << "ParserDiff Destroyed!" << std::endl;
    }
    bool run() override;

};

class OnFinished : public Action, public std::enable_shared_from_this<OnFinished> {
public:
    typedef std::shared_ptr<OnFinished> ptr;
    OnFinished(const std::string &msg, std::shared_ptr<LocalWorker> worker)
        : Action(worker), m_msg(msg) {
    }
    bool run() override;
private:
    std::string m_msg;
};

class OnError : public Action, public std::enable_shared_from_this<OnError> {
public:
    typedef std::shared_ptr<OnError> ptr;
    OnError(const std::string &reason, std::shared_ptr<LocalWorker> worker)
        : Action(worker), m_reason(reason) {
    }
    bool run() override;
private:
    std::string m_reason;
};

class OnStart : public Action, public std::enable_shared_from_this<OnStart> {
public:
    OnStart(const std::string &isoPath, std::shared_ptr<LocalWorker> worker)
        : Action(worker), m_isoPath(isoPath) {
    }
    bool run() override;
private:
    std::string m_isoPath;
};


class OnPause : public Action, public std::enable_shared_from_this<OnStart> {
public:
    OnPause(const std::string &msg, std::shared_ptr<LocalWorker> worker)
        : Action(worker) {
        (std::string)msg;
    }
    bool run() override;
};

class OnResume : public Action, public std::enable_shared_from_this<OnStart> {
public:
    OnResume(const std::string &msg, std::shared_ptr<LocalWorker> worker)
        : Action(worker) {
        (std::string)msg;
    }
    bool run() override;
};

}
}

#endif