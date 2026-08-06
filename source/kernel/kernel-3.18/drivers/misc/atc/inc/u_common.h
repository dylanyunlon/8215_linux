#ifndef _U_COMMON_H

#define _U_COMMON_H
#include "x_os.h"

#ifdef KERNEL_STANDARD_API
	#include <linux/types.h>
	typedef u64  STC_T;
#else
	typedef UINT64  STC_T;
#endif

#endif