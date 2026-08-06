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
 * @file aud_io_clock.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_IO_CLOCK_H
#define _AUD_IO_CLOCK_H

#include "aud_reg_ckgen.h"
#include "aud_reg_top_misc.h"
#include "aud_reg_rgbk2.h"
#include "aud_if_comm.h"
#include "aud_if_hw.h"
#include "aud_define.h"
#include "aud_io_clock_if.h"


#ifdef __cplusplus
    extern "C"
    {
#endif



/**********************************************************************************
*
*   macros
*
**********************************************************************************/
    #define APLL1_VALUE     270950400
    #define APLL2_VALUE     294912000
    

/**********************************************************************************
*
*   data type
*
**********************************************************************************/



/**********************************************************************************
*
*   For Debug Log
*
**********************************************************************************/


/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_IO_CLOCK_H
