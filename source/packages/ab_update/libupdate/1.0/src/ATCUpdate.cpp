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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>

#include "ATCUpdate.hpp"
#include <UpdateMessageType.hpp>
#include <utils/macro.hpp>

#include "private/Tag.hpp"

#define    CMDLINE_DEV           "/proc/cmdline"

#define    TARGET_PREFIX         "slot_suffix="
#define    STD_KB                (1024)
#define    BLKSZ                 (4096)

#define ISO_MD5FILE_NAME        "iso.md5"
namespace atcupdateservice {
namespace {
const static std::string g_serviceInstance = "atcupdateservice";
const static std::string g_serivceDomain = "local";
}

ATCUpdate::ATCUpdate() {
    m_conn.reset(new SocketConnection());
    if (m_conn) {
        if (m_conn->start() == false) {
            m_conn.reset();
        }
    }
}

ATCUpdate::ptr ATCUpdate::getInstance() {
    static ATCUpdate::ptr s_instance(new ATCUpdate());
    return s_instance;
}

int ATCUpdate::startService() {
    ProtocolHeader header;
    if (m_conn == nullptr) return -UPD_ECONN;
    ByteArray::ptr ba = m_conn->handleRequest(make_request(START_SERVICE, 0));
    int rt = 0;
    if (ba == nullptr) {
        return -UPD_ECONN;
    }
    if (ba->readBinary((uint8_t*)&header, sizeof(header)) == false) {
        return -UPD_ECONN;
    }
    rt = header.status;

    return rt;
}

int ATCUpdate::beginUpdate(const std::string &pathname, const std::string &method) {
    ProtocolHeader header;
    if (m_conn == nullptr) return -UPD_ECONN;
    ByteArray::ptr ba = m_conn->handleRequest(make_request(BEGIN_UPDATE, 0, pathname, method));
    int rt = 0;
    if (ba == nullptr) {
        return -UPD_ECONN;
    }
    if (ba->readBinary((uint8_t*)&header, sizeof(header)) == false) {
        return -UPD_ECONN;
    }
    rt = header.status;

    return rt;
}

int ATCUpdate::cancelUpdate() {
    return -UPD_ENOOP;
}

int ATCUpdate::pauseUpdate() {
    return -UPD_ENOOP;
}

int ATCUpdate::resumeUpdate() {
    return -UPD_ENOOP;
}

int ATCUpdate::getProgress(unsigned &progress) {
    ProtocolHeader header;
    if (m_conn == nullptr) return -UPD_ECONN;
    ByteArray::ptr ba = m_conn->handleRequest(make_request(GET_PROGRESS, 0));
    int rt = 0;
    if (ba == nullptr) {
        return -UPD_ECONN;
    }
    if (ba->readBinary((uint8_t*)&header, sizeof(header)) == false) {
        return -UPD_ECONN;
    }
    if (ba->read(progress) == false) {
        return -UPD_ECONN;
    }
    rt = header.status;

    return rt;
}

LastUpdateStatus ATCUpdate::getLastStatus() {
    if (m_conn == nullptr) return LAST_UNKNOWN;
    ByteArray::ptr ba = m_conn->handleRequest(make_request(GET_LAST_STATUS, 0));
    uint32_t status = 0;
    if (ba == nullptr) {
        return LAST_UNKNOWN;
    }
    if (ba->seek(sizeof(ProtocolHeader)) == false) {
        return LAST_UNKNOWN;
    }
    if (ba->read(status) == false) {
        return LAST_UNKNOWN;
    }

    return (LastUpdateStatus)status;
}

std::string ATCUpdate::getSystemVersion() {
    std::string version;
    if (m_conn == nullptr) return version;
    ByteArray::ptr ba = m_conn->handleRequest(make_request(GET_SYSTEM_VERSION, 0));
    if (ba == nullptr) {
        return version;
    }
    if (ba->seek(sizeof(ProtocolHeader)) == false) {
        return version;
    }
    if (ba->read(version) == false) {
        return version;
    }

    return version;
}

bool ATCUpdate::checkUpdating() {
    if (m_conn == nullptr) return false;
    ByteArray::ptr ba = m_conn->handleRequest(make_request(CHECK_UPDATING, 0));
    uint32_t status = 0;
    if (ba == nullptr) {
        return false;
    }
    if (ba->seek(sizeof(ProtocolHeader)) == false) {
        return false;
    }
    if (ba->read(status) == false) {
        return false;
    }

    return !!status;
}

bool ATCUpdate::registerProgressCb(ProgressCb cb) {
    if (cb && m_conn) {
        m_conn->registerBroadcastHandler(SEND_PROGRESS, [cb](ByteArray::ptr ba){
            uint32_t progress = 0;
            if(ba->seek(sizeof(ProtocolHeader)) && ba->read(progress)) {
                try {
                    cb(progress);
                } catch (...) {
                    return;
                }
            }
        });
        return true;
    }
    return false;
}

bool ATCUpdate::registerMessageCb(MessageCb cb) {
    if (cb && m_conn) {
        m_conn->registerBroadcastHandler(SEND_MESSAGE, [cb](ByteArray::ptr ba){
            uint32_t type;
            std::string msg;
            if (ba->seek(sizeof(ProtocolHeader)) &&
                ba->read(type) && ba->read(msg))
                try {
                    cb((UpdateMessageType)type, msg);
                } catch (...) {
                    return;
                }
        });
        return true;
    }
    return false;
}

static std::string getSuffix() {
    char buf[BLKSZ] = {0};
    int fd = open(CMDLINE_DEV, O_RDONLY);
    int curPos = 0;
    int rt = 0;
    std::stringstream ss;
    std::string res = "";
    std::string item = "";
    size_t targetLen = strlen(TARGET_PREFIX);
    if (fd < 0) {
        syslog(LOG_ERR, "failed to open dev : %s, error : %s\r\n", CMDLINE_DEV, strerror(errno));
        return res;
    }

    while ((rt = read(fd, buf + curPos, BLKSZ)) > 0) {
        curPos += rt;
    }
    close(fd);
    if (rt < 0) {
        syslog(LOG_ERR, "failed to open dev : %s, error : %s\r\n", CMDLINE_DEV, strerror(errno));
        return res;
    }
    buf[curPos] = '\0';
    ss.str(buf);
    while (ss >> item) {
        std::cout << "item : " << item << std::endl;
        if (strncmp(item.c_str(), TARGET_PREFIX, targetLen) == 0) {
            res = item.substr(targetLen);
            if (res == "_a" || res == "_b"){
                return res;
            } else {
                syslog(LOG_ERR, "suffix(%s) not valid\r\n", res.c_str());
                return "";
            }
        }
    }
    syslog(LOG_INFO, "not valid suffix\r\n");
    return "";
}

int ATCUpdate::currentSlot() {
    std::string suffix = getSuffix();
    if (suffix == std::string("_a")) {
        return 0;
    } else if (suffix == std::string("_b")) {
        return 1;
    } else {
        return -1;
    }
}

bool ATCUpdate::checkPackage(std::string path) {
    //check package existance
    std::string filename = path;
    std::string isoPath;
    std::string md5Str, name;

    if (path.empty() || access(path.c_str(), F_OK) < 0) {
        syslog(LOG_ERR, "iso package %s not exist!\n", path.c_str());
        return false;
    }
    //check iso.md5 existance
    for (int i = path.size() - 1; i >= 0; --i) {
        if (path[i] == '/') {
            //remove all the character after '/'
            filename = path.substr(i+1);
            path.resize(i+1);
            break;
        }
    }
    syslog(LOG_ERR, "iso package name : %s, path : %s\n", filename.c_str(), path.c_str());
    if (path.empty()) {
        return false;
    }
    isoPath = path + ISO_MD5FILE_NAME;
    if (access(isoPath.c_str(), F_OK)) {
        syslog(LOG_ERR, "iso.md5 : %s not exist!\n", isoPath.c_str());
        return false;
    }
    std::ifstream fin(isoPath);
    fin >> md5Str >> name;

    return (filename == name);
}

bool ATCUpdate::beginUpdate(const std::string& pathname, std::string &msg, const std::string &method) {
    int status = beginUpdate(pathname, method);
    msg = UpdErrorStr((UpdErrorType)-status);
    return status == 0;
}

bool ATCUpdate::cancelUpdate(std::string &msg) {
    int status = cancelUpdate();
    msg = UpdErrorStr((UpdErrorType)-status);
    return status == 0;
}

bool ATCUpdate::pauseUpdate(std::string &msg) {
    int status = pauseUpdate();
    msg = UpdErrorStr((UpdErrorType)-status);
    return status == 0;
}

bool ATCUpdate::resumeUpdate(std::string &msg) {
    int status = resumeUpdate();
    msg = UpdErrorStr((UpdErrorType)-status);
    return status == 0;
}

bool ATCUpdate::startService(std::string &msg) {
    int status = startService();
    msg = UpdErrorStr((UpdErrorType)-status);
    return status == 0;
}

}