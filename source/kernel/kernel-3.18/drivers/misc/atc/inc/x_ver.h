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

#ifndef _X_VERSION_H_
#define _X_VERSION_H_

#ifdef __KERNEL__
#define VER_PRINT printk
//#include <generated/atc_project.h>
#else
#define VER_PRINT printf
#endif

#include "x_cl.h"
#ifdef  __cplusplus
extern "C" {
#endif


#define AUTO_BUILD ("")
#define BRANCH_NAME      ("android-trunk-m0.atc")
//#define BRANCH_NAME      ("linux-trunk-dizzy")
#define P4_CHANGELIST 20170811

#define MOD_VERSION_INFO(mod, major, minor,rev) \
    VER_PRINT("[VER][%s] V%02d.%02d_%02d %06d [%s]\r\n", mod,major, minor, rev, P4_CHANGELIST, BRANCH_NAME)

#define MOD_VERSION_INFO_CL(mod, major, minor,rev, cl) \
    VER_PRINT("[VER][%s] V%02d.%02d_%02d %06d [%s]\r\n", mod,major, minor, rev, cl, BRANCH_NAME)

#define MOD_VER_MSG(mod, major, minor,rev) \
    VER_PRINT("[VER][%s] V%02d.%02d_%02d %06d [%s]\r\n", mod,major, minor, rev, P4_CHANGELIST, BRANCH_NAME)

#define MOD_VER_MSG_CL(mod, major, minor,rev, cl) \
    VER_PRINT("[VER][%s] V%02d.%02d %02d_%06d [%s]\r\n", mod,major, minor, rev, cl, BRANCH_NAME)

#define DEP_VER_MSG_CL(mod, major, minor,rev, cl) \
    VER_PRINT("[DEP][%s] V%02d.%02d_%02d %06d \r\n", mod,major, minor, rev, cl)

#define DEP_VER_MSG(mod, major, minor,rev) \
    VER_PRINT("[DEP][%s] V%02d.%02d_%02d %06d \r\n", mod,major, minor, rev, P4_CHANGELIST)

#define DEP_VER_MSG_RANGE(mod, major_s, minor_s,rev_s,cl_s, major_e, minor_e,rev_e,cl_e) \
    VER_PRINT("[DEP][%s] V%02d.%02d_%02d_%06d -- V%02d.%02d_%02d_%06d\r\n", mod, major_s, minor_s, rev_s, cl_s, major_e, minor_e, rev_e, cl_e)

#ifdef  __cplusplus
}
#endif

#endif  // _X_VERSION_H_
