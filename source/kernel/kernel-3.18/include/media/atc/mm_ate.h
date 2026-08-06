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

#ifndef MM_ATE_H
#define MM_ATE_H

/* #define MM_ATE_CHECK */

#undef MMATE_INIT_STRUCT
#undef MMATE_INIT_POINTER
#undef MMATE_INIT_VAL
#undef MMATE_CHECK_STRUCT
#undef MMATE_CHECK_POINTER
#undef MMATE_CHECK_VAL

#define ERR_PRINT(arg) do { \
	if (!(arg)) \
		ALOGE("DBGCHK Failed:%s at line %d in %s\r\n", __func__, __LINE__, __FILE__); \
} while (0)

#ifdef MM_ATE_CHECK
#define MMATE_INIT_STRUCT(structname) do { \
		(structname).u4MMATECHKStart = 0; \
		(structname).u4MMATECHKEnd = 0; \
} while (0)

#define MMATE_INIT_POINTER(ptrtname) do { \
		(ptrtname)->u4MMATECHKStart = 0; \
		(ptrtname)->u4MMATECHKEnd = 0; \
} while (0)

#define MMATE_INIT_VAL(val)		((val) = 0)
} while (0)

#define MMATE_CHECK_STRUCT(structname) do { \
		if (0 != (structname).u4MMATECHKStart) \
				ERR_PRINT(0); \
		else if (0 != (structname).u4MMATECHKEnd)\
				ERR_PRINT(0); \
} while (0)

#define MMATE_CHECK_POINTER(ptrtname) \
do { \
		if (0 != (ptrtname)->u4MMATECHKStart) \
				ERR_PRINT(0); \
		else if (0 != (ptrtname)->u4MMATECHKEnd)\
				ERR_PRINT(0); \
} while (0)

#define MMATE_CHECK_VAL(val) \
do { \
		if (0 != (val)) \
				ERR_PRINT(0); \
} while (0)
#else
#define MMATE_INIT_STRUCT(structname)
#define MMATE_INIT_POINTER(ptrtname)
#define MMATE_INIT_VAL(val)
#define MMATE_CHECK_STRUCT(structname)
#define MMATE_CHECK_POINTER(ptrtname)
#define MMATE_CHECK_VAL(val)
#endif

#endif				/* MM_ATE_H */
