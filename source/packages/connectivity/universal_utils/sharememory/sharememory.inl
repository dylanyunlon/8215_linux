
#include "sharememory.h"
#include <sys/shm.h>
#include <errno.h>
#include <string.h>

#include "clog.h"
#include "semaphoreutils.h"

using namespace universal_utils;
template<typename MemStruct>
CShareMemory<MemStruct>::CShareMemory(int key, int count)
: m_shm(0)
, m_memAddr(NULL)
, m_firstCreate(false)
, m_semphore(NULL)
{
    int size = count * sizeof(MemStruct);

    m_shm = shmget(key, size, IPC_CREAT|IPC_EXCL|0666);
    if (m_shm < 0) {
        if (errno == EEXIST) {
            m_firstCreate = false;
            m_shm = shmget(key, 0, 0666);
        }
    } else {
        m_firstCreate = true;
    }

    if (m_shm >= 0) {
        m_memAddr = (MemStruct*)shmat(m_shm, NULL, 0);
    }

    if ((long)m_memAddr == -1) {
        m_memAddr = NULL;
    }

    m_semphore = new CSemaphoreUtils(key);
}


template<typename MemStruct>
CShareMemory<MemStruct>::~CShareMemory()
{
    if (m_shm >= 0 && m_memAddr != NULL) {
        shmdt((void *)m_memAddr);
    }

    shmid_ds ds;
    shmctl(m_shm, IPC_STAT, &ds);

    if (ds.shm_nattch == 0) {
        shmctl(m_shm, IPC_RMID, 0);
    }
    if (m_semphore) {
        delete m_semphore;
        m_semphore = NULL;
    }
}

template<typename MemStruct>
void CShareMemory<MemStruct>::clear()
{
    if (m_memAddr != NULL)
    {
        lock();
        memset((char*)m_memAddr, 0, sizeof(MemStruct));
        unlock();
    }
}


template<typename MemStruct>
bool CShareMemory<MemStruct>::isFirstCreate () const
{
    return m_firstCreate;;
}


template<typename MemStruct>
MemStruct* CShareMemory<MemStruct>::getMemData()
{
    return m_memAddr;
}

template<typename MemStruct>
bool CShareMemory<MemStruct>::getMemData(MemStruct &data)
{
    bool ret = false;
	if (m_memAddr != NULL) {
	    data = m_memAddr;
        ret = true;
    }
    
    return ret;
}

template<typename MemStruct>
int CShareMemory<MemStruct>::getMemId() const
{
    return m_shm;
}

template<typename MemStruct>
bool CShareMemory<MemStruct>::updateMemData(MemStruct &data)
{
    bool ret = false;

    if (m_memAddr != NULL)
    {
        lock();
        *m_memAddr = data;
        unlock();

        ret = true;
    }

    return ret;
}

template<typename MemStruct>
bool CShareMemory<MemStruct>::updateMemData(int offset, const void* data, int size)
{
    bool ret = false;

    if (m_memAddr != NULL && data != NULL)
    {
        lock();
        memcpy((char*)m_memAddr + offset, data, size);
        unlock();

        ret = true;
    }

    return ret;
}



template<typename MemStruct>
MemStruct* CShareMemory<MemStruct>::getMemDataLock()
{
    lock();
    return m_memAddr;
}


template<typename MemStruct>
bool CShareMemory<MemStruct>::memDataUnLock()
{

    unlock();
    return true;
}

template<typename MemStruct>
bool CShareMemory<MemStruct>::lock ()
{
    bool ret = false;
    if (m_semphore) {
        m_semphore->lock();
    }

    if (m_shm >= 0) {
        shmctl(m_shm, SHM_LOCK, 0);
        ret = true;
    }

    return ret;
}

template<typename MemStruct>
bool CShareMemory<MemStruct>::unlock ()
{
    bool ret = false;

    if (m_shm >= 0) {
        shmctl(m_shm, SHM_UNLOCK, 0);
        ret = true;
    }

    if (m_semphore) {
        m_semphore->unlock();
    }

    return ret;
}