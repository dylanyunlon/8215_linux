#ifndef ARM2_COMM_DATA_STRUCT_H_
#define ARM2_COMM_DATA_STRUCT_H_

#define TVD_EVT_NAME_MAX_LENGTH        20

typedef void (*PFNArm2EvtCBFunc)(u32 para);

typedef struct
{
   u8               szEvtName[20];
   PFNArm2EvtCBFunc pfEvtCallBack;
   u32              u4EvtData;
} ARM2_EVT_T, *PARM2_EVT_T;


#endif
