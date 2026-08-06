
#include <sys/types.h>
#include <stdint.h>
#include "x_types.h"
#include <printf.h>
#include <string.h>
#include <malloc.h>

#include "reserved_memory.h"
#include "metazone_inter.h"

BOOL _fgWritableZoneInited = FALSE;
VOID  *_pMetaZone = NULL;
VOID  *_pMetaZone2 = NULL;
TMetaZone *_pMetaHeader = NULL;
TMetaZone *_pMetaHeader2 = NULL;
PBYTE _pbReserve = NULL;
UINT32 *_pu4Value = NULL;
UINT32 _u4BinaryStart = 0;

#define MTZ_PARTITION_SIZE 0x20000

/** only support read API for arm2 */

UINT32 Metazone_Init(void)
{
    RSV_MEM_T *mtz_rsv = NULL;
    UINT32 mtz1_base = 0;
    UINT32 mtz2_base = 0;
    UINT32 u4Data=0;

    Printf("[MTZ][AMR2] Metazone_Init enter\r\n");

    mtz_rsv = get_rsv_mem_by_name("metazone");
    if (NULL == mtz_rsv) {
        Printf("[ARM2] metazone in arm2 start addr is (0x%x)\r\n", mtz_rsv->start_addr);
        return 0;
    }

    mtz1_base = (UINT32)(mtz_rsv->start_addr);
    mtz2_base = (UINT32)(mtz_rsv->start_addr + MTZ_PARTITION_SIZE / 2);

    _pMetaZone = ARM1PHY2ARM2UCV(mtz1_base);
    _pMetaZone2 = ARM1PHY2ARM2UCV(mtz2_base);
    Printf("[ARM2] mtz1_base:0x%x mtz2_base:0x%x _pMetaZone:0x%x _pMetaZone2:0x%x\r\n", mtz1_base, mtz2_base, _pMetaZone, _pMetaZone2);

    _pMetaHeader = (TMetaZone *)_pMetaZone;
    _pMetaHeader2 = (TMetaZone *)_pMetaZone2;

    _pbReserve = (BYTE *)((UINT32) _pMetaZone + sizeof(TMetaZone));
    _pu4Value = (UINT32 *)((UINT32) _pMetaZone + _pMetaHeader->dwValueOffset);
    _u4BinaryStart = (UINT32) _pMetaZone + _pMetaHeader->dwBinaryOffset;

    if (_pMetaHeader->dwSignature != METAZONE_SIGNATURE)
	{
		Printf("[MTZ][AMR2] metazone1 signature invalid!\r\n");
		return -1;
	}

    Printf("[MTZ][AMR2] metazone init ok\r\n");
    _fgWritableZoneInited = TRUE;
    _MetaZone_Read(0x10001, &u4Data);
    Printf("[ARM2] MTZ Read 0x10001 val : %d\r\n", u4Data);
    return 0;
}

UINT32  _MetaZone_Read(UINT32 u4Idx, UINT32 *pu4Data)
{
    if (MZ_WR_IDX_START <= u4Idx )
    {
        if (!_fgWritableZoneInited)
        {
            return (MZ_FAILURE);
        }
        u4Idx -= MZ_WR_IDX_START;
        if (u4Idx < _pMetaHeader->dwValueNum)
        {
            *pu4Data = _pu4Value[u4Idx];
            return (MZ_SUCCESS);
        }
        u4Idx += MZ_WR_IDX_START;
    }
    return (MZ_FAILURE);
}

UINT32  _MetaZone_ReadBinary(UINT32 u4Idx, BYTE *pbData, UINT32 u4Size)
{
    if (MZ_WR_IDX_START <= u4Idx)
    {
        if (!_fgWritableZoneInited)
        {
            return (MZ_FAILURE);
        }
        u4Idx -= MZ_WR_IDX_START;
        if (u4Idx < _pMetaHeader->dwBinaryNum)
        {
            UINT32 u4Tmp = *(UINT32 *)( _u4BinaryStart + (_pMetaHeader->dwBinaryItemSize + 4) * u4Idx);
            if (u4Size > u4Tmp)
                u4Size = u4Tmp;
            memcpy(pbData, (BYTE *)(_u4BinaryStart + (_pMetaHeader->dwBinaryItemSize + 4) * u4Idx + 4), u4Size);
            return (u4Size);
        }
    }
    return (MZ_FAILURE);
}


UINT32 _MetaZone_ReadReserved(BYTE *pbData, UINT32 u4Size)
{
    if (!_fgWritableZoneInited)
        return (MZ_FAILURE);
    if (u4Size > _pMetaHeader->dwReserveSize)
        u4Size = _pMetaHeader->dwReserveSize;
    memcpy(pbData, _pbReserve, u4Size);
    return (u4Size);
}

UINT32 _MetaZone_ReadReserved_Offset(BYTE *pbData,UINT32 offset, UINT32 u4Size)
{
    if (!_fgWritableZoneInited)
        return (MZ_FAILURE);
	
	if(offset>_pMetaHeader->dwReserveSize)
		return MZ_FAILURE;

	if((u4Size+offset)>_pMetaHeader->dwReserveSize)
		u4Size=_pMetaHeader->dwReserveSize-offset;

    memcpy(pbData, _pbReserve+offset, u4Size);
    return (u4Size);
}

/*
UINT32  _MetaZone_Write(UINT32 u4Idx, UINT32 u4Data)
{
    if (MZ_WR_IDX_START <= u4Idx)
    {
        if (!_fgWritableZoneInited)
        {
            return (MZ_FAILURE);
        }
        u4Idx -= MZ_WR_IDX_START;
        if (u4Idx < _pMetaHeader->dwValueNum)
        {
            _pu4Value[u4Idx] = u4Data ;
            return (MZ_SUCCESS);
        }
    }
	return (MZ_FAILURE);
}

UINT32  _MetaZone_WriteBinary(UINT32 u4Idx, BYTE *pbData, UINT32 u4Size)
{
    if (MZ_WR_IDX_START <= u4Idx)
    {
        if (!_fgWritableZoneInited)
        {
            return (MZ_FAILURE);
        }
        u4Idx -= MZ_WR_IDX_START;
        if (u4Idx < _pMetaHeader->dwBinaryNum)
        {
            if (u4Size > _pMetaHeader->dwBinaryItemSize)
                u4Size = _pMetaHeader->dwBinaryItemSize;
            *(UINT32 *)( _u4BinaryStart + (_pMetaHeader->dwBinaryItemSize + 4) * u4Idx) = u4Size;
            memcpy((BYTE *)(_u4BinaryStart + (_pMetaHeader->dwBinaryItemSize + 4) * u4Idx + 4), pbData, u4Size);

            return (MZ_SUCCESS);
        }
    }
    return (MZ_FAILURE);
}


UINT32  _MetaZone_WriteReserved(BYTE *pbData, UINT32 u4Size)
{
    if (!_fgWritableZoneInited)
        return (MZ_FAILURE);
    if (u4Size > _pMetaHeader->dwReserveSize)
        u4Size = _pMetaHeader->dwReserveSize;
     memcpy(_pbReserve, pbData, u4Size);

     return (MZ_SUCCESS);
}

UINT32  _MetaZone_WriteReserved_Offset(BYTE *pbData,UINT32 offset, UINT32 u4Size)
{
    if (!_fgWritableZoneInited)
        return (MZ_FAILURE);
	
	if(offset>_pMetaHeader->dwReserveSize)
		return MZ_FAILURE;

	if((u4Size+offset)>_pMetaHeader->dwReserveSize)
		u4Size=_pMetaHeader->dwReserveSize-offset;

     memcpy(_pbReserve+offset, pbData, u4Size);

     return (MZ_SUCCESS);
}
*/



