/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#ifndef ATC_GSTREAMERPLAYER_H
#define ATC_GSTREAMERPLAYER_H


#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <locale.h>

//#include "atcmutex.h"
//#include "atcmediadefs.h"
//#include "atcmediadumpctrl.h"
#include <linux/fb.h>
#include "avcodec.h"
#include "display.h"
#include <dirent.h>
#include "MMDataSource.h"
#include "atcsema.h"
#include <list>
#include <mutex>
#include <thread>
#include <queue>


//#define PRINT_INFO(format, ...)   fprintf(stderr, "[I][MM][ATCMEDIAPLAYER][%s:%d] " format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
//#define PRINT_ERROR(format, ...)  fprintf(stderr, "[E][MM][ATCMEDIAPLAYER][%s:%d] " format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define DEC_ALIGN_MASK(value, mask)			((((value) + ((mask) - 1)) / (mask)) * (mask))

class FileSource {
public:
    // FileSource takes ownership and will close the fd
    FileSource(int fd, int64_t offset, int64_t length);

    virtual ssize_t initCheck() const;

    virtual ssize_t readAt(off64_t offset, void *data, size_t size);

    virtual ssize_t getSize(off64_t *size);

    virtual void SetNeedReadTimeoutFlag(bool flag);
    virtual ~FileSource();
protected:
    virtual ssize_t readAt_l(off64_t offset, void *data, size_t size);

    int mFd;
    int64_t mOffset;
    int64_t mLength;
    //UnitTestMsdkMutex mLock;
    bool mNeedReadTimeoutFlag;
private:

    FileSource(const FileSource&);
    FileSource& operator=(const FileSource&);

};
class DataSourceProxy: public MMDataSource
{
public:
    DataSourceProxy(FileSource *source);

    virtual __s32 initCheck() const;
    virtual off64_t getFileOffset();
    virtual ssize_t readAt(off64_t offset, void *data, size_t size);
    virtual __s32 getSize(off64_t *size);
    virtual void    vTerminateRead();
protected:
    virtual ~DataSourceProxy() {};
private:
    DataSourceProxy(const DataSourceProxy&);
    DataSourceProxy& operator=(const DataSourceProxy&);

    FileSource *mDataSource;
    //UnitTestMsdkMutex          mReadLock;
};

enum CmdType
{
  UnknownCmd,	 //unkown type
  PreparedCmd,
  PlayCmd,
  StopCmd,
  PauseCmd,
  SeekCmd
};

enum CmdFlag
{
  IdleFlag,
  InDoingFlag,
  InWaitingFlag
};

typedef struct {
  CmdType eType;
  CmdFlag eFlag;
  double pos;
  bool	  is_async;
  bool	is_errchunk;
} Command;

// Forward declaration
class MediaPlayer;

// State change callback function type (must be outside class for proper linkage)
typedef void (*MediaPlayerStateCallback)(int newState, void* userData);

class MediaPlayer {
public:
  
    enum media_play_state {
        PausedState,    /* it is ready to play or process data, or the pause function is called*/
        PlayingState,   /* it is playing or processing data */
        StoppedState,   /* No media to play, or if the stop function is called, it will go to this state */
        ErrorState,     /* Error State, any operation will be error while in this state */
    };
	
	MediaPlayer();
	~MediaPlayer();
	bool setVideoSurface(int width, int height, int x = 0, int y = 0);
	bool setup();
	// Load and start playing a TS file
	void play(const std::string &filepath);
	// Pause playback
	void pause();
	// resume playback
	void resume();
	// Stop playback
	void stop();
	// Seek to position (milliseconds) 
	void seek(double positionMs);
	// Set state change callback
	void setStateCallback(MediaPlayerStateCallback callback, void* userData); 
	// Set a callback to report progress (position, duration)
	//void setProgressCallback(std::function<void(double, double)> cb);
	// Query current position and duration
	double getPosition();
	//double getDuration();
	void parserThreadFunc();
    Command *getCommand(bool *need_exit);
    void  replyCmd(Command * cmd);
	void processPlayCmd(Command *cmd);
	void processPauseCmd(Command *cmd);
	void processSeekCmd(Command *cmd);
	void processStopCmd(Command *cmd);
private:
	enum State {
		Stopped,
		Playing,
		Paused
	} state_;
	// Worker threads
	media_play_state  m_playState;
    std::list<Command* > m_cmdList;
    pthread_t         m_cmdThread;
    std::mutex           m_cmdMutex;
    ATCSEMA  *cmd_sema;
    ATCSEMA  *reply_sema;
    bool                       m_cmdThreadExit;
	void startThreads();
	void joinThreads();
	std::string filepath_;
	bool exitThreads_;
	// Seek control
	bool seekRequested_;
	double seekPositionMs_;
	// Thread synchronization
	pthread_mutex_t mutex_; 
	pthread_cond_t cond_;
	//std::condition_variable cv_; 
	pthread_t parserThread_;
	//pthread_t decoderThread_;
	IAtcSurface *video_surface;
    AVCodec *codec;
    //FileSource *m_source;
    //MMDataSource *m_dataSource;
    
    // State callback related members
    MediaPlayerStateCallback m_stateCallback;
    void* m_callbackUserData;
    
    void DataReceived(__u8 *data, __u32 len, int64_t pts, void *pvarg);
    static void* onCommandReceived(void* pvarg);
    bool sendCmd(Command *insertcmd);
    bool removeCmds(unsigned int  u4CmdsMask);
    bool clearAllCmds();
    
    // Internal function to notify state changes
    void notifyStateChanged(media_play_state newState);
};



#endif // ATC_GSTREAMERPLAYERSESSION_H

