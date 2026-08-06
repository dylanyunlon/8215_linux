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

/**
 * @file aud_io_manager_if.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_IO_MANAGER_IF_H
#define _AUD_IO_MANAGER_IF_H

#include "aud_if_comm.h"
#include "aud_if_hw.h"
#include "aud_define.h"


     
#ifdef __cplusplus
     extern "C"
    {
#endif


/**********************************************************************************
*
*   macros
*
**********************************************************************************/
//#define AUD_IO_POWER_CONTROL



/**********************************************************************************
*
*   data type
*
**********************************************************************************/




/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/

extern void AudIoMg_InitHw(bool fgPowerOnByArm9);





#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_IO_MANAGER_IF_H

