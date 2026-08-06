#ifndef __VDEC_FBM_H__
#define __VDEC_FBM_H__

//#include <linux/spinlock.h>

typedef struct
{
    UINT64  u8Pts;
    UINT32  u4Flag;
    UINT32  u4PicCdTp;
    UINT32  u4TemporalRef;

    UINT32  u4VYStartAddr;
    UINT32  u4VCStartAddr;
    UINT32  u4YStartAddr;
    UINT32  u4CStartAddr;
    UINT32  u4VAlphaAddr;
    UINT32  u4AlphaAddr;

    BOOL    fgDispUse;
    BOOL    fgDBPUse;
    BOOL    fgOSEMem;

    UINT32 u4Duration;
    UINT32 u4PicWidth;
    UINT32 u4PicHeight;
    UINT32 u4AlignWidth;
    UINT32 u4AlignHeight;
    INT32  i4Rate;
} VDec_FBUF_INFO_T;

typedef struct _FB_INPUT_T
{
    VDec_FBUF_INFO_T *prVDecFBuf;
    UINT32 u4FBNum;
} FB_INPUT_T;

//extern spinlock_t fbm_lock;

BOOL FBM_IsInit(void);
BOOL FBM_Init(FB_INPUT_T *prFBInput);
VOID FBM_Deinit(VOID);
VOID FBM_Clear(BOOL fgClearDisp);

UINT32 FBM_QueryFreeFBuf(VOID);
BOOL FBM_GetFreeFBuf(UINT32 *pu4FBId);
BOOL FBM_QueryFreeFBufByDPB(VOID);
BOOL FBM_GetFreeFBufByDPB(UINT32 * pu4FBId);
BOOL FBM_FreeFBuf(UINT32 u4FBId);
VOID FBM_ShowBufStatus(VOID);

VOID FBM_SetDispUse(UINT32 u4FBId);
BOOL FBM_ClearDispUse(UINT32 u4PYAddr, UINT32 u4PCAddr);

BOOL FBM_AddToDispQueue(UINT32 u4FBId);
BOOL FBM_GetFromDispQueue(UINT32 *pu4FBId);
BOOL FBM_GetDispQueueNum(UINT32 *pu4FBNum);

BOOL FBM_GetFBufTotalNum(UINT32 *pu4FBNum);

INT32 FBM_GetDpbIdx(UINT32 u4YAddr, UINT32 u4CAddr);

#endif  // __VDEC_FBM_H__

