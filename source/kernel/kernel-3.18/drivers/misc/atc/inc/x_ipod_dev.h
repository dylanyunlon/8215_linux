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

#ifndef _X_IPOD_DEV_H_
#define _X_IPOD_DEV_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "x_common.h"
#include "drv_def.h"
#include "x_drv_cb.h"
#include "u_ipod_def.h"

/*-----------------------------------------------------------------------------
 * Get types
 *---------------------------------------------------------------------------*/

#define IPODDEV_GET_TYPE_DEVICE_NUM                      (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 0))

#define IPODDEV_GET_TYPE_DEVICE_STATUS                   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 1))

#define IPODDEV_GET_TYPE_USBAUD_HANDLE                   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 2))
//INT32 _IpodCliGetEQProfile( );
#define IPODDEV_GET_TYPE_NUM_EQ_PROFILE                      (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 3))
//INT32 _IpodCliGetCurEQProfileIndx( );  
#define IPODDEV_GET_TYPE_EQ_PROFILE_INDEX                (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 4))//INT32 _IpodCliGetPlaySt( );#define IPODDEV_GET_TYPE_PLAY_STATUS                     (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 5))//INT32 _IpodCliGetPlayStEx( );#define IPODDEV_GET_TYPE_PLAY_STATUS_EX                  (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 6))
//INT32 _IpodCliGetNumPlayTracks( );
#define IPODDEV_GET_TYPE_NUM_TRACKS                      (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 7))
//INT32 _IpodCliGetOptionForLingo( );
#define IPODDEV_GET_TYPE_OPTION_FOR_LINGO                (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 8))
//INT32 _IpodCliRequestRemoteUIMode( );
#define IPODDEV_GET_TYPE_REQ_REMOTE_UI_MODE              (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 10))//INT32 _IpodCliRequestLingoProtocolVersion( );#define IPODDEV_GET_TYPE_REQ_LINGO_VERSION               (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 11))
//INT32 _IpodCliGetIndexPlayingTrackInfo( );
#define IPODDEV_GET_TYPE_PLAYING_TRACK_INFO              (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 12))
//INT32 _IpodCliGetTrackArtWorkTimes( );
#define IPODDEV_GET_TYPE_TRACK_ARTWORK_TIMES             (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 13))//INT32 _IpodCliGetTrackArtData( );#define IPODDEV_GET_TYPE_TRACK_ARTWORK_DATA              (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 14))//INT32 _IpodCliGetArtWorkFormat( );  #define IPODDEV_GET_TYPE_TRACK_ARTWORK_FORMAT            (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 15))
//INT32 _IpodCliGetNumCateDBRecords();
#define IPODDEV_GET_TYPE_NUM_CATEDB_RECORD               (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 16))
//INT32 _IpodCliRetrivCateDBRecords();
#define IPODDEV_GET_TYPE_RETRIVE_CATEDB_RECORD           (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 17))
//INT32 _IpodCliGetShuffle( );
#define IPODDEV_GET_TYPE_SHUFFLE                         (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 18))
//INT32 _IpodCliGetRepeat( );
#define IPODDEV_GET_TYPE_REPEAT                          (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 19))
//INT32 _IpodCliGetCurPlayingTrackIdx( ); 
#define IPODDEV_GET_TYPE_PLAYING_TRACK_INDEX             (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 20))
//INT32 _IpodCliGetNumPlayingTracks_ExLingo( );
#define IPODDEV_GET_TYPE_NUM_PLAYING_TRACKS_EX           (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 21))
//INT32 _IpodCliGetIndexedPlayingTrackTitle( );
#define IPODDEV_GET_TYPE_INDEXED_PLAYING_TRACKS_TITLE    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 22))
//INT32 _IpodCliGetIndexedPlayingTrackArtistName( );
#define IPODDEV_GET_TYPE_INDEXED_PLAYING_TRACKS_ARTIST   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 23))
//INT32 _IpodCliGetIndexedPlayingTrackAlbumName( );
#define IPODDEV_GET_TYPE_INDEXED_PLAYING_TRACKS_ALBUM    (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 24))
//INT32 _IpodCliGetDbTrackInfo( );
#define IPODDEV_GET_TYPE_DB_TRACK_INFO                   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 25))

#define IPODDEV_GET_TYPE_LIGO_PROTOL_VERSION                   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 26))

#define IPODDEV_GET_TYPE_PLAYING_TRACK_CHAPTER_INFO                   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 27))

#define IPODDEV_GET_TYPE_PLAY_STATUS                   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 28))

#define IPODDEV_GET_TYPE_PLAY_STATUS_EX                   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 29))

//static INT32 _IpodCliGetTrackArtData(INT32 i4Argc, const CHAR ** szArgv);
#define IPODDEV_GET_TYPE_ARTWORK_DATA                   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 30))

//static INT32 _IpodCliGetArtWorkFormat(INT32 i4Argc, const CHAR ** szArgv);
#define IPODDEV_GET_TYPE_ARTWORK_FMT                   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 31))

#if CONFIG_DRV_CUSTOM_KLG
#define  IPODDEV_GET_TYPE_PREFERENCE                        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 32))
#define  IPODDEV_GET_TYPE_MODELINFO                        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 33))
#endif

#define IPODDEV_GET_TYPE_AUTH_STATUS                   (RM_GET_TYPE_LAST_ENTRY + ((DRV_GET_TYPE_T) 34))


/*-----------------------------------------------------------------------------
 * Set types
 *---------------------------------------------------------------------------*/
#define IPODDEV_SET_TYPE_HOTSWAP_NFY    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 0))

#define IPODDEV_SET_TYPE_START_IPOD     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 1))

#define IPODDEV_SET_TYPE_SEND_KEY       (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 2))

#define IPODDEV_SET_TYPE_STOP_IPOD      (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 3))

#define IPODDEV_SET_TYPE_ABORT_IPOD      (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 4))

#define IPODDEV_SET_TYPE_CREATE_IPOD    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 5))

#define IPODDEV_SET_TYPE_DESTROY_IPOD    (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 6))
//INT32 _IpodCliSetCurEQProfileIndx( );
#define IPODDEV_SET_TYPE_EQ_PROFILE_INDEX            (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 7))
// INT32 _IpodCliSetRemoteEvent( );
#define IPODDEV_SET_TYPE_REMOTE_EVENT                 (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 8))
//INT32 _IpodCliSiwtichDbH( );
#define IPODDEV_SET_TYPE_SWITH_DB_HIRACHY             (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 9))
//INT32 _IpodCliEnterRemoteUIMode( );
#define IPODDEV_SET_TYPE_ENTER_REMOTE_UI_MODE         (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 10))
//INT32 _IpodCliExitRemoteUIMode( );
#define IPODDEV_SET_TYPE_EXIT_REMOTE_UI_MODE          (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 11))
//INT32 _IpodCliResetDBSelection( );
#define IPODDEV_SET_TYPE_REST_DB_SELECTION            (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 12))
//INT32 _IpodCliSetShuffle( );
#define IPODDEV_SET_TYPE_SHUFFLE                      (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 13))
// INT32 _IpodCliSetRepeat( )
#define IPODDEV_SET_TYPE_REPEAT                       (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 14))
// INT32 _IpodCliPlayCurrentSelection( );
#define IPODDEV_SET_TYPE_PLAY_CUR_SECLETION           (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 15))
// INT32 _IpodCliSetPlayStatusChangeNotification( );
#define IPODDEV_SET_TYPE_PLAYBACK_STATUS_NOTIFY       (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 16))
// INT32 _IpodCliSetCurrentPlayingtrack( );
#define IPODDEV_SET_TYPE_CURRENT_PLAYING_TRACK        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 17))
// INT32 _IpodCliSetVideoDelay( );
#define IPODDEV_SET_TYPE_VIDEO_DELAY                  (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 18))
// INT32 _IpodCliSelectDBRecord( );
#define IPODDEV_SET_TYPE_SELECT_DB_RECORD             (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 19))
 // INT32 _IpodCliSelectSortDBRecord( );
#define IPODDEV_SET_TYPE_SELECT_SORT_DB_RECORD        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 20))

#define  IPODDEV_SET_PB_CONTROL                        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 21))

#define  IPODDEV_SET_CLEAN_AUD_BUF                     (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 22))
#if CONFIG_DRV_CUSTOM_KLG
#define  IPODDEV_SET_PREFERENCE                        (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 23))

#define  IPODDEV_SET_AvailableCurrent                  (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 24))

#define IPODDEV_SET_IMAGE   (RM_SET_TYPE_LAST_ENTRY + ((DRV_SET_TYPE_T) 25))

#endif
#endif /* _X_IPOD_DEV_H_ */
