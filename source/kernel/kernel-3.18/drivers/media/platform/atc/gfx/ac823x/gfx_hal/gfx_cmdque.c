/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/


//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------

#include "gfx_if.h"
#include "gfx_cmdque.h"
#include "gfx_dif.h"
#include "gfx_hw.h"
//#include "gfx_drv_if.h"  --ATC68024

//#include "gfx_time.h"
//#include "gfx_debug.h"
//#include "x_hal_1176.h" --ATC68024
#include "x_rtos.h"
#include "x_hal_ic.h"
#include "drv_gfx.h"
#include "gfx_cmdque.h"

//ATC68024 --dont find symbol:GFX_IsLog
#define GFX_IsLog
#define DEFINE_IS_LOG   GFX_IsLog       // for LOG use
#include "x_debug.h"
#include "chip_ver.h"
#include <linux/delay.h>

#include "sys_config.h"

#if CONFIG_SYS_MEM_PHASE2
#include "x_mem_phase2.h"
#elif CONFIG_SYS_MEM_PHASE3
#include "x_kmem.h"
#endif


//---------------------------------------------------------------------------
// Configurations
//---------------------------------------------------------------------------
//extern uint32_t RleFlushFlag;
#ifdef GFX_RISC_MODE
#define D_GFX_CQ_HW_INT_MODE    1   // 1: turn command queue off
#else
#define D_GFX_CQ_HW_INT_MODE    0   // 0: turn command queue on
#endif
#ifndef GFX_QUE_REG
//#define GFX_QUE_REG
#endif
#ifdef GFX_QUE_REG
static uint32_t GfxReg[80];
//static uint32_t GfxRegOld[80];
#endif

#ifdef GFX_DRV_LOG

extern GFX_CMD_QUEUE_TEST_T _t_cmd_queue_s50_test_struct[200];
extern GFX_CMD_QUEUE_TEST_T _t_cmd_queue_s100_test_struct[200];
extern GFX_CMD_QUEUE_TEST_T _t_cmd_queue_sc100_test_struct[200];
extern GFX_CMD_QUEUE_TEST_T _t_cmd_queue_ea100_test_struct[200];
extern GFX_CMD_QUEUE_TEST_T _t_cmd_queue_s200_test_struct[200];

extern uint32_t  ui4_s50_test_count;
extern uint32_t  ui4_s100_test_count;
extern uint32_t  ui4_sc100_test_count;
extern uint32_t  ui4_ea100_test_count;
extern uint32_t  ui4_s200_test_count;
extern UINT8  _ui1_test_type;
#endif


//#define ASYNC_MECHANISM
//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------

// que capacity
// each command occupys 8 bytes
// there are always even commands in que when gfx engine start
// 32768 * 8 = 262144 = 256 KB
//  4096 * 8 =  32768 =  32 KB
//#define E_GFX_CQ_QCAPACITY       4096 //32768

/** que align mask
 *  gfx command que should align on dram word (128 bits = 16 bytes)
 */
#define E_GFX_CQ_ALIGN          16

/** min of que capacity is 2
 *  mt5351 is 4
 */
#define GFX_CMQ_MIN_SIZE        2

#define GFX_CMD_MARGIN          2
#define GFX_CMD_MAX_FIRE       10

#define GFX_ONE_CMD_SIZE        8   // one cmd = 8 bytes

//#define VIRTUAL(addr)               (addr)

//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------

typedef struct _GFX_CMDQUE_VAR
{
   INT32    u4GfxHwFlushCount ;
   INT32    i4GfxCqCapacity;
   // for sw mode use
   uint32_t u4GfxSwFlushCount;
   uint32_t u4GfxSwIntCount ;

 volatile UINT64 *pu8GfxCmdqueBuf ;
   #if defined(GFX_ENABLE_SW_MODE)
 volatile  MI_DIF_UNION_T *prRegBase;
   #endif
} GFX_CMDQUE_VAR;


//---------------------------------------------------------------------------
// Macro definitions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Imported variables
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Imported functions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Static function forward declarations
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Static variables
//---------------------------------------------------------------------------

//static volatile UINT64 *_pu8GfxCmdqueBuf = NULL;

static volatile GFX_CMDQUE_T _rGfxCmdQue[GFX_HAL_HW_INST_NUM];
GFX_CMDQUE_VAR  Gfx_CmdQueVar[GFX_HAL_HW_INST_NUM];
extern unsigned long gfx_base ;

#if 0
#if (CONFIG_DRV_VERIFY_SUPPORT)
volatile uint32_t _u4GfxCmdQueBuf;
volatile uint32_t _u4GfxRegistBuf;
#endif
#else
#if 1//(CONFIG_DRV_VERIFY_SUPPORT)
uint32_t _u4GfxCmdQueBuf[4096];
uint32_t _u4GfxRegistBuf[8];
#endif
#endif

#if defined(GFX_ENABLE_SW_MODE)
//static volatile MI_DIF_UNION_T *_prRegBase = NULL;
#endif  //#if defined(GFX_ENABLE_SW_MODE)

//static uint32_t _u4GfxHwFlushCount = 0;

//static INT32 _i4GfxCqCapacity = (INT32)EGFX_CPT_128KB;

static INT32 _i4GfxCmdqueBufExist = (INT32)FALSE;

//static INT32 _i4GfxCqFireCnt = 0;

// for sw mode use
//static uint32_t _u4GfxSwFlushCount = 0;
//static uint32_t _u4GfxSwIntCount = 0;


//---------------------------------------------------------------------------
// Static functions
//---------------------------------------------------------------------------

void _Gfx_CmdQueVarInt(void)
{
     uint32_t i4;
	 
  if (_i4GfxCmdqueBufExist == (INT32)FALSE)
  {
     for(i4=0;i4<GFX_HAL_HW_INST_NUM;i4++)
     {
        Gfx_CmdQueVar[i4].u4GfxHwFlushCount = 0;
    	Gfx_CmdQueVar[i4].u4GfxSwFlushCount = 0; 
    	Gfx_CmdQueVar[i4].u4GfxSwIntCount= 0;
    #if CONFIG_LOSSLESS_COMPRESS_AUTO_FLIP_MODE
        Gfx_CmdQueVar[i4].i4GfxCqCapacity = (INT32)EGFX_CPT_128KB;
    #else
    #ifdef GFX_PERFORMANCE_TEST
        Gfx_CmdQueVar[i4].i4GfxCqCapacity = (INT32)EGFX_CPT_128KB;
    #else
        Gfx_CmdQueVar[i4].i4GfxCqCapacity = (INT32)EGFX_CPT_32KB;
    #endif
    #endif
 #if defined(GFX_ENABLE_SW_MODE)
	    Gfx_CmdQueVar[i4].prRegBase = NULL;
 #endif
	    Gfx_CmdQueVar[i4].pu8GfxCmdqueBuf = NULL;

     }
  }
}
#if defined(GFX_ENABLE_SW_MODE)
//-------------------------------------------------------------------------
/** _GfxSwRealIsr
 *  gfx software interrupt routine
 */
//-------------------------------------------------------------------------
static void _GfxSwRealIsr(uint32_t u4GfxHwId)
{
   // GFX_INC_CM_INT_COUNT(u4GfxHwId);  // for debug use

    ++Gfx_CmdQueVar[u4GfxHwId].u4GfxSwIntCount;     // sw interrupt count
    UNUSED(Gfx_CmdQueVar[u4GfxHwId].u4GfxSwIntCount);

    GFX_DifSetIdle(u4GfxHwId,TRUE);

#if !(CONFIG_DRV_LINUX) || (1 == GFX_SYNC_OLD_METHOD)
    // unlock cmdque resource
    GFX_UnlockCmdque(u4GfxHwId);
#elif 1//(CONFIG_DRV_VERIFY_SUPPORT)
    GFX_UnlockCmdque(u4GfxHwId);
#endif
}


//-------------------------------------------------------------------------
/** _GfxSwQueAction
 *  start sw engine
 */
//-------------------------------------------------------------------------
static void _GfxSwQueAction(uint32_t u4GfxHwId)
{
    INT32 i4Ret;
    INT32 i4Element; 
    INT32 i4CurIdx;
    UINT64 *pu8QueTop; 
    unsigned long *pu4RegBase;
    unsigned long u4RegAddr;
    uint32_t u4RegValue;

    pu4RegBase = (unsigned long *)(Gfx_CmdQueVar[u4GfxHwId].prRegBase);

    i4Element  = (INT32)(Gfx_CmdQueVar[u4GfxHwId].prRegBase->rField.fg_DRAMQ_LEN >> 3);
    ASSERT(i4Element == _rGfxCmdQue[u4GfxHwId].i4QueSize);

    pu8QueTop = (UINT64 *)_rGfxCmdQue[u4GfxHwId].pu8QueTop;
    i4CurIdx  = _rGfxCmdQue[u4GfxHwId].i4ReadIndex;

    while (i4Element--)
    {
        u4RegAddr  = (unsigned long)((pu8QueTop[i4CurIdx] >> 34) & 0x3ff);
        u4RegValue = (unsigned long)(pu8QueTop[i4CurIdx] & 0xffffffff);

        pu4RegBase[u4RegAddr] = u4RegValue;

        // only direct mode by now
        if ((u4RegAddr == (unsigned long)GREG_G_MODE) &&
            ((u4RegValue & 0x00000800) != 0))
        {
            if (0 != (i4Ret = pfnGFX_DifAction(u4GfxHwId)))
            {
                UNUSED(i4Ret);
                LOG(5, "C-model Action Fail!!\n");
                return;
            }
        }

        i4CurIdx++;     // cmdque current index

        // If current index >= bottom of cmdque then
        // set current index to top of cmdque.
        //if (i4CurIdx >= E_GFX_CQ_QCAPACITY)
        if (i4CurIdx >= _rGfxCmdQue[u4GfxHwId].i4QueCapacity)
        {
            i4CurIdx = 0;
        }
    }

    _GfxSwRealIsr(u4GfxHwId);
}
#endif // #if defined(GFX_ENABLE_SW_MODE)



//---------------------------------------------------------------------------
// Inter-file functions
//---------------------------------------------------------------------------


//-------------------------------------------------------------------------
/** _GFX_GetFlushCount
 *  get cmdque flush count
 *
 */
//-------------------------------------------------------------------------
uint32_t _GFX_GetFlushCount(uint32_t u4GfxHwId)
{
    return (uint32_t)Gfx_CmdQueVar[u4GfxHwId].u4GfxHwFlushCount;
}


//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------


//-------------------------------------------------------------------------
/** GFX_CmdQueInit
 *  init gfx command queue
 *  get hw register address, reset command queue
 */
//-------------------------------------------------------------------------
INT32 GFX_CmdQueInit(void)
{
    INT32 i4Ret;
    unsigned long u4GfxRegBase;
    uint32_t i4;
    _Gfx_CmdQueVarInt();

    for(i4 = 0;i4<GFX_HAL_HW_INST_NUM;i4++)
    {
        if ((uint32_t)E_GFX_SW_MOD == GFX_DifGetData(i4)->u4GfxMode)
        {
            i4Ret = GFX_DifGetRegBase(i4,&u4GfxRegBase);
            VERIFY(i4Ret == (INT32)E_GFX_OK);

            Gfx_CmdQueVar[i4].prRegBase = (volatile MI_DIF_UNION_T *)u4GfxRegBase;
        }
    }

    if (_i4GfxCmdqueBufExist == (INT32)FALSE)
    {
        uint32_t u4Size, u4Align;

        //u4Size  = (uint32_t)(E_GFX_CQ_QCAPACITY * GFX_ONE_CMD_SIZE);

        // allocate gfx cmdque buffer for gfx engine use
        //_pu8GfxCmdqueBuf =
        //    (volatile UINT64 *)BSP_AllocAlignedDmaMemory(u4Size, u4Align);
        for (i4=0; i4<GFX_HAL_HW_INST_NUM; i4++) // create a semaphore
        {
            u4Size  = (uint32_t)(Gfx_CmdQueVar[i4].i4GfxCqCapacity * GFX_ONE_CMD_SIZE);
            u4Align = (uint32_t)(E_GFX_CQ_ALIGN);
        #if 1//CONFIG_SYS_MEM_PHASE2
            Gfx_CmdQueVar[i4].pu8GfxCmdqueBuf=
                    (volatile UINT64 *) GFX_ALLOC_MEM(u4Size, u4Align);
        #elif CONFIG_SYS_MEM_PHASE3
            Gfx_CmdQueVar[i4].pu8GfxCmdqueBuf=
                    (volatile UINT64 *) GFX_ALLOC_MEM(u4Size, u4Align);
        #endif
            LOG(22, "GFX_CmdQueInit\n");
            LOG(23, "\n[GFX] Gfx_CmdQueVar[i4].pu8GfxCmdqueBuf= %d\n", Gfx_CmdQueVar[i4].pu8GfxCmdqueBuf);
            VERIFY(  Gfx_CmdQueVar[i4].pu8GfxCmdqueBuf != NULL);
        }
        _i4GfxCmdqueBufExist = (INT32)TRUE;
        //_i4GfxCqFireCnt = 0;
    }

#if 0 // CmdQueInit
    #if defined(GFX_ENABLE_SW_MODE)
    //-----------------------------------------------------------
        // sw mode
        if ((uint32_t)E_GFX_SW_MOD == GFX_DifGetData()->u4GfxMode)
        {
            uint32_t u4GfxRegBase;
            i4Ret = GFX_DifGetRegBase(&u4GfxRegBase);
            VERIFY(i4Ret == (INT32)E_GFX_OK);

            _prRegBase = (volatile MI_DIF_UNION_T *)u4GfxRegBase;
        }
    //-----------------------------------------------------------
    #endif  //#if defined(GFX_ENABLE_SW_MODE)
#endif  // 0
   for (i4=0; i4<GFX_HAL_HW_INST_NUM; i4++) 
   {
     i4Ret =  GFX_CmdQueReset(i4);
   }
     return i4Ret;
}
//-------------------------------------------------------------------------
/** GFX_CmdQueUninit
 *  Uninit gfx command queue
 * 
 */
//-------------------------------------------------------------------------
INT32 GFX_CmdQueUninit(uint32_t u4GfxHwId)
{
    INT32 i4Ret;
    uint32_t u4GfxRegBase;

    // sw mode
    if ((uint32_t)E_GFX_SW_MOD == GFX_DifGetData(u4GfxHwId)->u4GfxMode)
    {
        i4Ret = GFX_DifGetRegBase(u4GfxHwId, &u4GfxRegBase);
        VERIFY(i4Ret == (INT32)E_GFX_OK);

        Gfx_CmdQueVar[u4GfxHwId].prRegBase= (volatile MI_DIF_UNION_T *)u4GfxRegBase;
    }

    if (_i4GfxCmdqueBufExist == (INT32)TRUE)
    {
    #if 1//CONFIG_SYS_MEM_PHASE2
        GFX_FREE_MEM(Gfx_CmdQueVar[u4GfxHwId].pu8GfxCmdqueBuf);
    #elif CONFIG_SYS_MEM_PHASE3
        GFX_FREE_MEM(Gfx_CmdQueVar[u4GfxHwId].pu8GfxCmdqueBuf);
    #endif
        _i4GfxCmdqueBufExist = (INT32)FALSE;
        //_i4GfxCqFireCnt = 0;
    }
     return (INT32)E_GFX_OK;
}


VOID  GFX_CmdQueSetFlushAllFlag(uint32_t u4GfxHwId, uint32_t ui4FlushAllFlag)
{
	if (!_rGfxCmdQue[u4GfxHwId].bNeedFlushAll)
	{
		_rGfxCmdQue[u4GfxHwId].bNeedFlushAll = (BOOL)ui4FlushAllFlag;
	}
}


//-------------------------------------------------------------------------
/** GFX_CmdQueReset
 *  reset gfx command queue
 *  set command queue info includes
 *  que capacity, que occupied size, idle flag, que top, read/write index
 *  hardware command que mode
 */
//-------------------------------------------------------------------------
INT32 GFX_CmdQueReset(uint32_t u4GfxHwId)
{
    //_rGfxCmdQue.i4QueCapacity = E_GFX_CQ_QCAPACITY;
    _rGfxCmdQue[u4GfxHwId].i4QueCapacity = Gfx_CmdQueVar[u4GfxHwId].i4GfxCqCapacity;
     _rGfxCmdQue[u4GfxHwId].i4QueSize     = 0;
     _rGfxCmdQue[u4GfxHwId].i4Idle        = 1;
     _rGfxCmdQue[u4GfxHwId].i4ReadIndex   = 0;
     _rGfxCmdQue[u4GfxHwId].i4WriteIndex  = 0;
	 _rGfxCmdQue[u4GfxHwId].bNeedFlushAll = FALSE;

     _rGfxCmdQue[u4GfxHwId].i4ShortCmdque = (Gfx_CmdQueVar[u4GfxHwId].i4GfxCqCapacity  < (INT32)EGFX_CPT_32KB);

    switch (Gfx_CmdQueVar[u4GfxHwId].i4GfxCqCapacity )
    {
    case (INT32)EGFX_CPT_256KB:
         _rGfxCmdQue[u4GfxHwId].i4CqSizeCfg = (INT32)EGFX_CQCFG_256KB; break;
    case (INT32)EGFX_CPT_128KB:
         _rGfxCmdQue[u4GfxHwId].i4CqSizeCfg = (INT32)EGFX_CQCFG_128KB; break;
    case (INT32)EGFX_CPT_64KB:
         _rGfxCmdQue[u4GfxHwId].i4CqSizeCfg = (INT32)EGFX_CQCFG_64KB; break;
    case (INT32)EGFX_CPT_32KB:
         _rGfxCmdQue[u4GfxHwId].i4CqSizeCfg = (INT32)EGFX_CQCFG_32KB; break;
    case (INT32)EGFX_CPT_16KB:
         _rGfxCmdQue[u4GfxHwId].i4CqSizeCfg = (INT32)EGFX_CQCFG_16KB; break;
    case (INT32)EGFX_CPT_8KB:
         _rGfxCmdQue[u4GfxHwId].i4CqSizeCfg = (INT32)EGFX_CQCFG_8KB; break;
    case (INT32)EGFX_CPT_4KB:
         _rGfxCmdQue[u4GfxHwId].i4CqSizeCfg = (INT32)EGFX_CQCFG_4KB; break;
    case (INT32)EGFX_CPT_2KB:
         _rGfxCmdQue[u4GfxHwId].i4CqSizeCfg = (INT32)EGFX_CQCFG_2KB; break;
    default:
        break;
    }

     _rGfxCmdQue[u4GfxHwId].pu8QueTop     = (volatile UINT64 *)
                                ((unsigned long)Gfx_CmdQueVar[u4GfxHwId].pu8GfxCmdqueBuf);
     _rGfxCmdQue[u4GfxHwId].prNext        = (struct _GFX_CMDQUE_T *)NULL;



#if defined(GFX_ENABLE_SW_MODE)
    // sw mode setting
    if ((uint32_t)E_GFX_SW_MOD == GFX_DifGetData(u4GfxHwId)->u4GfxMode)
    {
    #ifdef GFX_RISC_MODE
        Gfx_CmdQueVar[u4GfxHwId].prRegBase->rField.fg_EN_DRAMQ = GFX_DISABLE;
    #else
        Gfx_CmdQueVar[u4GfxHwId].prRegBase->rField.fg_EN_DRAMQ = GFX_ENABLE;
    #endif

        Gfx_CmdQueVar[u4GfxHwId].prRegBase->rField.fg_DRAMQ_MODE = GFX_CMD_BUF_CYLIC;
        Gfx_CmdQueVar[u4GfxHwId].prRegBase->rField.fg_CYC_SIZE   =   _rGfxCmdQue[u4GfxHwId].i4CqSizeCfg;

        // Sw must use VIRTUAL address
        Gfx_CmdQueVar[u4GfxHwId].prRegBase->rField.fg_DRAMQ_BSAD = ((unsigned long)_rGfxCmdQue[u4GfxHwId].pu8QueTop)&0x3fffffff;

        // power saving mode (need to measure HW)
        //_prRegBase->rField.fg_ENG_LP = GFX_ENABLE;

        ///??
        // enable/disable short cmdque mode
        #if 0//(!CONFIG_DRV_VERIFY_SUPPORT)
        Gfx_CmdQueVar[u4GfxHwId].prRegBase->rField.fg_SRAM_LP =   _rGfxCmdQue[u4GfxHwId].i4ShortCmdque;
        #endif
    }
#endif  //#if defined(GFX_ENABLE_SW_MODE)

    // hw mode setting
    if ((uint32_t)E_GFX_HW_8520_MOD == GFX_DifGetData(u4GfxHwId)->u4GfxMode)
    {
        volatile uint32_t u4Value;

    #ifdef GFX_RISC_MODE
        // _prRegBase->rField.fg_EN_DRAMQ = GFX_DISABLE;
        u4Value = GFX_READ32(u4GfxHwId, GFX_REG_G_CONFIG);
        u4Value = (u4Value) & (~GREG_MSK_EN_DRAMQ);
        GFX_WRITE32(u4GfxHwId, GFX_REG_G_CONFIG, u4Value);
    #else
        // _prRegBase->rField.fg_EN_DRAMQ = GFX_ENABLE;
        u4Value = GFX_READ32(u4GfxHwId, GFX_REG_G_CONFIG);
        u4Value = (u4Value) | (GREG_MSK_EN_DRAMQ);
        GFX_WRITE32(u4GfxHwId, GFX_REG_G_CONFIG, u4Value);
    #endif
        GFX_MB();

        // _prRegBase->rField.fg_DRAMQ_MODE = GFX_CMD_BUF_CYLIC;
        u4Value = GFX_READ32(u4GfxHwId, GFX_REG_G_CONFIG);
        u4Value = (u4Value) & (~GREG_MSK_DRAMQ_MODE);
        GFX_WRITE32(u4GfxHwId, GFX_REG_G_CONFIG, u4Value);
        GFX_MB();

        // _prRegBase->rField.fg_CYC_SIZE   = _rGfxCmdQue.i4CqSizeCfg;
        // _prRegBase->rField.fg_DRAMQ_BSAD = (uint32_t)_rGfxCmdQue.pu8QueTop;
        // Hw must use PHYSICAL address
        u4Value = ((uint32_t)_rGfxCmdQue[u4GfxHwId].i4CqSizeCfg << GREG_SHT_CYC_SIZE) |
        			((unsigned long)_rGfxCmdQue[u4GfxHwId].pu8QueTop & 0x3FFFFFFF);
                  //PHYSICAL(((uint32_t)_rGfxCmdQue[u4GfxHwId].pu8QueTop));
        GFX_WRITE32(u4GfxHwId, GFX_REG_DRAMQ_STAD, u4Value);

        // power saving mode (need to measure HW)
        //_prRegBase->rField.fg_ENG_LP = GFX_ENABLE;

        ///??
        // enable/disable short cmdque mode
        // _prRegBase->rField.fg_SRAM_LP = _rGfxCmdQue.i4ShortCmdque;
        u4Value = GFX_READ32(u4GfxHwId, GFX_REG_G_CONFIG);
        u4Value = (u4Value) |
                  ((uint32_t)_rGfxCmdQue[u4GfxHwId].i4ShortCmdque << GREG_SHT_SRAM_LP);
        GFX_WRITE32(u4GfxHwId, GFX_REG_G_CONFIG, u4Value);
        GFX_MB();
    }

    return (INT32)E_GFX_OK;
}


#ifdef GFX_QUE_REG
static uint32_t  gaui4MoniterQueue[1024*10];
static uint32_t  gui4MoniterQueueCount = 0;
static uint32_t  gui4MoniterQueueCountPrev = 0;

extern void GFX_MoniterQueueReset()
{
    gui4MoniterQueueCountPrev = gui4MoniterQueueCount;
    gui4MoniterQueueCount = 0;
}


extern void GFX_PrintMoniterInfo(void)
{
    uint32_t   ui4_idx;

    LOG(0, "\n\n[GFX] Moniter queue info**************\n");
    LOG(0, "\n[GFX] gui4MoniterQueueCountPrev = %d\n", gui4MoniterQueueCountPrev);

    for (ui4_idx = 0; ui4_idx<gui4MoniterQueueCountPrev; ui4_idx++)
    {
        LOG(0, "[GFX] 0x%x | ", gaui4MoniterQueue[ui4_idx*2]);
        LOG(0, "0x%x\n", gaui4MoniterQueue[ui4_idx*2 +1]);
    }
}
#endif
//-------------------------------------------------------------------------
/** GFX_CmdQuePushBack
 *  push back one register value into queue
 *  @param u4Reg indicates which register
 *  @param u4Val indicates the value of the register
 */
//-------------------------------------------------------------------------
INT32 GFX_CmdQuePushBack(uint32_t u4GfxHwId, uint32_t u4Reg, uint32_t u4Val)
{
    INT32 i4Ret = (INT32)E_GFX_OK;


    // for debug use
    ASSERT(_rGfxCmdQue[u4GfxHwId].pu8QueTop != NULL);
    ASSERT(_rGfxCmdQue[u4GfxHwId].i4QueCapacity != 0);

    if ((_rGfxCmdQue[u4GfxHwId].i4QueSize + GFX_CMD_MARGIN) >= _rGfxCmdQue[u4GfxHwId].i4QueCapacity)
    {
        i4Ret = GFX_CmdQueAction(u4GfxHwId);
        if (i4Ret)
        {
            return i4Ret;
        }
    }

    // lock cmdque resource
  // GFX_LockCmdque(u4GfxHwId);

    // calculate GFX register address
     u4Reg = ( (unsigned long)gfx_base | ((u4Reg << 2) & 0xfff));
    // release cmdque resource
 //GFX_UnlockCmdque(u4GfxHwId);


#if 0//D_GFX_CQ_HW_INT_MODE

    GFX_Wait();
    *((uint32_t*)u4Reg) = u4Val;
    
#else


    // write one cmd to cmdque buffer
    _rGfxCmdQue[u4GfxHwId].i4QueSize++;

    _rGfxCmdQue[u4GfxHwId].pu8QueTop[_rGfxCmdQue[u4GfxHwId].i4WriteIndex++] =
        ((((UINT64)(u4Reg)) << 32) | u4Val);

    #ifdef GFX_QUE_REG
    gaui4MoniterQueue[gui4MoniterQueueCount * 2] = u4Val;
    gaui4MoniterQueue[gui4MoniterQueueCount* 2 + 1] = u4Reg;
    gui4MoniterQueueCount++;
    #endif

    if (_rGfxCmdQue[u4GfxHwId].i4WriteIndex >= _rGfxCmdQue[u4GfxHwId].i4QueCapacity)
    {
        _rGfxCmdQue[u4GfxHwId].i4WriteIndex = 0;
    }
    
#if CONFIG_LOSSLESS_COMPRESS_AUTO_FLIP_MODE
    if (((_rGfxCmdQue[u4GfxHwId].i4QueSize) >= (_rGfxCmdQue[u4GfxHwId].i4QueCapacity>>1)) && (u4Reg == 0xFD004010) && ((u4Val & 0x800) == 0x800))
    {
        i4Ret = GFX_CmdQueAction(u4GfxHwId);
        if (i4Ret)
        {
            return i4Ret;
        }
    }
#endif

#if 1//(CONFIG_DRV_VERIFY_SUPPORT)
     if (((uint32_t)E_GFX_SW_MOD == GFX_DifGetData(u4GfxHwId)->u4GfxMode) &&
         ((u4Reg == (uint32_t)0xFD004010) && ((u4Val & GREG_MSK_FIRE) == GREG_MSK_FIRE)))
     {
         i4Ret = GFX_CmdQueAction(u4GfxHwId);
         if (i4Ret)
         {
             return i4Ret;
         }
     }   
#endif

 /*   
    i4Ret=BSP_GetIcVersion();
    if (IC_VER_A == i4Ret)
    {
        if (0x70004010 == u4Reg)
        {
            if (u4Val & 0x800)
            {
                _i4GfxCqFireCnt++;
            }
            if (_i4GfxCqFireCnt >= GFX_CMD_MAX_FIRE)
            {
                i4Ret = GFX_CmdQueAction();
                if (i4Ret)
                {
                    return i4Ret;
                }
            }
        }
    }
*/
#endif // #if D_GFX_CQ_HW_INT_MODE

    return (INT32)E_GFX_OK;
}


//-------------------------------------------------------------------------
/** GFX_CmdQueDbgInfo
 *  dump previously executed gfx command queue
 *  debug dump directly to console rs232 port
 *  start from previous index (in gfx command queue structure)
 *  till write index
 */
//-------------------------------------------------------------------------
void GFX_CmdQueDbgInfo(uint32_t u4GfxHwId)
{
    LOG(5, "\ngfx cmdq dump - begin\n");

    LOG(5, "\t_rGfxCmdQue.i4QueCapacity = %d\n", _rGfxCmdQue[u4GfxHwId].i4QueCapacity);
    LOG(5, "\t_rGfxCmdQue.i4QueSize = %d\n", _rGfxCmdQue[u4GfxHwId].i4QueSize);
    LOG(5, "\t_rGfxCmdQue.i4PrevIndex = 0x%08x\n", _rGfxCmdQue[u4GfxHwId].i4PrevIndex);
    LOG(5, "\t_rGfxCmdQue.i4ReadIndex = 0x%08x\n", _rGfxCmdQue[u4GfxHwId].i4ReadIndex);
    LOG(5, "\t_rGfxCmdQue.i4WriteIndex = 0x%08x\n", _rGfxCmdQue[u4GfxHwId].i4WriteIndex);
    LOG(5, "\t_rGfxCmdQue.i4Idle = %d\n", _rGfxCmdQue[u4GfxHwId].i4Idle);
    LOG(5, "\t_rGfxCmdQue.pu8QueTop = 0x%08x\n", _rGfxCmdQue[u4GfxHwId].pu8QueTop);

    LOG(5, "gfx cmdq dump - end\n\n");
}


//-------------------------------------------------------------------------
/** GFX_CmdQueSetCqCapacity
 *  gfx set cmdque capacity
 */
//-------------------------------------------------------------------------
void GFX_CmdQueSetCqCapacity(uint32_t u4GfxHwId, INT32 i4Capacity)
{
    Gfx_CmdQueVar[u4GfxHwId].i4GfxCqCapacity  = i4Capacity;
}

//#define GFX_POWER_SAVE_SUPPORT

#ifdef GFX_QUE_REG
static uint32_t gui4QueueSize = 0;
static uint32_t gui4PreExt=0;
#endif

INT32 GFX_CmdQueCheckSize(uint32_t u4GfxHwId)
{
    INT32 i4Ret = E_GFX_OK;

    // # of command must be even
    if (_rGfxCmdQue[u4GfxHwId].i4QueSize & 1)
    {
        i4Ret = GFX_CmdQuePushBack(u4GfxHwId, (INT32)(GREG_DUMMY), 0);
    }

    return (i4Ret);
}

//-------------------------------------------------------------------------
/** GFX_CmdQueAction
 *  start gfx hardware command queue
 */
//-------------------------------------------------------------------------
extern BOOL _fgGfxDelay[2];
INT32 GFX_CmdQueAction(uint32_t u4GfxHwId)
{
    volatile uint32_t u4CmdQueLen;
    INT32 i4Ret = E_GFX_OK;

#ifndef GFX_RISC_MODE
    if (!GFX_DifGetIdle(u4GfxHwId))
    {
        GFX_Wait();
    }
    GFX_DifSetIdle(u4GfxHwId, FALSE);        // set system to busy

    if (_rGfxCmdQue[u4GfxHwId].i4QueSize < GFX_CMQ_MIN_SIZE)
    {
        INT32 i4Ret;
        INT32 i4Count;
        INT32 i4Times;

        i4Times = ((INT32)GFX_CMQ_MIN_SIZE - _rGfxCmdQue[u4GfxHwId].i4QueSize);

        for (i4Count = 0; i4Count < i4Times; i4Count++)
        {
            i4Ret = GFX_CmdQuePushBack(u4GfxHwId, (INT32)GREG_DUMMY, 0);
            if (i4Ret)
            {
                return i4Ret;
            }
        }
    }

    // # of command must be even
    if (_rGfxCmdQue[u4GfxHwId].i4QueSize & 1)
    {
        i4Ret = GFX_CmdQuePushBack(u4GfxHwId, (INT32)(GREG_DUMMY), 0);
        if (i4Ret)
        {
            return i4Ret;
        }
    }

    // hw mode
    if ((uint32_t)E_GFX_HW_8520_MOD == GFX_DifGetData(u4GfxHwId)->u4GfxMode)
    {
        //GFX_INC_DRV_FLUSH_COUNT(u4GfxHwId);  // for debug use

        // lock cmdque resource
        // (release cmdque resource in _GfxHwRealIsr())
#if !(CONFIG_DRV_LINUX) || (1 == GFX_SYNC_OLD_METHOD)
       GFX_LockCmdque(u4GfxHwId); //qing li modify it and use event GFX_EV_HW_DONE instead
#endif
        //add by msz00441 07-12-14 for rle sync
       // if(RleFlushFlag==1)
       // {
       //     RleFlushFlag=2;
        //}
        #ifdef GFX_QUE_REG
		1
        uint32_t ui4i=0;
        for(ui4i=0;ui4i<80;ui4i++)
        {
            //GfxRegOld[ui4i]= GfxRegOld[ui4i];
            GfxReg[ui4i]= GFX_READ32(4*ui4i);
        }
        #endif
        ///??
        //HalFlushInvalidateDCache();
        //HalFlushDCache();
        //BSP_FlushDCacheRange((uint32_t)(_rGfxCmdQue[u4GfxHwId].pu8QueTop + _rGfxCmdQue[u4GfxHwId].i4ReadIndex), _rGfxCmdQue[u4GfxHwId].i4QueSize*GFX_ONE_CMD_SIZE);
        /*if (!_rGfxCmdQue[u4GfxHwId].bNeedFlushAll)
        {
            BSP_FlushDCacheRange((uint32_t)(_rGfxCmdQue[u4GfxHwId].pu8QueTop), _rGfxCmdQue[u4GfxHwId].i4QueCapacity*GFX_ONE_CMD_SIZE);
        }
        else
        {
            HalFlushInvalidateDCache();
        }*/
        //ATC68024 --dont find symbol:BSP_FlushDCacheRange
        //BSP_FlushDCacheRange((uint32_t)(_rGfxCmdQue[u4GfxHwId].pu8QueTop), _rGfxCmdQue[u4GfxHwId].i4QueCapacity*GFX_ONE_CMD_SIZE);
      //  GFX_SAVE_FLUSH_TIME();         // for debug use
        ++Gfx_CmdQueVar[u4GfxHwId].u4GfxHwFlushCount;          // for debug use
        UNUSED(Gfx_CmdQueVar[u4GfxHwId].u4GfxHwFlushCount);    // for lint happy

        #ifdef GFX_POWER_SAVE_SUPPORT
        // Wakeup from Power Save Mode
        GFX_WRITE32(u4GfxHwId, GFX_REG_G_MODE, (GFX_READ32(u4GfxHwId, GFX_REG_G_MODE)|(0x1F << GREG_SHT_OP_MODE)));
        GFX_WRITE32(u4GfxHwId, GFX_REG_G_CONFIG, (GFX_READ32(u4GfxHwId, GFX_REG_G_CONFIG)|(1 << GREG_SHT_EN_DRAMQ)));
        #endif
        
        //LOG(0, "[GFX] ------ Flush it.");
        // in hw mode, gfx engine will run if DRAMQ_LEN is written.
        // 1 CMD size = 8 bytes
        //u4CmdQueLen = (uint32_t)(_rGfxCmdQue[u4GfxHwId].i4QueSize * GFX_ONE_CMD_SIZE);
        // [YgLi][20090211]SRC_CM use new method.

#ifdef GFX_DRV_LOG
        switch (_ui1_test_type)
        {
             case 1:
             {
                 if (ui4_s50_test_count < 200)
                 {
                     _t_cmd_queue_s50_test_struct[ui4_s50_test_count].ui1_hw_id = u4GfxHwId;
                     _t_cmd_queue_s50_test_struct[ui4_s50_test_count].ui4_cmd_data = _rGfxCmdQue[u4GfxHwId].i4QueSize;
                     HAL_GetTime(&_t_cmd_queue_s50_test_struct[ui4_s50_test_count].t_start_time);

                 }
                 break;
             }
 
             case 2:
             {
                 if (ui4_s100_test_count < 200)
                 {
                     _t_cmd_queue_s100_test_struct[ui4_s100_test_count].ui1_hw_id = u4GfxHwId;
                     _t_cmd_queue_s100_test_struct[ui4_s100_test_count].ui4_cmd_data = _rGfxCmdQue[u4GfxHwId].i4QueSize;
                     HAL_GetTime(&_t_cmd_queue_s100_test_struct[ui4_s100_test_count].t_start_time);

                 }
                 break;
             }
 
             case 3:
             {
                 if (ui4_sc100_test_count < 200)
                 {
                     _t_cmd_queue_sc100_test_struct[ui4_sc100_test_count].ui1_hw_id = u4GfxHwId;
                     _t_cmd_queue_sc100_test_struct[ui4_sc100_test_count].ui4_cmd_data = _rGfxCmdQue[u4GfxHwId].i4QueSize;
                     HAL_GetTime(&_t_cmd_queue_sc100_test_struct[ui4_sc100_test_count].t_start_time);

                 }
                 break;
             }
 
             case 4:
             {
                 if (ui4_ea100_test_count < 200)
                 {
                     _t_cmd_queue_ea100_test_struct[ui4_ea100_test_count].ui1_hw_id = u4GfxHwId;
                     _t_cmd_queue_ea100_test_struct[ui4_ea100_test_count].ui4_cmd_data = _rGfxCmdQue[u4GfxHwId].i4QueSize;
                     HAL_GetTime(&_t_cmd_queue_ea100_test_struct[ui4_ea100_test_count].t_start_time);

                 }
                 break;
             }
 
             case 5:
             {
                 if (ui4_s200_test_count < 200)
                 {
                     _t_cmd_queue_s200_test_struct[ui4_s200_test_count].ui1_hw_id = u4GfxHwId;
                     _t_cmd_queue_s200_test_struct[ui4_s200_test_count].ui4_cmd_data = _rGfxCmdQue[u4GfxHwId].i4QueSize;
                     HAL_GetTime(&_t_cmd_queue_s200_test_struct[ui4_s200_test_count].t_start_time);

                 }
                 break;
             }
 
             default:
                 break;
        }
#endif

        u4CmdQueLen = (uint32_t)(_rGfxCmdQue[u4GfxHwId].i4QueSize * GFX_ONE_CMD_SIZE) | 0x10000000;
#ifdef GFX_PERFORMANCE_TEST
#include "x_timer.h"
        if (g_fgGfxPfmTesting)
        {
            HAL_GetTime(&g_rGfxStartTime);
			UTIL_Printf("	   g_rGfxStartTime.u4Seconds = %d, g_rGfxStartTime.u4Micros = %d\n", g_rGfxStartTime.u4Seconds, g_rGfxStartTime.u4Micros);
        }
#endif
#if 0//(CONFIG_DRV_VERIFY_SUPPORT)
    {
        uint32_t u4i;
        uint32_t *u4GfxCmqTop;
        u4GfxCmqTop = (uint32_t *)_rGfxCmdQue[u4GfxHwId].pu8QueTop;
        for(u4i=0;u4i<_rGfxCmdQue[u4GfxHwId].i4QueSize*2;u4i++)
        {
            _u4GfxCmdQueBuf[u4i]=*(u4GfxCmqTop+u4i);
            if (u4i == 4096)u4i = 0;
        }
        for (u4i=0; u4i<3; u4i++)
        {
            _u4GfxRegistBuf[u4i] = GFX_READ32(u4GfxHwId, u4i*4);
        }
        _u4GfxRegistBuf[3] = u4CmdQueLen;
    }
#endif
        GFX_WRITE32(u4GfxHwId, GFX_REG_DRAMQ_LEN, u4CmdQueLen);

        #if 1
                 GFX_LockCmdque(u4GfxHwId);
        #if !(CONFIG_DRV_LINUX) || (1 == GFX_SYNC_OLD_METHOD)
		1
                 GFX_UnlockCmdque(u4GfxHwId);
        #endif
        #endif

        #ifdef GFX_QUE_REG
          gui4PreExt = _rGfxCmdQue.i4PrevIndex;
        #endif
        _rGfxCmdQue[u4GfxHwId].i4PrevIndex = _rGfxCmdQue[u4GfxHwId].i4ReadIndex;
        _rGfxCmdQue[u4GfxHwId].i4ReadIndex = _rGfxCmdQue[u4GfxHwId].i4WriteIndex;
        #ifdef GFX_QUE_REG
        gui4QueueSize = _rGfxCmdQue[u4GfxHwId].i4QueSize;
        #endif
        _rGfxCmdQue[u4GfxHwId].i4QueSize   = 0;

        _rGfxCmdQue[u4GfxHwId].bNeedFlushAll = FALSE;
		
        #ifdef GFX_POWER_SAVE_SUPPORT
        // Enter Power Save Mode
        GFX_WRITE32(u4GfxHwId, GFX_REG_G_CONFIG, (GFX_READ32(u4GfxHwId, GFX_REG_G_CONFIG)&(~(1 << GREG_SHT_EN_DRAMQ))));
        GFX_WRITE32(u4GfxHwId, GFX_REG_G_MODE, (GFX_READ32(u4GfxHwId, GFX_REG_G_MODE)&(~(0x1F << GREG_SHT_OP_MODE))));
        #endif

        //
        #if 0//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8550)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
        if (GFX_HwGetBpCompStopID(u4GfxHwId, NULL))
        {
            i4Ret = -(INT32)E_GFX_BPCOMP_STOP;
        }
        #endif

		//i4GfxSetMmuSelfFire(u4GfxHwId);
    }

#endif // #if D_GFX_CQ_HW_INT_MODE
#if defined(GFX_ENABLE_SW_MODE)
    // sw mode
    if ((uint32_t)E_GFX_SW_MOD == GFX_DifGetData(u4GfxHwId)->u4GfxMode)
    {
      //  GFX_INC_CM_FLUSH_COUNT(u4GfxHwId);  // for debug use

        //HalFlushInvalidateDCache();
        //ATC68024 --dont find symbol:BSP_FlushDCacheRange
        //BSP_FlushDCacheRange((uint32_t)(_rGfxCmdQue[u4GfxHwId].pu8QueTop), _rGfxCmdQue[u4GfxHwId].i4QueCapacity*GFX_ONE_CMD_SIZE);

        ++Gfx_CmdQueVar[u4GfxHwId].u4GfxSwFlushCount;           // for debug use
        UNUSED(Gfx_CmdQueVar[u4GfxHwId].u4GfxSwFlushCount);     // for lint happy

        // in sw mode, just record cmdque len for _GfxSwQueAction() use
        // 1 CMD size = 8 bytes
        u4CmdQueLen = (uint32_t)( _rGfxCmdQue[u4GfxHwId].i4QueSize * GFX_ONE_CMD_SIZE);
        Gfx_CmdQueVar[u4GfxHwId].prRegBase->rField.fg_DRAMQ_LEN = u4CmdQueLen;

        _GfxSwQueAction(u4GfxHwId);

        _rGfxCmdQue[u4GfxHwId].i4PrevIndex = _rGfxCmdQue[u4GfxHwId].i4ReadIndex;
        _rGfxCmdQue[u4GfxHwId].i4ReadIndex = _rGfxCmdQue[u4GfxHwId].i4WriteIndex;
        _rGfxCmdQue[u4GfxHwId].i4QueSize   = 0;

        ///??
        GFX_HwISR(u4GfxHwId);    // notify MW

        #if !(CONFIG_DRV_LINUX) || (1 == GFX_SYNC_OLD_METHOD)
         //GFX_UnlockCmdque(u4GfxHwId);
        #endif
    }
#endif  //#if defined(GFX_ENABLE_SW_MODE)

return (INT32)i4Ret;
}

#ifdef GFX_QUE_REG
extern void Gfx_PrintCmdQueue(void)
{
    uint32_t  * pui4_data;
    uint32_t  * pui4_data_old;
    uint32_t  ui4_idx;

    pui4_data = (uint32_t *)(&_rGfxCmdQue.pu8QueTop[_rGfxCmdQue.i4PrevIndex]);
    pui4_data_old = (uint32_t *)(&_rGfxCmdQue.pu8QueTop[gui4PreExt]);
    LOG(0, "\n\n************* Queue info excuted **********\n");
    LOG(0, "Old_PrevReadIdx = 0x%x,PrevReadIdx = 0x%x",
        gui4PreExt, _rGfxCmdQue.i4PrevIndex);
    for (ui4_idx = 0; ui4_idx < _rGfxCmdQue.i4PrevIndex-gui4PreExt; ui4_idx ++)
    {
        LOG(0, "[GFX] 0x%x | ", pui4_data_old[ui4_idx*2]);
        LOG(0, "0x%x\n", pui4_data_old[ui4_idx*2 +1]);
           
    }

    LOG(0, "\n\n************* Queue info **********\n");
    LOG(0, "PrevReadIdx = 0x%x, ReadIdx = 0x%x, WriteIdx=0x%x\n ",
        _rGfxCmdQue.i4PrevIndex, _rGfxCmdQue.i4ReadIndex, 
        _rGfxCmdQue.i4WriteIndex);
    for (ui4_idx = 0; ui4_idx < gui4QueueSize; ui4_idx ++)
    {
        LOG(0, "[GFX] 0x%x | ", pui4_data[ui4_idx*2]);
        LOG(0, "0x%x\n", pui4_data[ui4_idx*2 +1]);
           
    }
    
}


void GFX_PrintGfxConfigure(void)
{
    uint32_t  ui4_idx;
    uint32_t  * pui4_reg_addr = NULL;

    LOG(0, "\n\n************* Register info **********\n");

    pui4_reg_addr = (uint32_t * )0x70004000;

    for (ui4_idx = 0; ui4_idx<80; ui4_idx++ )
    {
        LOG(0, "[GFX] 0x%x | 0x%x\n",
            pui4_reg_addr, *pui4_reg_addr);
        pui4_reg_addr ++;
    }
    LOG(0, "\n\n************Prev Register info **********\n");

    pui4_reg_addr = (uint32_t * )0x70004000;

    for (ui4_idx = 0; ui4_idx<80; ui4_idx++ )
    {
        LOG(0, "[GFX] 0x%x | 0x%x\n",
            pui4_reg_addr, GfxReg[ui4_idx]);
        pui4_reg_addr ++;
    }

    
}
#endif

//-------------------------------------------------------------------------
/** GFX_RiscPushBack (Risc Mode)
 *  push back one register value into gfx register
 *  @param u4Reg indicates which register
 *  @param u4Val indicates the value of the register
 */
//-------------------------------------------------------------------------
INT32 GFX_RiscPushBack(uint32_t u4GfxHwId, uint32_t u4Reg, uint32_t u4Val)
{
    INT32 i4Ret = (INT32)E_GFX_OK;
    uint32_t u4RegOffset;

    u4RegOffset = ((u4Reg << 2) & 0xfff);

    if ((uint32_t)E_GFX_SW_MOD == GFX_DifGetData(u4GfxHwId)->u4GfxMode)
    {
        GFX_CmdQuePushBack(u4GfxHwId, u4Reg, u4Val);
    }
    else if ((u4RegOffset == (uint32_t)GFX_REG_G_MODE)&& ((u4Val & GREG_MSK_FIRE) == GREG_MSK_FIRE)) 
    {
      //  GFX_INC_DRV_FLUSH_COUNT(u4GfxHwId);  // for debug use
        // lock cmdque resource
         if(GFX_LockCmdque(u4GfxHwId) !=0) 

     //   GFX_SAVE_FLUSH_TIME();      // for debug use

        // write a cmd to the hw register (flush)
        
        GFX_WRITE32(u4GfxHwId, u4RegOffset, u4Val);
		 msleep(5);
		 if (GFX_HwGetIdle(u4GfxHwId))
        {
      // printk("failed to trig hw trig again\n");
		 GFX_WRITE32(u4GfxHwId, u4RegOffset, u4Val);
		 
		 }
	 	//unlock in isr
       if(GFX_LockCmdque(u4GfxHwId) != 0) {
	   	printk("gfx trig hw return false\n");
		i4Ret = (INT32)E_GFX_UNDEF_ERR;
	   	}
        GFX_UnlockCmdque(u4GfxHwId);
		//return E_GFX_UNDEF_ERR;
        // ... wait for unlock fill command
    }
    else
    {
        // lock cmdque resource
        GFX_LockCmdque(u4GfxHwId);
        
        // write a cmd to the hw register
        #if 0//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)//if config MMU
        if ((u4RegOffset >= GREG_IOMMU_CFG0) && (u4RegOffset <= GREG_IOMMU_CFG11) && (0 == u4GfxHwId))
        {
            u4RegOffset += 0x5AEC0;
        }
        #endif
        GFX_WRITE32(u4GfxHwId, u4RegOffset, u4Val);

        // unlock cmdque resource
        GFX_UnlockCmdque(u4GfxHwId);
    }

    return i4Ret;
}


