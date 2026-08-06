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
*[File]                     aud_comm_test.h
*[Version]                  v1.0
*[Revision Date]            2014-03-10
*[Author]                   tongfa.luo@autochips.com 
*[Description]
*        
*
******************************************************************************/
#ifndef _AUD_COMM_TEST_H_
#define _AUD_COMM_TEST_H_

#include "aud_if_comm.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#define COMMTEST_BASE_GRP                   0x00
#define COMMTEST_GET_VER                    (COMMTEST_BASE_GRP + 0x01)
#define COMMTEST_GET_LOG                    (COMMTEST_BASE_GRP + 0x02)
#define COMMTEST_SET_LOG                    (COMMTEST_BASE_GRP + 0x03)
#define COMMTEST_READ_BUF                   (COMMTEST_BASE_GRP + 0x04)
#define COMMTEST_DRAW_BUF                   (COMMTEST_BASE_GRP + 0x05)
#define COMMTEST_READ_REG                   (COMMTEST_BASE_GRP + 0x06)
#define COMMTEST_WRITE_REG                  (COMMTEST_BASE_GRP + 0x07)

#define COMMTEST_REGKEY_GRP                 0x10
#define COMMTEST_REGKEY_AUD_GET_u32       (COMMTEST_REGKEY_GRP + 0x01)
#define COMMTEST_REGKEY_AUD_SET_u32       (COMMTEST_REGKEY_GRP + 0x02)
#define COMMTEST_REGKEY_AUD_GET_STRING      (COMMTEST_REGKEY_GRP + 0x03)
#define COMMTEST_REGKEY_AUD_SET_STRING      (COMMTEST_REGKEY_GRP + 0x04)
#define COMMTEST_REGKEY_AUD_GET_BINARY      (COMMTEST_REGKEY_GRP + 0x05)
#define COMMTEST_REGKEY_AUD_SET_BINARY      (COMMTEST_REGKEY_GRP + 0x06)

#define COMMTEST_REGKEY_WAV_GET_u32       (COMMTEST_REGKEY_GRP + 0x09)
#define COMMTEST_REGKEY_WAV_SET_u32       (COMMTEST_REGKEY_GRP + 0x0A)
#define COMMTEST_REGKEY_WAV_GET_STRING      (COMMTEST_REGKEY_GRP + 0x0B)
#define COMMTEST_REGKEY_WAV_SET_STRING      (COMMTEST_REGKEY_GRP + 0x0C)
#define COMMTEST_REGKEY_WAV_GET_BINARY      (COMMTEST_REGKEY_GRP + 0x0D)
#define COMMTEST_REGKEY_WAV_SET_BINARY      (COMMTEST_REGKEY_GRP + 0x0E)


#define COMMTEST_OTHER_GRP                  0xF0
#define COMMTEST_FILE_TEST                  (COMMTEST_OTHER_GRP + 0x01)
#define COMMTEST_SHOW_TBL_SADDR             (COMMTEST_OTHER_GRP + 0x02)


#ifdef __cplusplus
}
#endif // __cplusplus

#endif  //_AUD_COMM_LOG_H_

