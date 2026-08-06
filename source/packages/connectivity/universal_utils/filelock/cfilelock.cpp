/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "clog.h"
#include "cfilelock.h"

namespace universal_utils {

const static char *TAG = "CFileLock";
// #define OPEN_DEBUG_LOG

CFileLock::CFileLock() :
    m_fd(-1)
{

}

CFileLock::CFileLock(std::string file) :
    m_file(file),
    m_fd(-1),
    m_lockerPid(-1)
{

}

CFileLock::~CFileLock()
{
    deinitLock();
}

bool CFileLock::initLock()
{
    if (m_fd < 0) {
        m_fd = open(m_file.c_str(), O_RDWR | O_CREAT, 0666);
#ifdef OPEN_DEBUG_LOG
        UTILS_LOGI(TAG, "initLock open, fd = %d", m_fd);
#endif
        if (m_fd < 0) {
            UTILS_LOGE(TAG, "initLock error = %d", errno);
            return false;
        }
    } else {
#ifdef OPEN_DEBUG_LOG
        UTILS_LOGI(TAG, "already initLock, fd = %d", m_fd);
#endif
    }

    return true;
}

bool CFileLock::initLock(const std::string file)
{
    m_file = file;
    return initLock();
}

bool CFileLock::deinitLock()
{
    if (m_fd > 0) {
        close(m_fd);
        m_fd = -1;
    }

    return true;
}


bool CFileLock::lockForWrite()
{
    if (m_fd < 0) {
        return false;
    }
#ifdef OPEN_DEBUG_LOG
    UTILS_LOGI(TAG, "lockForWrite");
#endif
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(m_fd, F_SETLK, &lock) < 0) {
        UTILS_LOGE(TAG, "lockForWrite fail = %d", errno);
        setErrno(errno);
        return false;
    }

    return writePid();
}

bool CFileLock::lockForWriteWait()
{
    if (m_fd < 0) {
        return false;
    }
#ifdef OPEN_DEBUG_LOG
    UTILS_LOGI(TAG, "lockForWriteWait");
#endif
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(m_fd, F_SETLKW, &lock) < 0) {
        UTILS_LOGE(TAG, "lockForWriteWait fail = %d", errno);
        setErrno(errno);
        return false;
    }

    return true;
}


bool CFileLock::lockForRead()
{
    if (m_fd < 0) {
        return false;
    }
#ifdef OPEN_DEBUG_LOG
    UTILS_LOGI(TAG, "lockForRead");
#endif
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(m_fd, F_SETLK, &lock) < 0) {
        UTILS_LOGE(TAG, "lockForRead fail = %d", errno);
        setErrno(errno);
        return false;
    }

    return true;
}

bool CFileLock::lockForReadWait()
{
    if (m_fd < 0) {
        return false;
    }

#ifdef OPEN_DEBUG_LOG
    UTILS_LOGI(TAG, "lockForReadWait");
#endif
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(m_fd, F_SETLKW, &lock) < 0) {
        UTILS_LOGE(TAG, "lockForReadWait fail = %d", errno);
        setErrno(errno);
        return false;
    }

    return true;
}

bool CFileLock::unlock()
{
    if (m_fd < 0) {
        return false;
    }
#ifdef OPEN_DEBUG_LOG
    UTILS_LOGI(TAG, "unlock");
#endif
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(m_fd, F_SETLK, &lock) < 0) {
        setErrno(errno);
        return false;
    }

    return true;
}


bool CFileLock::isLocked()
{
    int pid = -1;

    if (m_fd < 0) {
        UTILS_LOGE(TAG, "get lock fail, fd error");
        return false;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = 0;

    if (fcntl(m_fd, F_GETLK, &lock) < 0) {
        UTILS_LOGE(TAG, "get lock fail");
        setErrno(errno);
        return false;
    }

#ifdef OPEN_DEBUG_LOG
    UTILS_LOGI(TAG, "get lock state = %d", lock.l_type);
#endif

    if (F_UNLCK == lock.l_type) {
#ifdef OPEN_DEBUG_LOG
        UTILS_LOGI(TAG, "lock state is not locked");
#endif
    } else if (F_WRLCK == lock.l_type) {
#ifdef OPEN_DEBUG_LOG
        UTILS_LOGI(TAG, "lock state is locked, locker pid is %d", lock.l_pid);
#endif
        pid = lock.l_pid;
    }

    return (pid > 0);
}

bool CFileLock::writePid()
{
    if (m_fd < 0) {
        UTILS_LOGE(TAG, "get lock fail, fd error");
        return false;
    }

    int ret = -1;
    char processID[8] = {0};
    int pid = (int)getpid();

#ifdef OPEN_DEBUG_LOG
    UTILS_LOGI(TAG, "write pid = %d", pid);
#endif

    int len = snprintf(processID, sizeof(processID) - 1, "%d", pid);
    ret = write(m_fd, processID, len);
    if (ret != len) {
        UTILS_LOGE(TAG, "write pid fail, len = %d, ret = %d", len, ret);
        return false;
    }

    return true;
}

int CFileLock::getLockerPid()
{
    int pid = -1;

    if (m_fd < 0) {
        UTILS_LOGE(TAG, "get lock fail, fd error");
        return false;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = 0;

    if (fcntl(m_fd, F_GETLK, &lock) < 0) {
        UTILS_LOGE(TAG, "get lock fail");
        setErrno(errno);
        return false;
    }

#ifdef OPEN_DEBUG_LOG
    UTILS_LOGI(TAG, "get lock state = %d", lock.l_type);
#endif

    if (F_UNLCK == lock.l_type) {
#ifdef OPEN_DEBUG_LOG
        UTILS_LOGI(TAG, "lock state is not locked");
#endif
    } else if (F_WRLCK == lock.l_type) {
#ifdef OPEN_DEBUG_LOG
        UTILS_LOGI(TAG, "lock state is locked, locker pid is %d", lock.l_pid);
#endif
        pid = lock.l_pid;
    }

    return pid;
}

void CFileLock::setErrno(int errNumber)
{
    m_errnumber = errNumber;
}

int CFileLock::getErrno()
{
    return m_errnumber;
}

}

