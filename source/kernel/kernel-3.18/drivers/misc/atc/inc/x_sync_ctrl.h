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

#ifndef _X_SYNC_CTRL_H_
#define _X_SYNC_CTRL_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/* Set operations */
#define SYNC_CTRL_SET_TYPE_DCC_INFO             (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 0))
#define SYNC_CTRL_SET_TYPE_REQ_TIME_TRIGGER     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 1))
#define SYNC_CTRL_SET_TYPE_CANCEL_TIME_TRIGGER  (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 2))
#define SYNC_CTRL_SET_TYPE_TIME_TRIGGER_NFY_FCT (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 3))
#define SYNC_CTRL_SET_TYPE_FRAMEACCURATE_PTS (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 4))
#define SYNC_CTRL_SET_TYPE_FRAMEACCURATE_DONE_NFY_FCT (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 5))
#define SYNC_CTRL_SET_TYPE_AVNOSYNC (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 6))
#define SYNC_CTRL_SET_TYPE_STCUPDATE_FREQUENCY (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 7))
#define SYNC_CTRL_SET_TYPE_STCUPDATE_STEP (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 8))
#define SYNC_CTRL_SET_TYPE_VIDEO_OUTPUT_EARLIER (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 9))

/* Get operations */
#define SYNC_CTRL_GET_TYPE_PRESENTATION_TIME    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 4))
#define SYNC_CTRL_GET_TYPE_SYSTEM_STC           (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 5))

#define _SYNCTRL_ADD_TRANSID_ 1
#define _SYNCTRL_ADD_TRANSID_FOR_FMACCUR_ 1

/*-----------------------------------------------------------------------------
  SYNC_CTRL_SET_TYPE_DCC_INFO
-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
  SYNC_CTRL_SET_TYPE_REQ_TIME_TRIGGER / SYNC_CTRL_SET_TYPE_CANCEL_TIME_TRIGGER
-----------------------------------------------------------------------------*/

typedef enum
{
    SYNC_CTRL_DIR_UNKNOWN     = 0,
    SYNC_CTRL_DIR_FORWARD     = 1,
    SYNC_CTRL_DIR_BACKWARD    = 2
}   SYNC_CTRL_PB_DIR_T;

typedef enum
{
    SYNC_CTRL_TIMER1          = 0,
    SYNC_CTRL_TIMER2          = 1
}   SYNC_CTRL_TIMER_T;

typedef struct _SYNC_CTRL_TIME_CTRL_T
{
    SYNC_CTRL_TIMER_T         e_timer;
    SYNC_CTRL_PB_DIR_T        e_pb_dir; /* only for request timer */
    UINT64                    u8_stc;   /* only for request timer */
    UINT32                    u4_trans_id;
}   SYNC_CTRL_TIME_CTRL_T;

/*-----------------------------------------------------------------------------
  SYNC_CTRL_SET_TYPE_TIME_TRIGGER_NFY_FCT
-----------------------------------------------------------------------------*/
#if UNIFORM_DRV_CALLBACK
typedef struct _SYNC_CTRL_TIMER_NFY_INFO_T
{
    SYNC_CTRL_TIMER_T e_nfy_timer;
    UINT64            u8_req_stc;
    UINT64            u8_now_stc; 
    UINT32            u4_trans_id;
} SYNC_CTRL_TIMER_NFY_INFO_T;
#else
typedef VOID (*x_sync_ctrl_timer_nfy_fct)( VOID*             pv_nfy_tag,
                                           SYNC_CTRL_TIMER_T e_nfy_timer,
                                           UINT64            u8_req_stc,
                                           UINT64            u8_now_stc, 
                                           UINT32            u4_trans_id);

typedef struct _SYNC_CTRL_TIMER_NFY_INFO_T
{
    VOID*                     pv_nfy_tag;
    x_sync_ctrl_timer_nfy_fct pf_sync_ctrl_timer_nfy;
}   SYNC_CTRL_TIMER_NFY_INFO_T;
#endif

/*-----------------------------------------------------------------------------
  SYNC_CTRL_SET_TYPE_FRAMEACCURATE_PTS
-----------------------------------------------------------------------------*/

#if (_SYNCTRL_ADD_TRANSID_FOR_FMACCUR_)

typedef struct _SYNC_CTRL_FMACCU_PTS_T
{
    UINT64                    u8_pts;  
    BOOL                       fgBegin;      ///< TRUE: begin pts; FALSE: end pts
    UINT32                    u4_trans_id;
}   SYNC_CTRL_FMACCU_PTS_T;

#else

typedef struct _SYNC_CTRL_FMACCU_PTS_T
{
    UINT64                    u8_pts;  
    BOOL                       fgBegin;      ///< TRUE: begin pts; FALSE: end pts
}   SYNC_CTRL_FMACCU_PTS_T;

#endif

/*-----------------------------------------------------------------------------
  SYNC_CTRL_SET_TYPE_FRAMEACCURATE_DONE_NFY_FCT
-----------------------------------------------------------------------------*/
#if UNIFORM_DRV_CALLBACK
typedef struct _SYNC_CTRL_FMACCU_DONE_NFY_INFO_T
{
    UINT32            u4_trans_id;
} SYNC_CTRL_FMACCU_DONE_NFY_INFO_T;

#else

#if (_SYNCTRL_ADD_TRANSID_FOR_FMACCUR_)

typedef VOID (*x_sync_ctrl_fmaccu_done_nfy_fct)(VOID*  pv_nfy_tag,
                                                UINT32 u4_trans_id);
#else
typedef VOID (*x_sync_ctrl_fmaccu_done_nfy_fct)(VOID* pv_nfy_tag);
#endif

typedef struct _SYNC_CTRL_FMACCU_DONE_NFY_INFO_T
{
    VOID*                     pv_nfy_tag;
    x_sync_ctrl_fmaccu_done_nfy_fct pf_sync_ctrl_nfy;
}   SYNC_CTRL_FMACCU_DONE_NFY_INFO_T;

#endif

/*-----------------------------------------------------------------------------
  SYNC_CTRL_SET_TYPE_AVNOSYNC
-----------------------------------------------------------------------------*/

typedef struct _SYNC_CTRL_AVNOSYNC_T
{
    BOOL                       fgEnable;      ///< TRUE: AV no sync; FALSE: AV sync
}   SYNC_CTRL_AVNOSYNC_T;


/*-----------------------------------------------------------------------------
  SYNC_CTRL_SET_TYPE_STCUPDATE_FREQUENCY
-----------------------------------------------------------------------------*/

typedef struct _SYNC_CTRL_STCUPDATE_FREQUENCY_T
{
    INT32                       i4SpeedMode;    ///< i4SpeedMode = real speed * 100. Negative: reverse play.
    BOOL                       fgEnable;      ///< TRUE: turn on stc update by syncctrl; FALSE: turn off stc update by syncctrl
}   SYNC_CTRL_STCUPDATE_FREQUENCY_T;

/*-----------------------------------------------------------------------------
  SYNC_CTRL_SET_TYPE_STCUPDATE_STEP
-----------------------------------------------------------------------------*/

typedef struct _SYNC_CTRL_STCUPDATE_STEP_T
{
    BOOL                       fgForward;      ///< TRUE: forward; FALSE: reverse
}   SYNC_CTRL_STCUPDATE_STEP_T;

/*-----------------------------------------------------------------------------
  SYNC_CTRL_SET_TYPE_VIDEO_OUTPUT_EARLIER
-----------------------------------------------------------------------------*/

typedef struct _SYNC_CTRL_VIDEO_EARLIER_T
{
    BOOL                       fgVideoEarlier; ///< [IN] TRUE: Video Output Early; FALSE: According A/U PTS.
}   SYNC_CTRL_VIDEO_EARLIER_T;

/*-----------------------------------------------------------------------------
  SYNC_CTRL_GET_TYPE_SYSTEM_STC
-----------------------------------------------------------------------------*/

typedef struct _SYNC_CTRL_GET_SYSTEM_STC_T
{
    BOOL                       fgValud;      ///< TRUE: Current system stc is valid; FALSE: is invalid.
    UINT64                     u8CurrentSystemStc; ///< Current system stc
}   SYNC_CTRL_GET_SYSTEM_STC_T;

/*-----------------------------------------------------------------------------
  SYNC_CTRL_GET_TYPE_PRESENTATION_TIME
-----------------------------------------------------------------------------*/

/* UINT64 u8_stc */


 #endif /* _X_SYNC_CTRL_H_ */
