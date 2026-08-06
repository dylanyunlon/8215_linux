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
#ifndef _IPOD_CONTROLLER_H_
#define _IPOD_CONTROLLER_H_

#include "handle.h"
#include "u_ipod_def.h"
/*
typedef enum
{
    IPOD_AUTHENTICATION_PASS,
    IPOD_AUTHENTICATION_FAIL
}E_IPOD_AUTHENTICATION_STATUS;
*/
#define MULTI_INFO_NOTIFY_ENABLE 1
#if !MULTI_INFO_NOTIFY_ENABLE
typedef void (*IPOD_AUTH_CB)(E_IPOD_AUTHENTICATION_STATUS eStatus);
#else
typedef void (*IPOD_AUTH_CB_EX)(E_IPOD_NOTIFY_INFO_E eInfoType,
                                                void *pvInfo);
#endif


/*******************************************************************************
**  Function     : u4CreateIpodCtrlInstance
**  descriptions : Create Ipod Controller Instance
**  parameters   : h_obj            [IN]  iPod Handle Get from DM
**                 IpodCtrlInst     [OUT] iPod Controller Instance
**  return       : TRUE             Create Instance Successfully
**                 FALSE            Create Instance Failed
*******************************************************************************/
UINT32 u4CreateIpodCtrlInstance(HANDLE_T h_obj, HANDLE_T* IpodCtrlInst);


/*******************************************************************************
**  Function     : vDestroyIpodCtrlInstance
**  descriptions : Create Ipod Controller Instance
**  parameters   : h_obj            [IN] iPod Handle Get from DM
**  return       : None
*******************************************************************************/
void vDestroyIpodCtrlInstance(HANDLE_T h_obj);


/*******************************************************************************
**  Function     : u4StartIpod
**  descriptions : Start ipod
**  parameters   : h_obj            [IN] iPod Handle Get from DM
**  return       : TRUE             Start iPod Successfully
**                 FALSE            Start iPod Failed
*******************************************************************************/
#if !MULTI_INFO_NOTIFY_ENABLE
UINT32 u4StartIpod(HANDLE_T h_obj, IPOD_AUTH_CB pfIpodAuthCB);
UINT32 u4StartIpod_ex(HANDLE_T h_obj, BOOL fgIdpMode, UINT8 u1Category, IPOD_AUTH_CB pfIpodAuthCB);
#else
UINT32 u4StartIpod_ex(HANDLE_T h_obj, BOOL fgIdpMode, UINT8 u1Category, IPOD_AUTH_CB_EX pfIpodAuthCB);
#endif
/*******************************************************************************
**  Function     : u4StopIpod
**  descriptions : Stop ipod
**  parameters   : h_obj            [IN] iPod Handle Get from DM
**  return       : TRUE             Stop iPod Successfully
**                 FALSE            Stop iPod Failed
*******************************************************************************/
UINT32 u4StopIpod(HANDLE_T h_obj);


/*******************************************************************************
**  Function     : u4AbortIpod
**  descriptions : Cancel Authentication
**  parameters   : h_obj            [IN] iPod Handle Get from DM
**  return       : TRUE             Abort iPod Successfully
**                 FALSE            Abort iPod Failed
*******************************************************************************/
UINT32 u4AbortIpod(HANDLE_T h_obj);
//new aded by binliu.
UINT32 u4IpodCtrlSetCurEQProfileIndex(HANDLE_T h_obj, UINT32 u4EQPorfileIndex);
UINT32 u4IpodCtrlSetRemoteEvent(HANDLE_T h_obj, UINT32 u4RemEventMask);
UINT32 u4IpodCtrlSetDBHirachyLayer(HANDLE_T h_obj, UINT32 u1HirachyLayer);
UINT32 u4IpodCtrlEnterRemoteUIMode(HANDLE_T h_obj);
UINT32 u4IpodCtrlExitRemoteUIMode(HANDLE_T h_obj);
UINT32 u4IpodCtrlResetDBSelection(HANDLE_T h_obj);
/*******************************************************************************
**  Function     : u4IpodCtrlSetRepeatMode
**  descriptions : Cancel Authentication
**  parameters   : h_obj            [IN] iPod Handle Get from DM
**   parameters  :u1ShuffleMode
**        0-------- "Repeat off",
 **      1---------"Repeat one track",
 **      2-------  -"Repeat all tracks"
**   parameters   :  fgRestore
***                            True--->keep setting after ipod plug stop
*******************************************************************************/
UINT32 u4IpodCtrlSetRepeatMode(HANDLE_T h_obj,UINT8 u1RepeatMode , BOOL fgRestore);
/*******************************************************************************
**  Function     : u4IpodCtrlSetShuffleMode
**  descriptions : Cancel Authentication
**  parameters   : h_obj            [IN] iPod Handle Get from DM
**   parameters  :u1ShuffleMode
**        0-------- "Shuffle off",
 **      1---------"Shuffle tracks",
 **      2----------"Shuffle albums"
**  return       : TRUE             Abort iPod Successfully
**                 FALSE            Abort iPod Failed
*******************************************************************************/
UINT32 u4IpodCtrlSetShuffleMode(HANDLE_T h_obj,UINT8 u1ShuffleMode, BOOL fgRestore);
UINT32 u4IpodCtrlPlayCurrentSelection(HANDLE_T h_obj, UINT32 u4SelectTrackRecIndex);
UINT32 u4IpodCtrlSetPlayStatusChangeNotification(HANDLE_T h_obj, UINT32 u4ParamIndex,UINT32 u4Parmeter);
UINT32 u4IpodCtrlSetCurrentPlayingtrack(HANDLE_T h_obj, UINT32 u4CurPlayingTrackIndex);
#if CONFIG_MW_CUSTOM_KLG
UINT32 u4IpodCtrlSetAvailableCurrent(HANDLE_T h_obj, UINT32 u4CurrentLimit);
UINT32 u4IpodCtrlSetPreference(HANDLE_T h_obj, UINT32 u4ClassId);
#endif
UINT32 u4IpodCtrlSetVideoDelayMs(HANDLE_T h_obj, UINT32 u4DelayMsec);
UINT32 u4IpodCtrlSelectDBRecord(HANDLE_T h_obj, UINT8 u1DbCategaroy,UINT32 u4RecIndex );
UINT32 u4IpodCtrlSelectSortDBRecord(HANDLE_T h_obj, UINT8 u1DbCategaroy,UINT32 u4RecIndex ,UINT8 u1SortOrder);
UINT32 u4IpodCtrlPlayControl(HANDLE_T h_obj,UINT32 u1Control);
UINT32 u4IpodCtrlCleanAudBuf(HANDLE_T h_obj);
UINT32 u4IpodCtrlCleanAudBuffer(HANDLE_T h_obj, UINT32 u4MuteDataLen);
#if CONFIG_MW_CUSTOM_KLG
UINT32 u4IpodCtrlSetImageData(HANDLE_T h_obj,
                                                           UINT8  u1PixFmtCode ,
                                                            UINT16 u2DescriptIndex,
                                                            UINT16 u2DataLenth,                             
                                                            UINT16 u2ImgWidth,
                                                            UINT16 u2ImgHeight,
                                                            UINT32 u4RowByteSize,
                                                            void   *pvImageData
                                                           );
#endif
/*******************************************************************************
**  Function     : u4IpodCtrlQueryUsbAudHandle
**  descriptions : Query Repsond Usb Audio Driver Handle of the iPod
**  parameters   : h_obj            [IN] iPod Handle Get from DM
**  return       : Usb Audio Driver Handle
*******************************************************************************/
UINT32 u4IpodCtrlQueryUsbAudHandle(HANDLE_T h_obj);

/*******************************************************************************
**  Function     : u4IpodCtrlGetNumEQProfile
**  descriptions : get the number of EQ profile of iPod
**  parameters   : h_obj            [IN] iPod Handle Get from DM
**  return       : number of EQ profile
*******************************************************************************/
UINT32 u4IpodCtrlGetNumEQProfile(HANDLE_T h_obj);

UINT32 u4IpodCtrlGetCurEQProfileIndex(HANDLE_T h_obj);

UINT32 u4IpodCtrlGetNumPlayingTracks(HANDLE_T h_obj);
UINT32 u4IpodCtrlGetNumPlayingTracks_ex(HANDLE_T h_obj);
UINT64 u8IpodCtrlGetIpodOptionForLingo(HANDLE_T h_obj, UINT8 u1LingoId);
UINT32 u4IpodCtrlRequestRemoteUIModeo(HANDLE_T h_obj, BOOL *pfgOsdMode);

UINT32 u4IpodCtrlGetTrackArtWorkTimes(
               HANDLE_T h_obj,
               UINT32 u4TrackIndex,
               UINT16 u2FmtID,
               UINT16 u2ArtWorkIndex,
               UINT16 u2ArtWorkCount,
                MUSB_IPOD_ARTWORK_TIMES_T *prTimes);
UINT32 u4IpodCtrlGetIndexedPlayingTrackInfo(
               HANDLE_T h_obj,
               UINT32 u4TrackIndex,
               UINT16 u2ChapterIndex,
               MUSB_TRACK_INF_TYPE_E eGetInfoType,
               VOID *pvGoInfo);
/*******************************************************************************
**  Function     : u4IpodCtrlGetNumberCategorizedDBRecords
**  descriptions : get the number of EQ profile of iPod
**  parameters   : h_obj            [IN] iPod Handle Get from DM
**  parameters   : u1Category
**                 0 --Resverd
**                 1 --Playlist
**                 2 --Artist
**                 3 --Album
**                 4 --Genre
**                 5 --Track
**                 6 --Composer
**                 7 --Audio Book
**                 8 --local casting
**                 9 --Nested playlist
**  return       : number of CategorizedDBRecords
*******************************************************************************/
UINT32 u4IpodCtrlGetNumberCategorizedDBRecords(
                  HANDLE_T h_obj,
                  UINT8 u1DbCategory);


UINT32 u4IpodCtrlRetrieveCategorizedDBRecords(HANDLE_T h_obj,
                            UINT8 u1DbCategory,
                            UINT32 u4StartRecIdx, 
                            UINT32 u4RecCnt,
                            VOID *pvInfo);

UINT32 u4IpodCtrlGetShuffleSetting(HANDLE_T h_obj,    UINT8 *pu1ShuffMode); 
UINT32 u4IpodCtrlGetRepeatSetting(HANDLE_T h_obj,    UINT8 *pu1RepeatMode) ;
UINT32 u4IpodCtrlGetCurPlayingTrackIdx(HANDLE_T h_obj,    UINT32 *pu4CurTrackIndex) ;
UINT32 u4IpodCtrlGetIndexedPlayingTrackTitle(HANDLE_T h_obj, 
                                                                                   UINT32 u4PlaybackTrackIndex,
                                                                                   CHAR *pszCurTrackTitle) ;
UINT32 u4IpodCtrlGetIndexedPlayingTrackArtist(HANDLE_T h_obj, 
                                                                                   UINT32 u4PlaybackTrackIndex, 
                                                                                   CHAR *pszCurTrackArtist) ;
UINT32 u4IpodCtrlGetIndexedPlayingTrackAlbum(HANDLE_T h_obj, 
                                                                                  UINT32 u4PlaybackTrackIndex,
                                                                                  CHAR *pszCurTrackAlbum)  ;
UINT32 u4IpodCtrlRequestLingoProtocolVersion(HANDLE_T h_obj, 
                                                                                   UINT8 u1LingoID,                                  
                                                                                   UINT32 *pu4Version) ;
UINT32 u4IpodCtrlGetCurPlayingTrackChapterInfo(HANDLE_T h_obj, 
                                                                                    VOID *pvCharpterInfo);
UINT32 u4IpodCtrlGetDbTrackinfo(HANDLE_T h_obj, 
                                                          UINT32 u4TrackDbStarIndex,
                                                          UINT32 u4TrackCount,
                                                          UINT8 u1TrackInfoBit,
                                                          VOID *pvGotInfo);
UINT32 u4IpodCtrlIPodGetPlayStatus(HANDLE_T h_obj, MUSB_IPOD_PLAY_ST_INFO_T *prPbStatus);
UINT32 u4IpodCtrlIPodGetPlayStatus_Ex(HANDLE_T h_obj, MUSB_IPOD_PLAY_ST_INFO_EX_T *prPbStatus);
UINT32 u4IpodCtrlGetTrackArtWorkData(HANDLE_T h_ob,
                                      UINT32 u4TrackIndex,
                                      UINT16 u2FmtID,
                                      UINT32 u4MsTime,
                                       void *pvArtWorkData
                                      );
UINT32 u4IpodCtrlGetArtWorkFormats(HANDLE_T h_obj, VOID *pvGotInfo);// got inff is MUSB_IPOD_ARTWORK_FMTS
#if CONFIG_MW_CUSTOM_KLG
UINT32 u4IpodCtrlGetReferenceInfo(HANDLE_T h_obj, UINT8 u1ClassID, MUSB_IPOD_REFERENCE_INFO *prGotInfo);
UINT32 u4IpodCtrlGetModelInfo(HANDLE_T h_obj, MUSB_IPOD_MODEL_INFO *prGotInfo);
#endif

UINT32 u4IpodCtrlIPodGetAuthStatus(HANDLE_T h_obj, MUSB_AUTH_RESULT_T *prAuthResult);
/*******************************************************************************
**  Function     : u4IpodCtrlSendIpodKeyCmd
**  descriptions : Send iPod Key Command to iPod Client Driver
**  parameters   : h_obj             [IN] iPod Handle Get from DM
**                 bCmdID            [IN] Repond Command ID of Current Key
**  return       : Usb Audio Driver Handle
*******************************************************************************/
UINT32 u4IpodCtrlSendIpodKeyCmd(HANDLE_T h_obj, UINT8 bCmdID);
UINT32 u4IpodCtrlSendIpodKeyCmdEx(HANDLE_T h_obj, UINT8 bCmdID,BOOL fgEnableKeyOff);

#endif  /* End of _IPOD_CONTROLLER_H_*/
