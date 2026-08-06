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
#include <atomic>
#include <memory>
#include <regex>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "monitor/Monitor.hpp"
#include "macro.hpp"
#include "updater/Updater.hpp"

#define     DISKSTATDEV     "/proc/diskstats"

namespace atcupdateservice {
namespace monitor {

const std::regex EMMC_PATTERN("mmcblk0$");
const std::string EMMC("mmcblk0");
const std::regex UDISK_PATTERN("sd[a-z]$");
const std::regex SDCARD_PATTERN("mmcblk[1-9]$");

/*
 * this class is supposed be hidden from user, and it's managed by GuardManager
 * Besides, any guard class who want to be manager by GuardManager should implement the interface IGuard
 * Last of all, GuardManager should be the friend of the inherit class, and the consturctor should be private
 */
class IOMonitor final: public IMonitorItem {
public:
    typedef std::shared_ptr<IOMonitor> ptr;
    IOMonitor();
    unsigned getKey() const override;
    bool loadingStatus() const override;
    double currentLoading() const override;
    bool update() override;
    unsigned weight() const override {
        return 0;
    }
    bool notify(const std::string& actionName, const UpdateMessage &msg);
private:
    struct IOInfo {
        enum Type{
            RD_SECTORS = 0,
            WR_SECTORS,
            DC_SECTORS,
            RD_IOS,
            RD_MERGES,
            WR_IOS,
            WR_MERGES,
            DC_IOS,
            DC_MERGES,
            RD_TICKS,
            WR_TICKS,
            DC_TICKS,
            IOS_PGR,
            TOTO_TICKS,
            RQ_TICKS,
            MAX_CNT
        };
        std::string m_dev;
        unsigned long m_emmc[MAX_CNT];
        unsigned long m_dev[MAX_CNT];
        IOInfo(const std::string &dev, const std::string &uuid);
        bool update();
    };

    IOInfo m_lastLoad;
    IOInfo m_curLoad;
    double wr_external
};

IOInfo::IOInfo()
    : m_dev(dev), m_uuid(uuid) {
}

bool IOMonitor::IOInfo::update() {
    std::string line;
    std::ifstream fin(DISKSTATDEV);

    if (!fin) return false;
    while (getline(fin, line)) {
        unsigned long *target = nullptr;
        std::string dev;
        std::stringstream ss(line);
        ss >> dev;
        if (dev == m_dev) {
            target = m_dev;
        } else if (dev == EMMC) {
            target = m_emmc;
        } else {
            continue;
        }

        for (unsigned i = 0; i < (unsigned)MAX_CNT; i++) {
            if (!(ss >> target[i])) {
                return false;
            }
        }
    }

    return true;
}

IOMonitor::IOMonitor()
    : IMonitorItem("IOMonitor") {
}

unsigned IOMonitor::getKey() const {
    return Key<IOMonitor>::getKey();
}

bool IOMonitor::loadingStatus() const {

    return true;
}

double IOMonitor::currentLoading() const {
    return 0;
}

bool IOMonitor::update() const {
    m_lastLoad = m_curLoad;

    m_curLoad.update();
    return true;
}

//not ready
bool IOMonitor::notify(const UpdateMessage &msg) {
    if (msg.m_type == UpdateMessage::Type::UMOUNTED) {

    } else if (UpdateMessage::Type::MOUNTED == msg.m_type) {
        ;
    } else {
        ;
    }
}

REGISTER_MONITOR(IOMonitor);

}
}