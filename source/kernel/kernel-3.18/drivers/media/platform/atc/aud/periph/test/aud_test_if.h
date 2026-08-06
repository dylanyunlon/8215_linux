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
 * @file aud_test_if.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_TEST_IF_H
#define _AUD_TEST_IF_H


#ifdef __cplusplus
    extern "C"
    {
#endif

extern void AudMicTest(u32 arg1, u32 arg2, u32 arg3);
extern void AudPcmTest(u32 arg1, u32 arg2, u32 arg3);
extern void AudLinTest(u32 arg1, u32 arg2, u32 arg3);
extern void AudBypsTest(u32 arg1, u32 arg2, u32 arg3);
extern void AudAoutTest(u32 arg1, u32 arg2, u32 arg3);
extern void AudMlinTest(u32 arg1, u32 arg2, u32 arg3);

extern void AudIOTest(u32 u4TestType, u32 u4Arg1, u32 u4Arg2);


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_TEST_IF_H