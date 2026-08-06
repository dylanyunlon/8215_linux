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
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "monitor/Monitor.hpp"

#define STATDEV    "/proc/stat"

namespace atcupdateservice {
namespace monitor {

const unsigned long g_limitRate = 300;

/*
 * this class is supposed be hidden from user, and it's managed by MonitorManager
 * Besides, any Monitor class who want to be manager by MonitorManager should implement the interface IMonitor
 * Last of all, MonitorManager should be the friend of the inherit class, and the consturctor should be private
 */
class CPUMonitor final: public IMonitorItem {
public:
    CPUMonitor();
    ~CPUMonitor() {}
    typedef std::shared_ptr<CPUMonitor> ptr;
    unsigned getKey() const override;
    bool loadingStatus() const override;
    double currentLoading() const override;
    bool update() override;
    unsigned weight() const override {
        return m_curLoad.weight();
    }
    //CPUMonitor didn't care
    bool notify(const std::string& actionName, const UpdateMessage &msg) override {
        return true;
    }
private:
    struct CPUInfo {
        enum Type{
            USER = 0,
            NICE,
            SYSTEM,
            IDLE,
            MAXCNT,
        };
        unsigned long m_load[MAXCNT];
        unsigned long getLoadPercentage(const CPUInfo &last);
        unsigned weight() const {
            //fix it later
            return 30;
        }
        bool update();
    };
    CPUInfo m_curLoad;
    CPUInfo m_lastLoad;
    unsigned m_loading;  // fixed number
   // std::vector<unsigned short> m_limits;
};

bool CPUMonitor::CPUInfo::update() {
    std::shared_ptr<char> data(new char[1024], [](char *p){
        delete p;
    });
    char *buf = data.get();
    int fd = open(STATDEV, O_RDONLY);
    int rt = 0;
    std::string header;

    if (fd < 0) {
        ATCLOGE("failed to open %s, errno = %d, error = %s",
                STATDEV, errno, strerror(errno));
        return false;
    }
    rt = read(fd, buf, 1024);
    if (rt < 0) {
        ATCLOGE("failed to read %s, errno = %d, error = %s",
                STATDEV, errno, strerror(errno));
        return false;
    }
    close(fd);
    buf[rt] = '\0';
    std::stringstream ss(buf);
    //take care
    ss >> header >> m_load[USER] >>  m_load[NICE]
       >>  m_load[SYSTEM] >>  m_load[IDLE];
    ATC_STREAM_LOGI() << "header : " << header << " USER :"<< m_load[USER] << " NICE :" <<  m_load[NICE]
       << " SYSTEM :" << m_load[SYSTEM] << " IDLE :"<< m_load[IDLE] << std::endl;
    return true;
}

unsigned long CPUMonitor::CPUInfo::getLoadPercentage(const CPUInfo &last) {
    unsigned long totalDiff = 0;
    unsigned long percentage = 0;
    unsigned long busyDiff = 0;

    for (unsigned i = 0; i < (unsigned)MAXCNT; i++) {
        unsigned long diff = m_load[i] - last.m_load[i];
        if (IDLE != i) {
            busyDiff += diff;
        }
        totalDiff += diff;
    }
    // just in case
    if(totalDiff == 0) {
        return 0;
    }
    percentage = (1000 * (busyDiff) + totalDiff/2) / totalDiff;
    ATC_STREAM_LOGI() << "percentage : " << percentage << std::endl;

    return percentage;
}

CPUMonitor::CPUMonitor()
    : IMonitorItem("CPUMonitor"),
      m_loading(-1) {
}

unsigned CPUMonitor::getKey() const {
    return hash::Key<CPUMonitor>::getKey();
}

bool CPUMonitor::loadingStatus() const {
    return m_loading < g_limitRate;
}

// updateLoading should be called before calling the currentLoading
double CPUMonitor::currentLoading() const {
    return ((double)m_loading) / 10.0;
}

// main function of CPUMonitor
bool CPUMonitor::update() {
    m_lastLoad = m_curLoad;
    if(m_curLoad.update() == false) {
        return false;
    }
    m_loading = m_curLoad.getLoadPercentage(m_lastLoad);

    return true;
}

REGISTER_MONITOR(CPUMonitor);

}
}
