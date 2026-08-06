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

#ifndef ATC_SLEEPER_HPP
#define ATC_SLEEPER_HPP

#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <string.h>
#include <exception>

#include "utils/Util.hpp"

namespace atcupdateservice {
namespace utils {

#define IDLE_MESSAGE    ""
class Sleeper {
public:
    typedef std::shared_ptr<Sleeper> ptr;
    Sleeper() {
        int rt = pipe(m_notifier);
        if (rt != 0) {
            ATCLOGE("failure to create pipe rt : %d, errno : %d , error : %s\n", rt, errno, strerror(errno));
            throw std::logic_error("create pipe failure!");
        }
        rt = setNonb(m_notifier[0]);
        if (rt != 0) {
            ATCLOGE("failure to set nonb rt : %d errno : %d , error : %s\n", rt, errno, strerror(errno));
            throw std::logic_error("create set nonb!");
        }
        rt = setNonb(m_notifier[1]);
        if (rt != 0) {
            ATCLOGE("failure to create pipe rt : %d, errno : %d , error : %s\n",rt, errno, strerror(errno));
            throw std::logic_error("create pipe failure!");
        }
    }

    bool wakeUp() {
        int rt = 0;

        rt = write(m_notifier[1], "", 1);
        if (rt < 0 && (rt != EWOULDBLOCK || rt != EAGAIN)) {
            return false;
        }
        return true;
    }

    bool sleepFor(long ms) {
        fd_set rdset;
        int rt = 0;
        char buf[1024];
        timeval tv;
        memset(&tv, 0, sizeof(tv));
        tv.tv_sec = ms / 1000;
        tv.tv_usec = (ms % 1000 ) * 1000;
        FD_ZERO(&rdset);
        FD_SET(m_notifier[0], &rdset);
        do {
            rt = select(m_notifier[0] + 1, &rdset, NULL, NULL, &tv);
            if (rt < 0) {
                if (errno == EINTR) {
                    continue;
                }
                return false;
            }
        } while(0);
        while (1) {
            // just clear the buffer of the pipe, read util it return -1
            rt = read(m_notifier[0], buf, 1024);
            if (rt < 0) {
                // this is what we expect
                if (rt == EWOULDBLOCK || rt == EAGAIN) {
                    break;
                } else {
                    return false;
                }
            }
        }

        return true;
    }

    ~Sleeper() {
        close(m_notifier[0]);
        close(m_notifier[1]);
    }

private:
    int setNonb(int fd) {
        int flags = fcntl(fd, F_GETFL);
        if (flags < 0) {
            return -1;
        }
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
private:
    int m_notifier[2] = {-1, -1};
};

}
}

#endif