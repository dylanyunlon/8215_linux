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
*[File]                     aud_comm_file.h
*[Version]                  v1.0
*[Revision Date]            2014-03-10
*[Author]                   tongfa.luo@autochips.com 
*[Description]
*        
*
******************************************************************************/
#ifndef _AUD_COMM_FILE_H_
#define _AUD_COMM_FILE_H_

#include "aud_comm_misc.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


//=========================================================//

typedef enum 
{
    PHY_FILE = 0,
    VIR_FILE,
}FILE_CLS_TYPE;


typedef struct
{
    s8 szFileName[AUD_FILE_MAX_PATH];
    
    u32 u4Chn;
    u32 u4ChBufSz;        
    u32 u4BW;
}FILE_PHY_PARAMS_T, *PFILE_PHY_PARAMS_T;


typedef struct
{
    u32 u4TblSAdr;        
    u32 u4ReadLoops;     
    
    u32 u4Chn;
    u32 u4ChBufSz;        
    u32 u4BW;    
    
}FILE_VIR_PARAMS_T, *PFILE_VIR_PARAMS_T;


typedef struct
{
    bool (*Open)(void * pThis, bool fgRead, void * pvParams); 
    bool (*Close)(void * pThis);  
    bool (*Read)(void * pThis, PAUD_DATA_BUF_T prDst); 
    bool (*Write)(void * pThis, PAUD_DATA_BUF_T prSrc);  

    u32 (*Delete)(void * pThis);
  
}FILE_CLS_PUB, *PFILE_CLS_PUB;


//=========================================================//

PFILE_CLS_PUB File_New(FILE_CLS_TYPE eType);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif  //_AUD_COMM_FILE_H_

