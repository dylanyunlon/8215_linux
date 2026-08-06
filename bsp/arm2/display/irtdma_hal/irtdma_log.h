#ifndef __IRT_DMA_LOG_H__
#define __IRT_DMA_LOG_H__

enum
{
    IRD_LOG_LVL_OFF = 0,
    IRD_LOG_LVL_ERR,
    IRD_LOG_LVL_WARN,
    IRD_LOG_LVL_INFO,
    IRD_LOG_LVL_HAL,    
    IRD_LOG_LVL_DBG,
    IRD_LOG_LVL_IRQ,
    IRD_LOG_LVL_REGRW,  
};

extern UINT32 _u4IRD_DBG_LVL;  
extern UCHAR* _pcIrtDmaLogLevel[];

#define IRTDMA_LOG_TAG "[IRTDMA] "

#define IRD_LOG(lvl, ...)\
{ \
    if (lvl <= _u4IRD_DBG_LVL) {\
    printk("[IRTDMA]: ");\
    printk(__VA_ARGS__);\
    }\
}

#endif

