#ifndef __ARM2__
#include <linux/mm.h>
#else
#include "x_types.h"
#endif

#include "x_os.h"
#include "osd_map.h"

typedef struct {
	__u32    u4Pa;
	__u32    u4Va;
	__u32    u4Size;
} PA_VA_MAP_T;

#define MAP_TABLE_SIZE 2000

PA_VA_MAP_T _aMapTableDal[MAP_TABLE_SIZE];

static __u32 _u4MapCnt;


void AddPaVatoMapTableDal(__u32 u4Pa, __u32 u4Va, __u32 u4Size)
{
	if (_u4MapCnt < MAP_TABLE_SIZE) {
		_aMapTableDal[_u4MapCnt].u4Pa = u4Pa;
		_aMapTableDal[_u4MapCnt].u4Va  = u4Va;
		_aMapTableDal[_u4MapCnt].u4Size = u4Size;
		_u4MapCnt++;
	}
}



__u32 PA_TO_VA_DAL(__u32 u4Pa)
{
	__u32 j, ret = (__u32)(-1);

	for (j = 0; j < _u4MapCnt; j++) {
		if ((u4Pa >= _aMapTableDal[j].u4Pa)  &&
		    (u4Pa < _aMapTableDal[j].u4Pa + _aMapTableDal[j].u4Size)) {
			break;
		}
	}

	if (j >= _u4MapCnt) {
		return ret;
	}

	ret = _aMapTableDal[j].u4Va + u4Pa - _aMapTableDal[j].u4Pa;

	return ret;
}

__u32 VA_TO_PA_DAL(__u32 u4Va)
{
	__u32 j, ret = (__u32)(-1);

	for (j = 0;  j < _u4MapCnt; j++) {
		if ((u4Va >= _aMapTableDal[j].u4Va) &&
		    (u4Va < _aMapTableDal[j].u4Va + _aMapTableDal[j].u4Size)) {
			break;
		}
	}

	if (j >= _u4MapCnt) {
		return ret;
	}
	ret = _aMapTableDal[j].u4Pa +  u4Va - _aMapTableDal[j].u4Va;
	return ret;
}


