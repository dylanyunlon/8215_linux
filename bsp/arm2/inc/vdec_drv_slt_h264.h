#ifndef _VDEC_DRV_SLT_h264_H_
#define _VDEC_DRV_SLT_h264_H_
struct DecCropParams {
    /** The left offset value */
    UINT32 crop_left_offset;
    /** The wigth value */
    UINT32 crop_out_width;
    /** The top offset value */
    UINT32 crop_top_offset;
    /** The height value */
    UINT32 crop_out_height;
};
typedef struct tagVidInfo
{
    UINT32 u4VDPDstYPA;
    UINT32 u4VDPDstCPA;
    UINT32 width;
    UINT32 height;
}VIDINFO;
#define MAX_OUTPUT 5
//#define DEBUG_VDEC
#define VDEC_USE_IRQ
extern void getcrop(struct DecCropParams *p_crop);
void resetHW();
extern BOOL fgVDec_SLT_AVC_Proc(UINT32 u4VFifoStartAddr, UINT32 u4VFifoEndAddr, UINT32 u4AuStart, UINT32 u4AuSize, UINT32 *u4VDPDstYPA, UINT32 *u4VDPDstCPA,
	UINT32 *width, UINT32 *height);
void waitVdecHwReady(VIDINFO *vidInfo);
void decode(UINT32 u4VFifoStartAddr, UINT32 u4VFifoEndAddr, UINT32 u4AuStart, UINT32 u4AuSize);
#endif

