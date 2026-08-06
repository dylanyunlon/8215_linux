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

#ifndef __SEMAPHOREUTILS_H__
#define __SEMAPHOREUTILS_H__

#include <errno.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>

#include <unistd.h>
#include "clog.h"

#define SEM_OK          0
#define SEMERR_BASE     -1
#define SEMERR_PARAM    -2
#define SEMERR_EEXIST   -3

namespace universal_utils
{
const static char* SEM_TAG = "CSemphoreUtils";
// #define OPEN_DEBUG_LOG

/* use for Mutual exclusion of processes or threads */
const static int INIT_VALUE_OF_SEM = 1;

class CSemaphoreUtils
{
public:
    union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                               (Linux specific) */
    };
public:
    CSemaphoreUtils(int key, unsigned int semValue = INIT_VALUE_OF_SEM)
        : m_key(key)
        , m_semId(-1)
        , m_semV(semValue)
    {
        int ret = semCreate();
        if (ret != SEM_OK) {
#ifdef OPEN_DEBUG_LOG
            UTILS_LOGW(SEM_TAG, "semCreate err:%d", ret);
#endif
            if (ret == SEMERR_EEXIST) {
#ifdef OPEN_DEBUG_LOG
                UTILS_LOGW(SEM_TAG, "sem already exist:%d\n", ret);
#endif
            }
        }

        if (ret == SEM_OK || ret == SEMERR_EEXIST) {
            ret = semOpen();
            if (ret != SEM_OK) {
#ifdef OPEN_DEBUG_LOG
                UTILS_LOGE(SEM_TAG, "aquire sem error:%d", ret);
#endif
            } else {
                int val = 0;
                semGetval(val);
#ifdef OPEN_DEBUG_LOG
                UTILS_LOGI(SEM_TAG, "sem val:%d", val);
#endif
            }
        }
    }

    virtual ~CSemaphoreUtils()
    {

    }

    /******************************************************************
     * unlock: P-operation of semaphores
     *
     * Parameters: Output parameters, semaphore values
     *
     * returns:   sucess:          SEM_OK
     *            error:           SEMERR_BASE
     ******************************************************************/
    int semGetval(int &value)
    {
        int ret = SEM_OK;

        ret = semctl(m_semId, 0, GETVAL);
        if (ret == SEMERR_BASE) {
            UTILS_LOGE(SEM_TAG, "semctl err:%s\n", strerror(errno));
        } else {
            value = ret;
            ret = SEM_OK;
        }

        return ret;
    }

    /******************************************************************
     * unlock: P-operation of semaphores
     *
     * returns:   sucess:          SEM_OK
     *            error:           SEMERR_BASE
     ******************************************************************/
    int lock(short sem_flag = SEM_UNDO)
    {
        struct sembuf buf = {0, -1, sem_flag};
        int ret = SEM_OK;
        ret = semop(m_semId, &buf, 1);
        if (ret != SEM_OK) {
            UTILS_LOGE(SEM_TAG, "lock semop err:%s", strerror(errno));
        }

        return ret;
    }

    /******************************************************************
     * unlock: V-operation of semaphores
     *
     * returns:   sucess:          SEM_OK
     *            error:           SEMERR_BASE
     ******************************************************************/
    int unlock(short sem_flag = SEM_UNDO)
    {
        struct sembuf buf = {0, 1, sem_flag};
        int ret = SEM_OK;
        ret = semop(m_semId, &buf, 1);
        if (ret != SEM_OK) {
            UTILS_LOGE(SEM_TAG, "unlock semop error:%s", strerror(errno));
        }

        return ret;
    }
private:
    /******************************************************************
     * semCreate: Create semaphores
     *
     * returns:   sucess:          SEM_OK
     *            error:           SEMERR_BASE
     *            already exist:   SEMERR_EEXIST
     ******************************************************************/
    int semCreate()
    {
        int ret = semget(m_key, 1, 0666 | IPC_CREAT | IPC_EXCL);
        if (ret == SEMERR_BASE) {
            if (errno == EEXIST) {
                ret = SEMERR_EEXIST;
#ifdef OPEN_DEBUG_LOG
                UTILS_LOGW(SEM_TAG, "sem_create err:sem already exist:%s\n", strerror(errno));
#endif
            } else {
#ifdef OPEN_DEBUG_LOG
                UTILS_LOGE(SEM_TAG, "sem_create err:%s\n", strerror(errno));
#endif
            }
        } else {
            m_semId = ret;

            /* set the initial semaphore value of 1, use for Mutual exclusion of processes or
               threads */
            ret = semSetVal(m_semV);
            if (ret != SEM_OK) {
                UTILS_LOGE(SEM_TAG, "semSetval err:%d\n", ret);
                ret = SEMERR_BASE;
            }
        }

        return ret;
    }

    /******************************************************************
     * semOpen: open semaphores
     *
     * returns:   sucess:          SEM_OK
     *            error:           SEMERR_BASE
     ******************************************************************/
    int semOpen()
    {
        int ret = SEM_OK;

        ret = semget(m_key, 0, 0);
        if (ret == SEMERR_BASE) {
            UTILS_LOGE(SEM_TAG, "semOpen err:%s\n", strerror(errno));
        } else {
            m_semId = ret;
            ret = SEM_OK;
        }

        return ret;
    }

    /******************************************************************
     * semSetVal: set the initial semaphore value
     * Parameter: If you want to be used for mutually exclusive processes
     *            or threads, the value should be 1
     *
     * returns:   sucess:          SEM_OK
     *            error:           SEMERR_BASE
     ******************************************************************/
    int semSetVal(int val)
    {
        int ret = SEM_OK;
        union semun su;
        su.val = val;
        ret = semctl(m_semId, 0, SETVAL, su);

        return ret;
    }

    int m_key;
    int m_semId;
    unsigned int m_semV;

};

}

#endif // __SEMAPHOREUTILS_H__

