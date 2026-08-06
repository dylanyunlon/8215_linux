#ifndef _X_TYPEDEF_H

#define _X_TYPEDEF_H

//#include <windows.h>
#include "types.h"
//#include <memory.h>




typedef UINT32  HANDLE_T;
#define NULL_HANDLE ((HANDLE_T)(NULL))

#ifdef __cplusplus
#define EXTERN        extern "C"
#else
#define EXTERN   extern 
#endif



#ifdef __cplusplus

  #define externC        extern "C"
#else

  #define externC        
#endif

#define VA_LIST  va_list

#define INLINE


#ifndef UNUSED
#define UNUSED(x)               (void)x
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif




#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif





#endif
