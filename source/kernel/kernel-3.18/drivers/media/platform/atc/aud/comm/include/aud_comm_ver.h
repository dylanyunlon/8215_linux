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
*[File]               aud_comm_ver.h
*[Version]            v1.0
*[Revision Date]      2014-03-10
*[Author]             tongfa.luo@autochips.com 
*[Description]
*        
*
******************************************************************************/

#ifndef _AUD_COMM_VER_H_
#define _AUD_COMM_VER_H_

#include "aud_comm_macros.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#define AUD_COMM_MOD_NAME               T("AUD_COMM")
#define AUD_COMM_VER_MAIN               1
#define AUD_COMM_VER_MINOR              0
#define AUD_COMM_VER_REV                7

#define AUD_COMM_VER_BRANCH             T("MT3363.Main")       //branch
#define AUD_COMM_VER_DATE               T("2014.04.14")        //yyyy.mm.dd
   
#define AUD_COMM_VERSION                \
    T("[%s][%s] %s(%d_%d_%d)\r\n"), AUD_COMM_VER_BRANCH, AUD_COMM_MOD_NAME, \
    AUD_COMM_VER_DATE, AUD_COMM_VER_MAIN, AUD_COMM_VER_MINOR, AUD_COMM_VER_REV



/***************************************************************************************

                          |aud_comm_macros.h    :  general macros
                          |aud_comm_ver.h       :  define aud comm module version                      
                          |aud_comm_datatype.h  :  define datatype  (include enum, struct)                         
                          |aud_comm_reg_rw.h    :  define base reg rw control
                          |
                          |aud_comm_log.h       :  for debug log control
                          |aud_comm_tbl.h       :  table data
                          |aud_comm_os.h        :  the behavior depend on os                        
         |--- \include ---|aud_comm_misc.h      :  misc function
         |                |
         |                |aud_comm_obj.h       :  C Object-oriented programming demo (not to build)
         |                |aud_comm_file.h      :  file(phy/vir) read / write control 
         |                |
         |                |aud_comm_test.h      :  unit test for this module
         |                |
         |                |aud_if_comm.h        :  for other modul to include           
\comm ---|
         |
         |--- aud_comm_log.c    : 
         |--- aud_comm_tbl.c    :
         |--- aud_comm_os.c     :         
         |--- aud_comm_misc.c   :  
         |
         |--- aud_comm_obj.c    :
         |--- aud_comm_file.c   :
         |
         |--- aud_comm_test.c   :  

**************************************************************************************/


#ifdef __cplusplus
}
#endif // __cplusplus

#endif  //_AUD_COMM_VER_H_

