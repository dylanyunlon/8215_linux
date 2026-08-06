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

#include "x_debug.h"

#ifndef _AUD_RET_DEFINE_H_
#define _AUD_RET_DEFINE_H_

#define AUDIN_RET_OK                          ((s32)0)
#define AUDIN_RET_FAIL                        ((s32)-1)
#define AUDIN_PARAM_ERR                       ((s32)-2)
#define AUDIN_ALLOC_FAIL                      ((s32)-3)
#define AUDIN_NOT_IN_SAME_BANK                ((s32)-4)
#define AUDIN_NO_BCK_FIT                      ((s32)-5)



#define AUDIN_FALSE    (0)
#define AUDIN_TRUE     (1)


 #define AUDIN_CHECK_RESULT(ret, funcName) if((ret) < 0) \
 { \
    LOG(LOG_CTRLF,TEXT("[AUD]call %s, error: %s(%d) in %s, ret=%d\n"), \
        funcName, __FILE__, __LINE__, __FUNCTION__, ret);\
    return ret; \
 }


#endif
