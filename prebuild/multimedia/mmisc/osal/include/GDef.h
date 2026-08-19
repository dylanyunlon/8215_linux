/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef __GDEF_H__
#define __GDEF_H__

#include "misc/atc/inc/x_typedef.h"

#define INVALID_HANDLE_VALUE  ((HANDLE)-1)

#define GVOID void 

#define GUINT32 UINT32

typedef unsigned char GUCHAR;
typedef double GDOUBLE ;
typedef signed char GBYTE  ;
typedef signed char GINT8  ;
typedef unsigned char GUINT8  ;
typedef unsigned char GBOOL ;
typedef signed short GINT16   ;
typedef unsigned short GUINT16   ;
typedef signed int  GINT32  ;
typedef unsigned long long GUINT64  ;
typedef signed long long GINT64  ;

//typedef void *HSPDECINST ;
typedef char  GCHAR;
typedef char GTCHAR;

typedef unsigned short GWORD;	
typedef unsigned long GDWORD;	

#define CP_UTF8  (1)

#endif 


