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

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "metazone_ioctl.h"
#include "metazone.h"
#include <stdio.h>

#define MTZ_DBG(format, ...)    //printf("[D][MM][DT][%s:%d] " format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MTZ_INFO(format, ...)   printf("[I][MTZ][%s:%d] " format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MTZ_TRACE(format, ...)  printf("[I][MTZ][%s:%d] " format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MTZ_WARN(format, ...)   printf("[W][MTZ][%s:%d] " format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MTZ_ERROR(format, ...)  printf("[E][MTZ][%s:%d] " format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)


#define METAZONE_DRV_NAME      "/dev/mtz"

#define INVALID_HANDLE_VALUE		(-1)

static int _metazone_fd = INVALID_HANDLE_VALUE;

typedef struct {
	void *pInBuf;
	int InSize ;
	void *pOutBuf;
	int OutSize ;
	unsigned int *pBytesReturned;
} WIN32_IOCTL_DATA;

/*********************************************************************************/
/* Metazone Read/Write Return Value Description:                                 */
/*      Read  --> return real read size, return 0 means read failed.             */
/*      Write --> return 0 means write success, else return non-zero value       */
/*********************************************************************************/

/****************************************/
/* return value: 0 - success, -1 - fail */
/****************************************/
int DeviceIoControl(int hDevice, int IoControlCOde, void* lpInBuf, unsigned int InBufSize ,
					void* lpOutBuf, unsigned int OutBufSize , unsigned int* lpBytesReturned, void* reserver3)
{
    int ret = 0;
    WIN32_IOCTL_DATA pData;

    pData.pInBuf = lpInBuf;
    pData.InSize = InBufSize;
    pData.pOutBuf = lpOutBuf;
    pData.OutSize = OutBufSize;
    pData.pBytesReturned = lpBytesReturned;

    ret = ioctl(hDevice, IoControlCOde, &pData);
    if ( ret < 0 )
    {
        MTZ_ERROR("ioctl err : 0x%x\n", ret);
        return -1;
    }
    return 0;
}


unsigned int MetaZone_Init(void)
{
    if (INVALID_HANDLE_VALUE == _metazone_fd)
    {
        _metazone_fd = open(METAZONE_DRV_NAME, O_RDWR);
    }
    if (INVALID_HANDLE_VALUE == _metazone_fd)
    {
        MTZ_ERROR("Invalid metazone fd!\n");
        return MZ_FAILURE;
    }
    return MZ_SUCCESS;
}

unsigned int MetaZone_Deinit(void)
{
    if (INVALID_HANDLE_VALUE != _metazone_fd)
    {
        close(_metazone_fd);
        _metazone_fd = INVALID_HANDLE_VALUE;
    }
    return MZ_SUCCESS;
}

unsigned int MetaZone_Read(unsigned int u4Idx, unsigned int *pu4Data)
{
    unsigned int u4Size = 0;
    if (NULL == pu4Data)
    {
        MTZ_WARN("data buf is NULL\n");
        return 0;
    }

    MetaZone_Init();
    if (INVALID_HANDLE_VALUE == _metazone_fd)
    {
        MTZ_ERROR("Invalid metazone fd!\n");
        return 0;
    }
    if (DeviceIoControl(_metazone_fd, IOCTL_MZ_READ_VALUE,(void *)(&u4Idx), sizeof(unsigned int),(void *)pu4Data, sizeof(unsigned int), &u4Size, NULL))
    {
        MTZ_ERROR("read index [0x%x] failed!\n", u4Idx);
        return 0;
    }
    return u4Size;
}

unsigned int MetaZone_Write(unsigned int u4Idx, unsigned int u4Data)
{
    unsigned int szInBuf[2] = {u4Idx, u4Data};

    MetaZone_Init();
    if (INVALID_HANDLE_VALUE == _metazone_fd)
    {
        return MZ_NO_INIT;
    }

    if (DeviceIoControl(_metazone_fd, IOCTL_MZ_WRITE_VALUE,(void *)szInBuf, sizeof(szInBuf), NULL, 0, NULL, NULL))
    {
        MTZ_ERROR("write index [0x%x] failed!\n", u4Idx);
        return MZ_FAILURE;
    }
    return MZ_SUCCESS;
}

unsigned int MetaZone_ReadBinary(unsigned int u4Idx, char *pbData, unsigned int u4Size)
{
    unsigned int u4BytesRet = 0;
    if (NULL == pbData)
    {
        MTZ_WARN("data buf is NULL\n");
        return 0;
    }

    MetaZone_Init();
    if (INVALID_HANDLE_VALUE == _metazone_fd)
    {
        MTZ_ERROR("Invalid metazone fd!\n");
        return 0;
    }

    if (DeviceIoControl(_metazone_fd, IOCTL_MZ_READ_BINARY,(void *)(&u4Idx), sizeof(unsigned int), (void *)pbData, u4Size, &u4BytesRet, NULL))
    {
        MTZ_ERROR("readbinary index [0x%x] failed!\n", u4Idx);
        return 0;
    }
    return u4BytesRet;
}

unsigned int MetaZone_WriteBinary(unsigned int u4Idx, char *pbData, unsigned int u4Size)
{
    if (NULL == pbData)
    {
        MTZ_WARN("data buf is NULL\n");
        return MZ_FAILURE;
    }

    MetaZone_Init();
    if (INVALID_HANDLE_VALUE == _metazone_fd)
    {
        MTZ_ERROR("Invalid metazone fd!\n");
        return MZ_NO_INIT;
    }

    if (DeviceIoControl(_metazone_fd, IOCTL_MZ_WRITE_BINARY,(void *)(&u4Idx), sizeof(unsigned int), (void *)pbData, u4Size, NULL, NULL))
    {
        MTZ_ERROR("writebinary index [0x%x] failed!\n", u4Idx);
        return MZ_FAILURE;
    }
    return MZ_SUCCESS;
}

unsigned int MetaZone_ReadReserved(char *pbData, unsigned int u4Size)
{
    unsigned int u4BytesRet = 0;
    if (NULL == pbData)
    {
        MTZ_WARN("data buf is NULL\n");
        return 0;
    }

    MetaZone_Init();
    if (INVALID_HANDLE_VALUE == _metazone_fd)
    {
        MTZ_ERROR("Invalid metazone fd!\n");
        return 0;
    }

    if (DeviceIoControl(_metazone_fd, IOCTL_MZ_READ_RESERVED, NULL, 0, (void *)pbData, u4Size, &u4BytesRet, NULL))
    {
        MTZ_ERROR("readreserved  failed!\n");
        return 0;
    }
    return u4BytesRet;
}

unsigned int MetaZone_WriteReserved(char *pbData, unsigned int u4Size)
{
    if (NULL == pbData)
    {
        MTZ_WARN("data buf is NULL\n");
        return MZ_FAILURE;
    }

    MetaZone_Init();
    if (INVALID_HANDLE_VALUE == _metazone_fd)
    {
        MTZ_ERROR("Invalid metazone fd!\n");
        return MZ_NO_INIT;
    }

    if (DeviceIoControl(_metazone_fd, IOCTL_MZ_WRITE_RESERVED, NULL, 0, (void *)pbData, u4Size, NULL, NULL))
    {
        MTZ_ERROR("writereserved failed!\n");
        return MZ_FAILURE;
    }
    return MZ_SUCCESS;
}

unsigned int MetaZone_ReadInfo(METAZONE_INFO_T *prInfo)
{
    unsigned int u4BytesRet = 0;
    if (NULL == prInfo)
    {
        MTZ_WARN("data buf is NULL\n");
        return MZ_FAILURE;
    }

    MetaZone_Init();
    if (INVALID_HANDLE_VALUE == _metazone_fd)
    {
        MTZ_ERROR("Invalid metazone fd!\n");
        return MZ_NO_INIT;
    }

    if (DeviceIoControl(_metazone_fd, IOCTL_MZ_READ_INFO, NULL, 0, (void *)prInfo, sizeof(METAZONE_INFO_T), &u4BytesRet, NULL))
    {
        MTZ_ERROR("readinfo failed!\n");
        return MZ_FAILURE;
    }
    return MZ_SUCCESS;
}


unsigned int MetaZone_Flush(int fgblock)
{
    MetaZone_Init();
    if (INVALID_HANDLE_VALUE == _metazone_fd)
    {
        MTZ_ERROR("Invalid metazone fd!\n");
        return MZ_NO_INIT;
    }

    if (DeviceIoControl(_metazone_fd, IOCTL_MZ_FLUSH, (void *)&fgblock, sizeof(int), NULL, 0, NULL, NULL))
    {
        MTZ_ERROR("flush flag [0x%x] failed!\n", fgblock);
        return MZ_FAILURE;
    }
    return MZ_SUCCESS;
}
