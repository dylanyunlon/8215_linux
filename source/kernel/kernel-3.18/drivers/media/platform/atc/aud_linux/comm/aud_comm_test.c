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




#include "aud_comm_test.h"


//========================================================//
    #define CodeSight_AudCommTest_Regkey
//========================================================//

static void CommTest_Regkey_GetDword(DRV_TYPE_E eDrv, u32 u4Name)
{
    s8 *szName = (s8 *)u4Name;
    u32 u4Value;  
    
    AudOS_Regkey_GetDword(eDrv, szName, &u4Value, 0);
    COMMLOG_CLI(T("[Regkey(%d)]GetDword  %s : %d \n"), eDrv, szName, (s32)u4Value);
}


static void CommTest_Regkey_SetDword(DRV_TYPE_E eDrv, u32 u4Name, u32 u4Value)
{
    s8 *szName = (s8 *)u4Name;
    
    AudOS_Regkey_SetDword(eDrv, szName, u4Value);
    COMMLOG_CLI(T("[Regkey(%d)]SetDword  %s : %d \n"), eDrv, szName, (s32)u4Value);
}


static void CommTest_Regkey_GetString(DRV_TYPE_E eDrv, u32 u4Name)
{
    s8 *szName = (s8 *)u4Name;
    s8 szValue[200];
    
    AudOS_Regkey_GetString(eDrv, szName, (s8 *)szValue, (s8 *)"Test");
    COMMLOG_CLI(T("[Regkey]GetString  %s : %s \n"), szName, szValue);
}


static void CommTest_Regkey_SetString(DRV_TYPE_E eDrv, u32 u4Name, u32 u4Value)
{
    s8 *szName = (s8 *)u4Name;
    s8 *szValue = (s8 *)u4Value;
    
    AudOS_Regkey_SetString(eDrv, szName, szValue);
    COMMLOG_CLI(T("[Regkey]SetString  %s : %s \n"), szName, szValue);
}


static void CommTest_Regkey_GetBinary(DRV_TYPE_E eDrv, u32 u4Name, u32 u4Size)
{
    s8 *szName = (s8 *)u4Name;
    u8 abValue[200];
    u32 i;
    
    AudOS_Regkey_GetBinary(eDrv, szName, abValue, u4Size);
    COMMLOG_CLI(T("[Regkey]GetBinary  %s  size(%d) "), szName, (s32)u4Size);
    for (i = 0; i < u4Size; i++)
    {
        if (i % 16 == 0) {
            AUDLOG_NO_PREFIX(ALOG_INFO, T("\n"));
        }
        AUDLOG_NO_PREFIX(ALOG_INFO, T("0x%2X "), abValue[i]);  
        
    }
    AUDLOG_NO_PREFIX(ALOG_INFO, T("\n"));
}


static void CommTest_Regkey_SetBinary(DRV_TYPE_E eDrv, u32 *pu4Argv, u32 u4Argc)
{
    if (u4Argc >= 3)
    {
        s8 *szName = (s8 *)pu4Argv[0];
        u8 *pbValue = (u8 *)pu4Argv[1];
        u32 u4Size = pu4Argv[2];
        
        AudOS_Regkey_SetBinary(eDrv, szName, pbValue, u4Size);
        COMMLOG_CLI(T("[Regkey(%d)]SetString  %s  size(%d) \n"), eDrv, szName, (s32)u4Size);
    }
}


//=========================================//
    #define CodeSight_AudCommTest_File
//=========================================//

static void CommTest_File_Test(PFILE_CLS_PUB prRead, PFILE_CLS_PUB prWrite,
                    void * pvRParams, void * pvWParams, u32 u4BW, u32 u4Chn)
{
    AUD_DATA_BUF_T rTempBuf;
    u32 u4PhyAddr;

    if (prRead && prWrite)
    {
        prRead->Open(prRead, TRUE, pvRParams);
        prWrite->Open(prWrite, FALSE, pvWParams);

        rTempBuf.u4BW = u4BW;
        rTempBuf.u4Chn = u4Chn;
        rTempBuf.u4ChBufSz = AUD_ALIGNMENT_MASK(1500, u4BW >> 3);
        rTempBuf.u4VirSAdr = AudOS_Memory_Alloc(rTempBuf.u4ChBufSz * u4Chn, 0, &u4PhyAddr);  
    
        COMMLOG_CLI(T("[FILE] UnitTest TempBuf(0x%x, %d, %d) \n"), (s32)rTempBuf.u4VirSAdr, (s32)rTempBuf.u4ChBufSz, (s32)rTempBuf.u4Chn);
  
        while (TRUE)
        {   
            rTempBuf.u4DataSize = rTempBuf.u4ChBufSz;
            rTempBuf.u4DataOff = 0;   
            if (!prRead->Read(prRead, &rTempBuf)) {            
                break; 
            }

            rTempBuf.u4DataSize = rTempBuf.u4ChBufSz;
            rTempBuf.u4DataOff = 0;   
            if (!prWrite->Write(prWrite, &rTempBuf)) {
                break;    
            }
        }
        AudOS_Memory_Free(&rTempBuf.u4VirSAdr);
    
        prRead->Close(prRead);
        prWrite->Close(prWrite);

        prRead->Delete(prRead);
        prWrite->Delete(prWrite);
    }
}


static void CommTest_PhyFile_Test(u32 u4IBW, u32 u4OBW, u32 u4InChn, u32 u4OutChn)
{
    PFILE_CLS_PUB prRead, prWrite; 
    FILE_PHY_PARAMS_T rRParams, rWParams; 
        
    prRead = File_New(PHY_FILE);
    prWrite = File_New(PHY_FILE);
    COMMLOG_CLI(T("[File] Phy Copy Test: BW(%d, %d) Chn(%d, %d)\n"), (s32)u4IBW, (s32)u4OBW, (s32)u4InChn, (s32)u4OutChn);

    sprintf(rRParams.szFileName, "%s%dbit_%dch.bin" , AUD_FILE_PATH, (s32)u4IBW, (s32)u4InChn);
    rRParams.u4BW = u4IBW;
    rRParams.u4Chn = u4InChn;
    rRParams.u4ChBufSz = 1200;
    
    sprintf(rWParams.szFileName, "%s%d_%dbit_%dch_to_%dch.bin" , AUD_FILE_PATH, (s32)u4IBW, (s32)u4OBW, (s32)u4InChn, (s32)u4OutChn);    
    rWParams.u4BW = u4OBW;
    rWParams.u4Chn = u4OutChn;
    rWParams.u4ChBufSz = 1200;  

    CommTest_File_Test(prRead, prWrite, &rRParams, &rWParams, u4OBW, u4InChn);
}


static void CommTest_VirFile_Test(u32 u4IBW, u32 u4OBW, u32 u4InChn, u32 u4OutChn)
{
    PFILE_CLS_PUB prRead, prWrite; 
    FILE_VIR_PARAMS_T rRParams;
    FILE_VIR_PARAMS_T rWParams; 

    prRead = File_New(VIR_FILE);
    prWrite = File_New(VIR_FILE);
    COMMLOG_CLI(T("[File] Vir Copy Test: BW(%d, %d) Chn(%d, %d)\n"), (s32)u4IBW, (s32)u4OBW, (s32)u4InChn, (s32)u4OutChn);

    rRParams.u4Chn = u4InChn;
    rRParams.u4BW = u4IBW;
    if (u4IBW == 16) {
        rRParams.u4TblSAdr = (u32)AUD_TBL_SINE_16BIT;
        rRParams.u4ChBufSz = 128;
    } else {
        rRParams.u4TblSAdr = (u32)AUD_TBL_SINE_24BIT;
        rRParams.u4ChBufSz = 192;
    }    
    rRParams.u4ReadLoops = 1000;
  
    rWParams.u4BW = u4OBW;
    rWParams.u4Chn = u4OutChn;
    rWParams.u4ChBufSz = rRParams.u4ChBufSz * rRParams.u4ReadLoops;
   
    CommTest_File_Test(prRead, prWrite, &rRParams, &rWParams, u4IBW, u4InChn);
}


static void CommTest_File_TestCmd(u32 *pu4Argv, u32 u4Argc)
{
    bool fgUsePhyFile = TRUE; 
    u32 u4IBW = 16, u4OBW = 16; 
    u32 u4InChn = 1; 
    u32 u4OutChn = 1;

    if (u4Argc > 0) {
        fgUsePhyFile = (bool)pu4Argv[0];
    }
    if (u4Argc > 1) {
        u4IBW = pu4Argv[1];
    }
    if (u4Argc > 2) {
        u4OBW = pu4Argv[2];
    }
    if (u4Argc > 3) {
        u4InChn = pu4Argv[3];
    }
    if (u4Argc > 4) {
        u4OutChn = pu4Argv[4];
    }

    COMMLOG_CLI(T("[File] Unit Test Start ======================> \n"));
    COMMLOG_CLI(T("[File] UsePhyFile(%d) IBW(%d, %d) Chn(%d, %d), CmdParamNum(%d)\n"), 
        (s32)fgUsePhyFile, (s32)u4IBW, (s32)u4OBW, (s32)u4InChn, (s32)u4OutChn, (s32)u4Argc);

    if (fgUsePhyFile) {
        CommTest_PhyFile_Test(u4IBW, u4OBW, u4InChn, u4OutChn);
    } else {
        CommTest_VirFile_Test(u4IBW, u4OBW, u4InChn, u4OutChn);
    }
    
    COMMLOG_CLI(T("[File] Unit Test Finish  <===================== \n"));
}


//=========================================//
    #define CodeSight_AudCommTest_Others
//=========================================//
static void CommTest_DrawBufferData(u32 *pu4Argv, u32 u4Argc)
{
    if (u4Argc >= 3)
    {
        u32 u4SAdr = pu4Argv[0];
        u32 u4Size = pu4Argv[1];
        u32 u4BW = pu4Argv[2];
        u32 u4CutBit = u4BW - 8;
        u32 u4CutSpace = 0;

        if (u4Argc > 3) {
            u4CutBit = pu4Argv[3];
        }

        if (u4Argc > 4) {
            u4CutSpace = pu4Argv[4];
        }

        AudMisc_BufferData_Draw(u4SAdr, u4Size, u4BW, u4CutBit, u4CutSpace);
    }
}


static void CommTest_ShowTblSAddr(void)
{
    COMMLOG_CLI(T("[TBL] Show Tbl SAddr  =================> \n"));

    COMMLOG_CLI(T("[TBL] SINE_16BIT: 0x%x: BW(%d) Size(%d)\n"), 
        (u32)AUD_TBL_SINE_16BIT, 16, 128);

    COMMLOG_CLI(T("[TBL] SINE_24BIT: 0x%x: BW(%d) Size(%d)\n"), 
        (u32)AUD_TBL_SINE_24BIT, 24, 192);

    COMMLOG_CLI(T("[TBL] <===================================\n"));
}


//========================================================//
    #define CodeSight_AudCommTest_Grp
//========================================================//

static void CommTest_BaseGrp(u32 u4Type, u32 u4Param1, u32 u4Param2)
{
    switch(u4Type)
    {
    case COMMTEST_GET_VER:
        AUDLOG_NO_PREFIX(ALOG_INFO, AUD_COMM_VERSION);
        break;

    case COMMTEST_GET_LOG:
        AudLog_GetLog();
        break;
        
    case COMMTEST_SET_LOG:
        AudLog_SetLog(u4Param1);
        break;

    case COMMTEST_READ_BUF:
        AudMisc_BufferData_Read(u4Param1, u4Param2);
        break;

    case COMMTEST_DRAW_BUF:
        CommTest_DrawBufferData((u32 *)u4Param1, u4Param2);
        break;

    case COMMTEST_READ_REG:
        AUDREG_READ_LOG(u4Param1, u4Param2);
        break;

    case COMMTEST_WRITE_REG:
        AUDREG_WRITE_LOG(u4Param1, u4Param2);
        break;

    default:
        break;
    }
}


static void CommTest_RegkeyGrp(u32 u4Type, u32 u4Param1, u32 u4Param2)
{
    switch(u4Type)
    {
    case COMMTEST_REGKEY_AUD_GET_u32:
        CommTest_Regkey_GetDword(AUD_DRV, u4Param1);
        break;

    case COMMTEST_REGKEY_AUD_SET_u32:
        CommTest_Regkey_SetDword(AUD_DRV, u4Param1, u4Param2);
        break;

    case COMMTEST_REGKEY_AUD_GET_STRING:
        CommTest_Regkey_GetString(AUD_DRV, u4Param1);
        break;

    case COMMTEST_REGKEY_AUD_SET_STRING:
        CommTest_Regkey_SetString(AUD_DRV, u4Param1, u4Param2);
        break;

    case COMMTEST_REGKEY_AUD_GET_BINARY:
        CommTest_Regkey_GetBinary(AUD_DRV, u4Param1, u4Param2);
        break;

    case COMMTEST_REGKEY_AUD_SET_BINARY:
        CommTest_Regkey_SetBinary(AUD_DRV, (u32 *)u4Param1, u4Param2);
        break;

    //----------------------------------------------------------//

    case COMMTEST_REGKEY_WAV_GET_u32:
        CommTest_Regkey_GetDword(WAV_DRV, u4Param1);
        break;

    case COMMTEST_REGKEY_WAV_SET_u32:
        CommTest_Regkey_SetDword(WAV_DRV, u4Param1, u4Param2);
        break;

    case COMMTEST_REGKEY_WAV_GET_STRING:
        CommTest_Regkey_GetString(WAV_DRV, u4Param1);
        break;

    case COMMTEST_REGKEY_WAV_SET_STRING:
        CommTest_Regkey_SetString(WAV_DRV, u4Param1, u4Param2);
        break;

    case COMMTEST_REGKEY_WAV_GET_BINARY:
        CommTest_Regkey_GetBinary(WAV_DRV, u4Param1, u4Param2);
        break;

    case COMMTEST_REGKEY_WAV_SET_BINARY:
        CommTest_Regkey_SetBinary(WAV_DRV, (u32 *)u4Param1, u4Param2);
        break;

    default:
        break;
    }
}


static void CommTest_OtherGrp(u32 u4Type, u32 u4Param1, u32 u4Param2)
{
    switch(u4Type)
    {
    case COMMTEST_FILE_TEST:
        CommTest_File_TestCmd((u32 *)u4Param1, u4Param2);
        break;
        
    case COMMTEST_SHOW_TBL_SADDR:
        CommTest_ShowTblSAddr();
        break;

    default:
        break;
    }
}


void AudCommTest_Cmd(u32 u4Type, u32 u4Param1, u32 u4Param2)
{
    switch(u4Type & 0xF0)
    {
    case COMMTEST_BASE_GRP:
        CommTest_BaseGrp(u4Type, u4Param1, u4Param2);
        break;

    case COMMTEST_REGKEY_GRP:
        CommTest_RegkeyGrp(u4Type, u4Param1, u4Param2);
        break;

    case COMMTEST_OTHER_GRP:
        CommTest_OtherGrp(u4Type, u4Param1, u4Param2);
        break;
        
    default:
        break;
    }
}


