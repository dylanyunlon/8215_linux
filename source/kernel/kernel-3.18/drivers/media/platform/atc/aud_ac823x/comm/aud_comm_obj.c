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
*[File]                 aud_comm_obj.c
*[Author]               tongfa.luo@autochips.com
*[Description]
*       
*[Copyright]
*       
******************************************************************************/

#include "aud_oal.h"
#include "aud_comm_obj.h"

typedef struct 
{
    AUD_OBJ_CLS_PUB rPub;
    
    u32 u4Value;  
}AUD_OBJ_CLS, *PAUD_OBJ_CLS;


//=======================================//
    #define CodeSight_AudObj
//=======================================//

static void Obj_SetValue(void * pThis, u32 u4Value)
{
    PAUD_OBJ_CLS prThis = (PAUD_OBJ_CLS)pThis;
    prThis->u4Value = u4Value;  
    COMMLOG_DBG((T("[OBJ(0x%x)]Set value: 0x%x \r\n"), (u32)prThis, (u32)prThis->u4Value));
}


static u32 Obj_GetValue(void * pThis)
{
    PAUD_OBJ_CLS prThis = (PAUD_OBJ_CLS)pThis;
    return (prThis->u4Value);
}

#if 0
static u32 Obj_Delete(void * pThis)
{
    PAUD_OBJ_CLS prThis = (PAUD_OBJ_CLS)pThis;
    
    COMMLOG_DBG((T("Delete OBJ[0x%x] \n"), (unsigned s32)prThis));
    AUD_CLASS_DELETE();

    return (AUD_RET_OK);
}
#endif


//=======================================//
    #define CodeSight_AudObj_Create
//=======================================//

PAUD_OBJ_CLS_PUB AudObj_New(void)
{
    PAUD_OBJ_CLS prThis = AUD_CLASS_NEW(AUD_OBJ_CLS);
    
    if (prThis) 
    {   
        COMMLOG_DBG((T("New OBJ[0x%x]! \n"), (u32)prThis)); 
        
        prThis->u4Value = 0;
        
        prThis->rPub.SetValue = Obj_SetValue;
        prThis->rPub.GetValue = Obj_GetValue;
    }
    
    return ((PAUD_OBJ_CLS_PUB)prThis);
}


//=========================================//
    #define CodeSight_AudObj_UnitTest
//=========================================//

static void AudObj_UnitTest(u32 u4Value1, u32 u4Value2, u32 u4Value3)
{
    PAUD_OBJ_CLS_PUB prObj1 = AudObj_New();
    PAUD_OBJ_CLS_PUB prObj2 = AudObj_New();
    PAUD_OBJ_CLS_PUB prObj3 = AudObj_New();

    prObj1->SetValue(prObj1, u4Value1);
    prObj2->SetValue(prObj2, u4Value2);
    prObj3->SetValue(prObj3, u4Value3);

    prObj1->Delete(prObj1);
    prObj2->Delete(prObj2);
    prObj3->Delete(prObj3);
}


void AudObj_UnitTest_Cmd(u32 au4Params[], u32 u4ParamNum)
{
    u32 u4Value1 = 0, u4Value2 = 0, u4Value3 = 0;

    if (u4ParamNum > 0) {
        u4Value1 = au4Params[0];
    }
    if (u4ParamNum > 1) {
        u4Value2 = au4Params[1];
    }
    if (u4ParamNum > 2) {
        u4Value3 = au4Params[2];
    }

    COMMLOG_TEST((T("[Obj] Unit Test Start ======================> \n")));
    COMMLOG_TEST((T("[Obj] Value1(%d) Value2(%d) Value3(%d), CmdParamNum(%d)\n"), 
        (s32)u4Value1, (s32)u4Value2, (s32)u4Value3, (s32)u4ParamNum));

    AudObj_UnitTest(u4Value1, u4Value2, u4Value3);
    
    COMMLOG_TEST((T("[Obj] Unit Test Finish  <===================== \n")));
}

