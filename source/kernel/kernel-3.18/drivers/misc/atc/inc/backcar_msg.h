#ifndef __BACKCAR__MSG__H_
#define __BACKCAR__MSG__H_


#define WM_FSC_BACKCAR          (WM_USER + 0X7000)
#define FSC_BACKCAR_STATUS_FIN         (0X2)
#define FSC_BACKCAR_STATUS_START    (0X4)



#define WM_VIDEORENDER_MSG                    (WM_USER + 0X7002)
#define VIDEORENDER_MSG_RELEASE_VDP_FIN         (0X2)



#define WM_FSC_BACKCAR_ACK_MSG                (WM_USER + 0X7004)
#define FSC_BACKCAR_ACK_RELEASE_RES_FIN         (0X2)

#ifdef SAMPLE_CODE

  PostMessage(HWND_BROADCAST,WM_FSC_BACKCAR_ACK_MSG,FSC_BACKCAR_ACK_RELEASE_RES_FIN,0);

#endif
#define BACKCAR_START_EVENT       TEXT("MTK_BACKCAR_START_EVENT")

/****************************************************************************************/

#define ARM2_STATUS_BACKING_CAR     1
#define ARM2_STATUS_NO_BACK_CAR     2

typedef enum _BACKCARMSG
{
    MSG_UNKNOWN = 0,
    MSG_ARM2_RESPONSE,
    MSG_ANDROID_APP_READY,
    MSG_ANDROID_GET_MCU_INFO,
    MSG_GET_ARM2_VERSION,
    MSG_NOTIFY_ARM2_STOP,
    MSG_NOTIFY_ARM2_BUFF_MEMSET,
    MSG_NOTIFY_ARM2_ANIMATION_STOP
}BACKCARMSG;

typedef enum _SHAREMEM_TYPE
{
    SHAREMEM_UNKNOWN = 0,
    SHAREMEM_MCU_INFO,
    SHAREMEM_ARM2_VERSION	
}SHAREMEM_TYPE;

typedef struct tagSHAREMEM
{
    UINT32          u4Size;           /*the structure size*/
    SHAREMEM_TYPE   ShareMemT;
    UINT32          u4ResponeStatus;
    UINT32          u4BufferSize;
    UINT8           Data[4];
}SHAREMEM;


/****************************************************************************************/


#endif
