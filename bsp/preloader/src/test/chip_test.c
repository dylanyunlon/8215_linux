#include "targetConfig.h"
#include "preloader_common.h"

//#if (CHIP_TEST_ENABLE)

#include "chip_test.h"

extern TEST_ITEM_t __test_list_begin;
extern TEST_ITEM_t __test_list_end;
extern UINT32 __dramk_start;
extern UINT32 __resident_start;
extern UINT32 _dramk_loader_start;
extern UINT32 _dramk_loader_end;


UINT32 testBuf1[MAX_BUF_SIZE];
UINT32 testBuf2[MAX_BUF_SIZE];
UINT32 testBuf3[MAX_BUF_SIZE];
UINT32 testBuf4[MAX_BUF_SIZE];
UINT32 testBuf5[MAX_BUF_SIZE];
UINT32 testBuf6[MAX_BUF_SIZE];

UINT32 Stop_Test;
void Chip_Test(UINT32 cpu)
{
  TEST_ITEM_t *pTestItem,*pTestItemEnd;
  Stop_Test = 0;
  Printf("Chip Test Start at CPU%d\n",cpu);

  pTestItem = &__test_list_begin;
  pTestItemEnd = &__test_list_end;

  while(pTestItem < pTestItemEnd)
  {
    Printf("-----------Start ");
	Printf(pTestItem->name);
	Printf(" test------------\n");
  	pTestItem->test_fn(cpu);
	pTestItem++;
  }
  
  

}

void Dramk_Move(UINT32 cpu)
{
    UINT32 Datalength = 0;
    UINT32 i=0;

    UINT8 *pSrcLoaderStart = (UINT8 *)(&_dramk_loader_start);
    UINT8 *pResidentStart = (UINT8 *)(&__resident_start);
    UINT8 *pSrcStart = (UINT8 *)(pSrcLoaderStart - pResidentStart);
    UINT8 *pSrcLoaderEnd = (UINT8 *)(&_dramk_loader_end);
    UINT8 *pDestStart = (UINT8 *)(&__dramk_start);
    UINT32 *pSrcStart32 = (UINT32 *)pSrcStart;
    UINT32 *pDestStart32 = (UINT32 *)pDestStart;
    Datalength =pSrcLoaderEnd-pSrcLoaderStart;

    Printf("pSrcLoaderStart = 0x%08x\n",pSrcLoaderStart);
    Printf("pResidentStart  = 0x%08x\n",pResidentStart);
    Printf("pSrcStart       = 0x%08x\n",pSrcStart);
    Printf("pSrcLoaderEnd   = 0x%08x\n",pSrcLoaderEnd);
    Printf("pDestStart      = 0x%08x\n",pDestStart);
    Printf("Datalength      = 0x%08x\n",Datalength);
    
    Datalength=(Datalength+3)/4;
    
    for(i=0;i<Datalength;i++)
    {
         *(pDestStart32++)= *(pSrcStart32++);

    }


}


//#endif

