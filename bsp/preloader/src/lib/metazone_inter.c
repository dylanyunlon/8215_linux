#include "x_typedef.h"
#include "x_printf.h"
#include "reserved_memory.h"
#include "metazone_inter.h"

bool _fgWritableZoneInited = FALSE;
VOID  *_pMetaZone = NULL;

TMetaZone *_pMetaHeader = NULL;
UINT32 *_pu4Value = NULL;


/** only support read API for loader */
UINT32 Metazone_Init(void)
{
    RSV_MEM_T *mtz_rsv = NULL;
    //Printf("[MTZ][Preloader] Metazone_Init enter\r\n");
    mtz_rsv = (RSV_MEM_T *)get_rsv_mem_by_name("metazone");
    if (NULL == mtz_rsv) {
        Printf("[MTZ][Preloader] metazone in arm2 start addr is (0x%x)\r\n", mtz_rsv->start_addr);
        return 0;
    }
    _pMetaZone = (UINT32 *)(UINT32)(mtz_rsv->start_addr);;
    //Printf("[MTZ][Preloader] mtz1_base:0x%x mtz2_base:0x%x _pMetaZone:0x%x _pMetaZone2:0x%x\r\n", mtz1_base, mtz2_base, _pMetaZone, _pMetaZone2);

    _pMetaHeader = (TMetaZone *)_pMetaZone;
    _pu4Value = (UINT32 *)((UINT32) _pMetaHeader + _pMetaHeader->dwValueOffset);

    if (_pMetaHeader->dwSignature != METAZONE_SIGNATURE) {
        Printf("[MTZ][Preloader] metazone1 signature invalid!\r\n");
        return -1;
    }

    Printf("[MTZ][Preloader]metazone init ok\r\n");
    _fgWritableZoneInited = TRUE;
    return 0;
}

UINT32  _MetaZone_Read(UINT32 u4Idx, UINT32 *pu4Data)
{

    Printf("[MTZ][Preloader] MTZ Read index:0x%x\r\n", u4Idx);
    if (MZ_WR_IDX_START <= u4Idx )
    {
        if (!_fgWritableZoneInited)
        {
            return -1;
        }
        u4Idx -= MZ_WR_IDX_START;
        if (u4Idx < _pMetaHeader->dwValueNum)
        {
            *pu4Data = _pu4Value[u4Idx];
            return 0;
        }
        u4Idx += MZ_WR_IDX_START;
    }
    return -1;
}
