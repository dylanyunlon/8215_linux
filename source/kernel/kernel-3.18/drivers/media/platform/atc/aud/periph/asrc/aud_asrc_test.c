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

/******************************************************************************
*[File]                   aud_test_asrc.c
*[Author]               tongfa.luo@autochips.com
*[Description]
*       
*[Copyright]
*       
******************************************************************************/

#include "aud_asrc_test.h"

u32 _u4AsrcLog = ALOG_DEFAULT;


#if (AUD_UNIT_TEST_SUPPORT)

static PASRC_TEST_CLS _aprAsrcTest[ASRC_TYPE_NUM] = {NULL};

//=======================================//
  #define CodeSight_AsrcTest_Others
//=======================================//

static void AsrcTest_IFileNew(PASRC_TEST_CLS prThis, u32 u4Idx, u32 u4ReadSize)
{
    PFILE_CLS_PUB prFile = NULL;
    PASRC_CHS_FMT_T prFmt = &prThis->arChsParam[u4Idx].rFmt;
    FILE_VIR_PARAMS_T rParams;

    rParams.u4BW = prFmt->u4IBW;
    rParams.u4Chn = prFmt->u4Chn;
    if (16 == rParams.u4BW) {
        rParams.u4ChBufSz = AUD_TBL_SINE_16BIT_SZ;
        rParams.u4TblSAdr = (u32)AUD_TBL_SINE_16BIT;
    } else {
        rParams.u4ChBufSz = AUD_TBL_SINE_24BIT_SZ;
        rParams.u4TblSAdr = (u32)AUD_TBL_SINE_24BIT;       
    }
    rParams.u4ReadLoops = u4ReadSize / rParams.u4ChBufSz;

    prFile = File_New(VIR_FILE);
    if (prFile) {
        prFile->Open(prFile, TRUE, &rParams);
        prThis->arChsParam[u4Idx].prIFile = prFile;
    }
}
  
  
static void AsrcTest_OFileNew(PASRC_TEST_CLS prThis, u32 u4Idx, u32 u4WriteSize)
{
    PFILE_CLS_PUB prFile = NULL;
    PASRC_CHS_FMT_T prFmt = &prThis->arChsParam[u4Idx].rFmt;

    if (prThis->fgUsePhyFile)
    {
        FILE_PHY_PARAMS_T rParams;

        sprintf(rParams.szFileName, "%sASRC%d_%d_%dch_%d_%dbit_%d_%dFS.bin" , 
          AUD_FILE_PATH, prThis->eType, (s32)u4Idx, (s32)(prFmt->u4Chn), 
          (s32)(prFmt->u4IBW), (s32)(prFmt->u4OBW), (s32)(prFmt->u4IFS/1000), (s32)(prFmt->u4OFS/1000));
    
        rParams.u4Chn = prFmt->u4Chn;
        rParams.u4ChBufSz = 1024;
        rParams.u4BW = prFmt->u4OBW;

        prFile = File_New(PHY_FILE);
        if (prFile) {
            prFile->Open(prFile, FALSE, &rParams);
        }
    }
    else
    {
        FILE_VIR_PARAMS_T rParams;
   
        rParams.u4Chn = prFmt->u4Chn;
        rParams.u4ChBufSz = u4WriteSize;
        rParams.u4BW = prFmt->u4OBW;

        prFile = File_New(VIR_FILE);
        prFile->Open(prFile, FALSE, &rParams);
    }  
    
    prThis->arChsParam[u4Idx].prOFile = prFile;
}


static void AsrcTest_FileDelete(PASRC_TEST_CHS_PARAM_T prChsParam)
{
    PFILE_CLS_PUB prIFile = prChsParam->prIFile;
    PFILE_CLS_PUB prOFile = prChsParam->prOFile;

    prIFile->Close(prIFile);
    prOFile->Close(prOFile);

    prIFile->Delete(prIFile);
    prOFile->Delete(prOFile);
}


//=======================================//
  #define CodeSight_AsrcTest_Chs_Data
//=======================================//

static void AsrcTest_InputProc(PASRC_TEST_CHS_PARAM_T prChsParam)
{
    AUD_DATA_BUF_T rDstBuf;
    PASRC_CHS_CLS_PUB prChs = prChsParam->prChs;
    PFILE_CLS_PUB prFile = prChsParam->prIFile;

    prChs->rHwIf.GetIBuf(prChs, &rDstBuf);
    if (prFile->Read(prFile, &rDstBuf)) {
        prChs->rHwIf.SetIWP(prChs, rDstBuf.u4DataOff);
    } else {
        prChs->rHwIf.Stop(prChs, 0);
        AsrcTest_FileDelete(prChsParam);
    }    
}


static void AsrcTest_OutputProc(PASRC_TEST_CHS_PARAM_T prChsParam)
{
    AUD_DATA_BUF_T rSrcBuf;
    PASRC_CHS_CLS_PUB prChs = prChsParam->prChs;
    PFILE_CLS_PUB prFile = prChsParam->prOFile;

    prChs->rHwIf.GetOBuf(prChs, &rSrcBuf);
    if (prFile->Write(prFile, &rSrcBuf)) {
        prChs->rHwIf.SetORP(prChs, rSrcBuf.u4DataOff);   
    } else {
        prChs->rHwIf.Stop(prChs, 0);
        AsrcTest_FileDelete(prChsParam);
    }    
}  


static s32 AsrcTest_ISTCB(u32 u4ChsParam, u32 u4IntType)
{
    PASRC_TEST_CHS_PARAM_T prChsParam = (PASRC_TEST_CHS_PARAM_T)u4ChsParam;
 
    if (u4IntType & (ASRC_IBUF_AMOUNT_INT | ASRC_IBUF_EMPTY_INT))
    {
        AsrcTest_InputProc(prChsParam);
    }
    else if (u4IntType & (ASRC_OBUF_AMOUNT_INT | ASRC_OBUF_OV_INT))
    {
        AsrcTest_OutputProc(prChsParam);
    }
    
    return (0);
}


//=======================================//
  #define CodeSight_AsrcTest_Chs_Ctrl
//=======================================//

static void AsrcTest_ChsSetup(PASRC_TEST_CLS prThis, u32 u4ChsIdx,
    PASRC_CHS_FMT_T prFmtParams, u32 u4ReadSize, u32 u4WriteSize)
{
    ASRC_CHS_CFG_T rParams;
    ASRC_CHS_ISRCB_T rIsrCb;  
    PASRC_CHS_CLS_PUB prChs = NULL;
    PASRC_CHS_FMT_T prFmt = &prThis->arChsParam[u4ChsIdx].rFmt;

    x_memcpy(prFmt, prFmtParams, sizeof(ASRC_CHS_FMT_T));
    AsrcTest_IFileNew(prThis, u4ChsIdx, u4ReadSize);
    AsrcTest_OFileNew(prThis, u4ChsIdx, u4WriteSize);

    ASRCLOG_CLI(T("[Test%d]New Chs: Idx(%d) Chn(%d) IFS(%d) OFS(%d) IBW(%d) OBW(%d) eIFS(%d) eOFS(%d) RSz(%d) WSz(%d)\n"),
        prThis->eType, (s32)u4ChsIdx, (s32)(prFmt->u4Chn), (s32)(prFmt->u4IFS), (s32)(prFmt->u4OFS),  
        (s32)(prFmt->u4IBW), (s32)(prFmt->u4OBW), (s32)(prFmt->u4IPalette), (s32)(prFmt->u4OPalette),(s32)(u4ReadSize), (s32)(u4WriteSize));
         
    rIsrCb.pfnCb = AsrcTest_ISTCB;
    rIsrCb.u4Param = (u32)(&prThis->arChsParam[u4ChsIdx]);
    rIsrCb.u4IntType = ASRC_IBUF_EMPTY_INT | ASRC_OBUF_OV_INT;
    
    prChs = prThis->prMgr->AllocAsrc(prThis->prMgr, FALSE);    
    prThis->arChsParam[u4ChsIdx].prChs = prChs;
    
    rParams.prIsrCb = &rIsrCb;
    rParams.prFmt = prFmt;
    prChs->rHwIf.Setup(prChs, &rParams);
}


static void AsrcTest_ChsSetupCmd(u32 *pu4Argv, u32 u4Argc)
{
    PASRC_TEST_CLS prThis = NULL;   
    ASRC_CHS_FMT_T rFmt;
    u32 u4Type, u4Idx, u4ITime, u4OTime, u4ReadSize, u4WriteSize;

    if (u4Argc == 0) {
        ASRCLOG_CLI(T("New Chs: eAsrcType ChsIdx Chn IFS OFS IBW OBW IPalette OPalette ITime OTime \n"));
    }
    
    u4Type = (u4Argc > 0) ? (*pu4Argv++) : GPS_ASRC; 
    prThis = (u4Type < ASRC_TYPE_NUM) ? _aprAsrcTest[u4Type] : _aprAsrcTest[GPS_ASRC];
    
    u4Idx  = (u4Argc > 1) ? (*pu4Argv++) : 0;  
     
    rFmt.u4Chn = (u4Argc > 2) ? (*pu4Argv++) : 1;
    rFmt.u4IFS = (u4Argc > 3) ? (*pu4Argv++) : 32000;
    rFmt.u4OFS = (u4Argc > 4) ? (*pu4Argv++) : 48000;
    rFmt.u4IBW = (u4Argc > 5) ? (*pu4Argv++) : 16;
    rFmt.u4OBW = (u4Argc > 6) ? (*pu4Argv++) : 16;
    rFmt.u4IPalette = (u4Argc > 7) ? (*pu4Argv++) : ASRC_PALETTE_NUM;
    rFmt.u4OPalette = (u4Argc > 8) ? (*pu4Argv++) : ASRC_PALETTE_NUM;

    u4ITime = (u4Argc > 9) ? (*pu4Argv++) : 500;
    u4OTime = (u4Argc > 10) ? (*pu4Argv++) : 500;
    u4ReadSize = rFmt.u4IFS * (rFmt.u4IBW >> 3) * u4ITime / 1000;       
    u4WriteSize = rFmt.u4OFS * (rFmt.u4OBW >> 3) * u4OTime / 1000;

    AsrcTest_ChsSetup(prThis, u4Idx, &rFmt, u4ReadSize, u4WriteSize);   
}


static void AsrcTest_ChsStart(PASRC_TEST_CLS prThis, u32 u4ChsIdx)
{
    PASRC_CHS_CLS_PUB prChs = prThis->arChsParam[u4ChsIdx].prChs;
    if (prChs) {
        prChs->rHwIf.Start(prChs, 0);
    }
}


static void AsrcTest_ChsStop(PASRC_TEST_CLS prThis, u32 u4ChsIdx)
{
    PASRC_CHS_CLS_PUB prChs = prThis->arChsParam[u4ChsIdx].prChs;
    if (prChs) {
        prChs->rHwIf.Stop(prChs, 0);
        AsrcTest_FileDelete(&prThis->arChsParam[u4ChsIdx]);
    }
}


//=======================================//
  #define CodeSight_AsrcTest_Create
//=======================================//
u32 AsrcTest_Delete(PASRC_TEST_CLS prThis)
{
    if (prThis) 
    {
        prThis->prMgr->Delete(prThis->prMgr);

        ASRCLOG_CLI(T("Delete Test: %d -> 0x%x \n"), prThis->eType, (u32)prThis);
        AUD_CLASS_DELETE();
    }
    return (TRUE);
}


PASRC_TEST_CLS AsrcTest_New(ASRC_CLS_TYPE eType, u32 u4IChSz, u32 u4OChSz, bool fgUsePhyFile)
{
    PASRC_TEST_CLS prThis = AUD_CLASS_NEW(ASRC_TEST_CLS);
    
    eType = (eType < ASRC_TYPE_NUM) ? eType : GPS_ASRC;
    if (prThis) 
    {
        u32 i;
        prThis->eType = eType;
        prThis->prMgr = AsrcMgr_New(eType, u4IChSz, u4OChSz);
        prThis->fgUsePhyFile = fgUsePhyFile;

        for(i = 0; i < ASRC_CHSET_NUM; i++)  { 
            prThis->arChsParam[i].prChs = NULL;
        }       
    }

    if (_aprAsrcTest[eType] != NULL) {
        AsrcTest_Delete(_aprAsrcTest[eType]);
    }
    _aprAsrcTest[eType] = prThis;
    
    ASRCLOG_CLI(T("New Test: %d->0x%x, IChSz(%d) OChSz(%d) UsePhyFile(%d)\n"), 
        eType, (u32)prThis, (s32)u4IChSz, (s32)u4OChSz, (s32)fgUsePhyFile);

    return (prThis);
}


void AsrcTest_NewCmd(u32 *pu4Argv, u32 u4Argc)
{
    u32 u4Type, u4IChSz, u4OChSz, u4PhyFile;

    if (u4Argc == 0) 
    {
        ASRCLOG_CLI(T("New Test: eAsrcType InputChSz OutputChSz UsePhyFile\n"));
    }
    else 
    {
        u4Type    = (u4Argc > 0) ? (*pu4Argv++) : GPS_ASRC;
        u4IChSz   = (u4Argc > 1) ? (*pu4Argv++) : 3840 * 2;
        u4OChSz   = (u4Argc > 2) ? (*pu4Argv++) : 7680 * 2;
        u4PhyFile = (u4Argc > 3) ? (*pu4Argv++) : 0;  

        AsrcTest_New((ASRC_CLS_TYPE)u4Type, u4IChSz, u4OChSz, (bool)u4PhyFile);
    }
}

#endif  //AUD_VFY_ASRC


//========================================================//
    #define CodeSight_AsrcTest_Grp
//========================================================//

void AsrcTest_BaseGrp(u32 u4Type, u32 u4Param1, u32 u4Param2)
{
    switch(u4Type)
    {
    case ASRCTEST_GET_VER:
        AUDLOG_NO_PREFIX(ALOG_INFO, AUD_COMM_VERSION);
        break;

    case ASRCTEST_GET_LOG:
        ASRCLOG_CLI(T("Get Log: 0x%x \n"), (u32)_u4AsrcLog);
        break;

    case ASRCTEST_SET_LOG:
        ASRCLOG_CLI(T("Set Log: 0x%x -> 0x%x \n"), (u32)_u4AsrcLog, (u32)u4Param1);
        _u4AsrcLog = u4Param1;
        break;

    case ASRCTEST_DUMP_REGS:
    {
        PASRC_MGR_CLS_PUB prMgr = AsrcMgr_Get((ASRC_CLS_TYPE)u4Param1);
        prMgr->LogAllRegs(prMgr);
    }    
        break;

    case ASRCTEST_MGR_INFO:
    {
        PASRC_MGR_CLS_PUB prMgr = AsrcMgr_Get((ASRC_CLS_TYPE)u4Param1);
        prMgr->LogAttribute(prMgr);
    }    
        break;

    case ASRCTEST_CHS_INFO:
    {
        PASRC_MGR_CLS_PUB prMgr = AsrcMgr_Get((ASRC_CLS_TYPE)u4Param1);
        prMgr->LogChsAttribute(prMgr, u4Param2);
    }    
        break;
        
    default:

        break;
    }
}


void AsrcTest_UnitTestGrp(u32 u4Type, u32 u4Param1, u32 u4Param2)
{
#if (AUD_UNIT_TEST_SUPPORT)       
    switch(u4Type)
    {
    case ASRCTEST_UNITTEST_NEW:
        AsrcTest_NewCmd((u32 *)u4Param1, u4Param2);     
        break;

    case ASRCTEST_UNITTEST_DELETE:
        if (u4Param1 < ASRC_TYPE_NUM) {
            AsrcTest_Delete(_aprAsrcTest[u4Param1]);
        }
        break;

    case ASRCTEST_UNITTEST_CHS_SETUP:
        AsrcTest_ChsSetupCmd((u32 *)u4Param1, u4Param2);
        break;

    case ASRCTEST_UNITTEST_CHS_START:
        if (u4Param1 < ASRC_TYPE_NUM) {
            AsrcTest_ChsStart(_aprAsrcTest[u4Param1], u4Param2);
        }
        break;

    case ASRCTEST_UNITTEST_CHS_STOP:
        if (u4Param1 < ASRC_TYPE_NUM) {
            AsrcTest_ChsStop(_aprAsrcTest[u4Param1], u4Param2);
        }
        break;
        
    default:

        break;
    }
#endif
}


void AsrcTest_OtherGrp(u32 u4Type, u32 u4Param1, u32 u4Param2)
{
    switch(u4Type)
    {      
    default:

        break;
    }
}


void AsrcTest_Cmd(u32 u4Type, u32 u4Param1, u32 u4Param2)
{

    ASRCLOG_CLI(T("[TestCmd] Type(0x%x) Params(0x%x, 0x%x)\n"), (u32)u4Type, (u32)u4Param1, (u32)u4Param2);
    switch(u4Type & 0xF0)
    {
    case ASRCTEST_BASE_GRP:
        AsrcTest_BaseGrp(u4Type, u4Param1, u4Param2);
        break;
        
    case ASRCTEST_UNITTEST_GRP:
        AsrcTest_UnitTestGrp(u4Type, u4Param1, u4Param2);
        break;

    case ASRCTEST_OTHER_GRP:
        AsrcTest_OtherGrp(u4Type, u4Param1, u4Param2);
        break;
        
    default:
        break;
    }
   
}


