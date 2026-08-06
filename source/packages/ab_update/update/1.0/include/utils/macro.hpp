/*
copyright (c) 2018 AutoChips Inc.
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

#ifndef ATC_LOG_HPP
#define ATC_LOG_HPP

#include <syslog.h>
#include <string.h>
#include <sstream>
#include <map>
#include <fstream>
#include <iostream>

#define CHECKPOINT_PATH     "/data/misc/checkpoint"
#define CHECKPOINT_PARENT   "/data/misc/"

#define VERSION_OLD         "/data/misc/version_old"
#define VERSION_FILE        "/etc/version"

#define ATC_ISO_MOUNTPOINT      "/data/misc/mnt/"

#define LOG_FILE    "/data/abLog.txt"


#ifdef OEM_PUBK
#undef OEM_PUBK
#endif

#ifndef OEM_PUBK
#define OEM_PUBK   0xF0, 0x7F, 0x11, 0x63, 0xAC, 0xDA, 0xAF, 0xEB, 0x57, 0x91, 0xD1, 0xA4, 0x0D, 0x26, 0xBD, 0x1B, \
                   0x99, 0x15, 0x89, 0xFB, 0xE3, 0x9C, 0x8F, 0x51, 0x21, 0xA7, 0x07, 0x67, 0x94, 0x08, 0x6A, 0xD6, \
                   0x66, 0x45, 0x3E, 0x28, 0x72, 0x64, 0x3C, 0xD8, 0xBF, 0xF9, 0xCE, 0x44, 0x5F, 0xEB, 0x4A, 0x86, \
                   0x90, 0x36, 0xFE, 0xA0, 0x47, 0x24, 0x14, 0xC1, 0x72, 0xEF, 0xE7, 0xB8, 0x0E, 0x9B, 0x7C, 0xF8, \
                   0x48, 0xCA, 0xDE, 0xC7, 0x5D, 0x3D, 0x44, 0xFB, 0x2F, 0x05, 0x9B, 0x14, 0xC7, 0x92, 0xD8, 0x1E, \
                   0x4F, 0x96, 0x77, 0xC3, 0xC6, 0x40, 0x27, 0x27, 0xE8, 0x3F, 0xED, 0x95, 0x44, 0xD9, 0xD2, 0xC6, \
                   0x22, 0xC9, 0x7D, 0xBB, 0xBE, 0xEB, 0x19, 0xEB, 0xC5, 0x8F, 0xBB, 0x4A, 0x47, 0x82, 0x64, 0x12, \
                   0xDA, 0xF8, 0x9D, 0x27, 0x1C, 0xC0, 0x48, 0xD1, 0x17, 0xBA, 0x60, 0xA0, 0xB7, 0x5F, 0x89, 0xAA, \
                   0x37, 0xB3, 0xF9, 0xCF, 0xF9, 0xFD, 0x77, 0x59, 0xF5, 0x20, 0x09, 0x00, 0xB7, 0x47, 0xE2, 0x25, \
                   0x2E, 0xC2, 0xF8, 0x3E, 0x1D, 0xF3, 0x68, 0x13, 0xED, 0x56, 0xCD, 0x2F, 0x7B, 0x87, 0x78, 0x42, \
                   0x42, 0x7B, 0xE2, 0xC3, 0xA9, 0x88, 0x50, 0xE1, 0x66, 0x3A, 0x12, 0x85, 0x89, 0x93, 0x2E, 0x95, \
                   0x37, 0xD7, 0x25, 0x44, 0xB6, 0x33, 0xA6, 0xFC, 0x28, 0x4C, 0x52, 0xD9, 0x9E, 0x5D, 0xF2, 0x87, \
                   0x78, 0x18, 0x7D, 0x51, 0xE0, 0xF1, 0xD1, 0x13, 0x99, 0xB2, 0x3C, 0xDF, 0x77, 0xFF, 0x69, 0x54, \
                   0xBB, 0xF1, 0xFF, 0x38, 0x55, 0x7A, 0x91, 0x69, 0x0C, 0xC2, 0x3C, 0x58, 0x99, 0x70, 0x4D, 0xE5, \
                   0x91, 0x67, 0xFB, 0xDD, 0x01, 0x4E, 0x1C, 0xCA, 0x06, 0x58, 0xFC, 0x7C, 0x57, 0xF5, 0xB7, 0x4A, \
                   0xF5, 0x5C, 0xE0, 0x24, 0x70, 0xFE, 0x8D, 0x85, 0xB0, 0x42, 0x1C, 0x2E, 0x13, 0x1C, 0x76, 0xA5
#endif

#define TAG "[UpdateService 1.3.0]"

#define ATCLOGD(format, ...)    syslog(LOG_DEBUG,   TAG "[D]" format, ##__VA_ARGS__)
#define ATCLOGI(format, ...)    syslog(LOG_INFO,    TAG "[I]" format, ##__VA_ARGS__)
#define ATCLOGN(format, ...)    syslog(LOG_NOTICE,  TAG "[N]" format, ##__VA_ARGS__)
#define ATCLOGW(format, ...)    syslog(LOG_WARNING, TAG "[W]" format, ##__VA_ARGS__)
#define ATCLOGE(format, ...)    syslog(LOG_ERR,     TAG "[E]" format, ##__VA_ARGS__)
#define ATCLOGC(format, ...)    syslog(LOG_CRIT,    TAG "[C]" format, ##__VA_ARGS__)
#define ATCLOGA(format, ...)    syslog(LOG_ALERT,   TAG "[A]" format, ##__VA_ARGS__)
#define ATCLOGM(format, ...)    syslog(LOG_EMERG,   TAG "[M]" format, ##__VA_ARGS__)

#define ATCLOGD_IF(cond, format, ...)                               \
        if ((cond))                                                 \
            ATCLOGD(format, ##__VA_ARGS__)
#define ATCLOGI_IF(cond, format, ...)                                     \
        if ((cond))                                                 \
            ATCLOGI(format, ##__VA_ARGS__)
#define ATCLOGN_IF(cond, format, ...)                                     \
        if ((cond))                                                 \
            ATCLOGN(format, ##__VA_ARGS__)
#define ATCLOGW_IF(cond, format, ...)                                     \
        if ((cond))                                                 \
            ATCLOGW(format, ##__VA_ARGS__)
#define ATCLOGE_IF(cond, format, ...)                                     \
        if ((cond))                                                 \
            ATCLOGE(format, ##__VA_ARGS__)
#define ATCLOGC_IF(cond, format, ...)                                     \
        if ((cond))                                                 \
            ATCLOGC(format, ##__VA_ARGS__)
#define ATCLOGA_IF(cond, format, ...)                                     \
        if ((cond))                                                 \
            ATCLOGA(format, ##__VA_ARGS__)
#define ATCLOGM_IF(cond, format, ...)                                     \
        if ((cond))                                                 \
            ATCLOGM(format, ##__VA_ARGS__)

struct LogWrapper {
public:
    LogWrapper(int prio, const char *tag)
        : m_prio(prio) {
        if (m_prio <= LOG_EMERG)
            m_prio = LOG_EMERG;
        if (m_prio >= LOG_DEBUG)
            m_prio = LOG_DEBUG;

        m_ss << tag << getPrioMap()[m_prio] << " ";
    }
    std::stringstream &getSS() {
        return m_ss;
    }
    ~LogWrapper() {
        std::ofstream fout(LOG_FILE, std::ios::app);
        //syslog(m_prio, "%s", m_ss.str().c_str());
        fout << m_ss.str();
        std::cout << m_ss.str();
        fout.flush();
    }
private:
    std::stringstream m_ss;
    int m_prio;
    static std::map<int, std::string> &getPrioMap() {
        static std::map<int, std::string> mp = {
            {LOG_DEBUG,  "[D]"},
            {LOG_INFO,   "[I]"},
            {LOG_NOTICE, "[N]"},
            {LOG_WARNING,"[W]"},
            {LOG_ERR,    "[E]"},
            {LOG_CRIT,   "[C]"},
            {LOG_ALERT,  "[A]"},
            {LOG_EMERG,  "[M]"},
        };

        return mp;
    }
};


#define ATC_STREAM_LOG(prio) LogWrapper(prio, TAG).getSS()

#define ATC_STREAM_LOGD() ATC_STREAM_LOG(LOG_DEBUG)
#define ATC_STREAM_LOGI() ATC_STREAM_LOG(LOG_INFO)
#define ATC_STREAM_LOGN() ATC_STREAM_LOG(LOG_NOTICE)
#define ATC_STREAM_LOGW() ATC_STREAM_LOG(LOG_WARNING)
#define ATC_STREAM_LOGE() ATC_STREAM_LOG(LOG_ERR)
#define ATC_STREAM_LOGC() ATC_STREAM_LOG(LOG_CRIT)
#define ATC_STREAM_LOGA() ATC_STREAM_LOG(LOG_ALERT)
#define ATC_STREAM_LOGM() ATC_STREAM_LOG(LOG_EMERG)

#endif
