#include <linux/mm.h>

#include "x_os.h"
#include "windows.h"

#include "imgresz_hal_if.h"
#include "imgresz_log.h"
#ifdef __linux__
#include <linux/dma-mapping.h>
#endif

/*These following API will be used in Imgresz_cmd.c,
but now we don't support imgresz cmd line, so this api is unnecessary.*/
void *x_alloc_aligned_ch2_mem(u32 u4Size, u32 u4Align);
void *x_alloc_aligned_ch1_mem(u32 u4Size, u32 u4Align);

/*static void *_pvPa;*/

static VOID RemovePaVaFromMapTable(unsigned int u4Pa);
static u32 PA_TO_VA(u32 u4Pa);



typedef struct {
	u32    u4Pa;
	u32    u4Va;
	u32    u4Size;
} PA_VA_MAP_T;

#define MAP_TABLE_SIZE 2000

PA_VA_MAP_T _aMapTable[MAP_TABLE_SIZE];

static u32 _u4MapCnt;

VOID AddPaVatoMapTable(u32 u4Pa, u32 u4Va, u32 u4Size)
{
	/*RETAILMSG(1, (L"AddPaVatoMapTable BEGIN \r\n"));*/

	if (_u4MapCnt < MAP_TABLE_SIZE) {
		/*RETAILMSG(1, (L"AddPaVatoMapTable ...%x \r\n", u4Pa));*/
		_aMapTable[_u4MapCnt].u4Pa = u4Pa;
		_aMapTable[_u4MapCnt].u4Va  = u4Va;
		_aMapTable[_u4MapCnt].u4Size = u4Size;
		_u4MapCnt++;
	}
}
/**/

VOID RemovePaVaFromMapTable(unsigned int u4Pa)
{
	u32 i;

	i = 0;

	for (i = 0; i < _u4MapCnt; i++) {
		if (_aMapTable[i].u4Pa == u4Pa) {
			if (i < (_u4MapCnt - 1)) {
				memmove(_aMapTable + i, _aMapTable + i + 1,
					(_u4MapCnt - 1 - i) * sizeof(_aMapTable[0]));
			}

			_u4MapCnt--;
			/*RETAILMSG(1, (L"unregister fb pa %x %d \r\n", u4Pa, i));*/
			return;
		}

		/*RETAILMSG(1, (L"POLLING fb pa %x %d  %x\r\n", _aMapTable[i].u4Pa, i, u4Pa));*/
	}

	if (i >= _u4MapCnt) {
		RETAILMSG(1, (("unregister failed pa %x  \r\n "), u4Pa));
	}


}




u32 PA_TO_VA(u32 u4Pa)
{
	u32 j;

	for (j = 0; j < _u4MapCnt; j++) {
		if ((u4Pa >= _aMapTable[j].u4Pa)  &&
		    (u4Pa < _aMapTable[j].u4Pa + _aMapTable[j].u4Size)) {
			break;
		}
	}

	if (j >= _u4MapCnt) {
		return (u32)(-1);
	} else {
		return (_aMapTable[j].u4Va + u4Pa - _aMapTable[j].u4Pa);
	}
}

u32 VA_TO_PA(u32 u4Va)
{
	u32 j;

	for (j = 0;  j < _u4MapCnt; j++) {
		if ((u4Va >= _aMapTable[j].u4Va) &&
		    (u4Va < _aMapTable[j].u4Va + _aMapTable[j].u4Size)) {
			break;
		}
	}

	if (j >= _u4MapCnt) {
		return (u32)(-1);
	} else {
		return (_aMapTable[j].u4Pa +  u4Va - _aMapTable[j].u4Va);
	}
}

void *x_alloc_aligned_nc_mem(u32 u4Size, u32 u4Align)
{
#if 1
	u32 pa;
	VOID *u4Va;

#ifdef __linux__
	u4Va = dma_alloc_writecombine(NULL, PAGE_ALIGN(u4Size), (dma_addr_t *)&pa, GFP_KERNEL);
#else
	u4Va = AllocPhysMem(u4Size, PAGE_READWRITE, 0, 0, &pa);
#endif

	AddPaVatoMapTable(pa, (u32) u4Va, PAGE_ALIGN(u4Size));

	if (NULL == u4Va) {
		RETAILMSG(1, (TEXT("AllocPhyMem failed.\r\n")));
		return (void *)u4Va;
	}

	return ((void *)pa);

#else

	return ((void *)0xE000000);

#endif
}



void  x_free_aligned_nc_mem(void *pUser)
{
	VOID *pvVa;
	u32 size = 0;
	u32 i = 0;


	pvVa = (VOID *) PA_TO_VA((u32)pUser);

#ifdef __linux__

	for (i = 0; i < _u4MapCnt; i++) {
		if (_aMapTable[i].u4Pa == (u32)pUser) {
			size = _aMapTable[i].u4Size;
			break;
		}
	}

	if (i == _u4MapCnt) {
		IMGR_LOG(IMGR_LOG_LVL_DBG, "[IMGREZ] %s line %d \r\n", __func__, __LINE__);
		return;
	}

#endif
	RemovePaVaFromMapTable((unsigned int)pUser);

	if (pvVa) {
#ifdef __linux__
		dma_free_writecombine(NULL, size, pvVa, (dma_addr_t)pUser);
#else
		FreePhysMem(pvVa);
#endif
	}
}

#if 0
void HalFlushInvalidateDCache(void)
{
#ifdef __linux__
	/*to do*/
#else
	CacheSync(CACHE_SYNC_DISCARD);
#endif
}
#endif

