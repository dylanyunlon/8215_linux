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



#ifndef _AUD_ASRC_TEST_H_
#define _AUD_ASRC_TEST_H_

#include "aud_if_hw_asrc.h"
#include "aud_asrc_ver.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#define ASRCTEST_BASE_GRP                   0x00
    #define ASRCTEST_GET_VER                (ASRCTEST_BASE_GRP + 0x01)
    #define ASRCTEST_GET_LOG                (ASRCTEST_BASE_GRP + 0x02)
    #define ASRCTEST_SET_LOG                (ASRCTEST_BASE_GRP + 0x03)
    
    #define ASRCTEST_DUMP_REGS              (ASRCTEST_BASE_GRP + 0x04)
    #define ASRCTEST_MGR_INFO               (ASRCTEST_BASE_GRP + 0x05)
    #define ASRCTEST_CHS_INFO               (ASRCTEST_BASE_GRP + 0x06)

#define ASRCTEST_UNITTEST_GRP               0x10
    #define ASRCTEST_UNITTEST_NEW           (ASRCTEST_UNITTEST_GRP + 0x01)
    #define ASRCTEST_UNITTEST_DELETE        (ASRCTEST_UNITTEST_GRP + 0x02)
    #define ASRCTEST_UNITTEST_CHS_SETUP     (ASRCTEST_UNITTEST_GRP + 0x03)
    #define ASRCTEST_UNITTEST_CHS_START     (ASRCTEST_UNITTEST_GRP + 0x04)
    #define ASRCTEST_UNITTEST_CHS_STOP      (ASRCTEST_UNITTEST_GRP + 0x05)


#define ASRCTEST_OTHER_GRP                  0xF0



#if (AUD_UNIT_TEST_SUPPORT)

typedef struct
{   
    PASRC_CHS_CLS_PUB prChs;
    
    ASRC_CHS_FMT_T rFmt;

    PFILE_CLS_PUB prIFile;
    PFILE_CLS_PUB prOFile;
    
}ASRC_TEST_CHS_PARAM_T, *PASRC_TEST_CHS_PARAM_T;


typedef struct
{
    ASRC_CLS_TYPE eType; 
    
    PASRC_MGR_CLS_PUB prMgr;  
    
    ASRC_TEST_CHS_PARAM_T arChsParam[ASRC_CHSET_NUM];  

    bool fgUsePhyFile;
    
}ASRC_TEST_CLS, *PASRC_TEST_CLS;


#endif  //AUD_UNIT_TEST_SUPPORT

#ifdef __cplusplus
}
#endif // __cplusplus


#endif  //_AUD_ASRC_TEST_H_

