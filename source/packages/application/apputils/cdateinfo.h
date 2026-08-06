/*
* Copyright (c) 2016 AutoChips Inc.
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
 
#ifndef CDATEINFO_H
#define CDATEINFO_H

#include "sys/time.h"
#include "time.h"
#include "unistd.h"

//static const char TAG[] = "CDateInfo";

class CDateInfo
{
public:
    time_t m_timeInfo;
    struct tm * m_currentDate;

    CDateInfo()
    {
        m_currentDate = new tm();
    }

    void UpdateTime(void)
    {
        time(&m_timeInfo);
        m_currentDate = localtime(&m_timeInfo);
    }

    void SetSysTime_Year(int year)
    {
        m_currentDate->tm_year = year;
    }

    void SetSysTime_Hour(int hour)
    {
        m_currentDate->tm_hour = hour;
    }

    void SetSysTime_Mon(int mon)
    {
        m_currentDate->tm_mon = (mon - 1);
    }

    void SetSysTime_Min(int min)
    {
        m_currentDate->tm_min = min;
    }

    void SetSysTime_Day(int day)
    {
        m_currentDate->tm_mday = day;
    }

    void SetSysTime_Sec(int sec)
    {
        m_currentDate->tm_sec = sec;
    }

    bool SetTime()
    {
        struct timeval * tmp_time;
        bool bRet;
        tmp_time = new timeval();
        m_timeInfo = mktime(m_currentDate);
        tmp_time->tv_sec = m_timeInfo;
        tmp_time->tv_usec = 0;
        if (settimeofday(tmp_time, (const struct timezone *) 0) < 0) {
            bRet = false;
        } else {
            time(&m_timeInfo);
            m_currentDate = localtime(&m_timeInfo);
            bRet = true;
        }

        return bRet;
    }

    int GetDate_Year()
    {
        return (m_currentDate->tm_year);
    }

    int GetDate_Mon()
    {
        return (m_currentDate->tm_mon + 1);
    }

    int GetDate_Hour()
    {
        return m_currentDate->tm_hour;
    }

    int GetDate_Min()
    {
        return m_currentDate->tm_min;
    }

    int GetDate_Day()
    {
        return m_currentDate->tm_mday;
    }

    int GetDate_Sec()
    {
        return m_currentDate->tm_sec;
    }

    ~CDateInfo()
    {
        SAFE_DELETE(m_currentDate);
    }
};
#endif

