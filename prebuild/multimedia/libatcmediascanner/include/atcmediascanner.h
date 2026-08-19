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

#ifndef ATC_MEDIASCANNER_H
#define ATC_MEDIASCANNER_H

#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <ctime>
#include <map>
#include <list>
#include <string.h>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>
#include <stdexcept>
#include <openssl/md5.h>
#include <semaphore.h>
#include "atcsema.h"

#define WAIT_SCAN   true
#define ON_SCANNING false

struct VideoInfo {
    std::string filename;  // File name
    std::string duration;  // Duration in HH:MM:SS format
};

enum CmdType {
    UnknownCmd,    // unknown type
    ScanCmd,
    AddCmd,
    RemoveCmd
};
enum scan_state{
    IDLE,
    SCANING,
    SCANDONE,
    ErrorState
};
enum CmdFlag {
    IdleFlag,
    InDoingFlag,
    InWaitingFlag
};

typedef struct {
    CmdType eType;
    CmdFlag eFlag;
    bool    is_async;
    std::string path;
    std::list<std::string> videoNames;
} Command;

class AtcMediaScanner {
public:
    AtcMediaScanner();
    virtual ~AtcMediaScanner();
    bool  setup();
    void              scanDirectory(const std::string& path);
    void deleteVideos(const std::string& path, const std::list<std::string>& files);
    void addVideos(const std::string& path, const std::list<std::string>& files);
    std::list<VideoInfo>  getScanDataList(const std::string& path);
    scan_state getScanState(const std::string& path);
    std::unique_ptr<Command> getCommand(bool *need_exit);
    void  replyCmd(Command * cmd);
    void processAddFilesToList(Command* cmd);
    void processRemoveFilesFromList(Command* cmd);
    void processScanCmd(Command* cmd);
private:

    bool sendCmd(std::unique_ptr<Command>  insertcmd);
    bool removeCmds(unsigned int  u4CmdsMask, const std::string& path);

    void setState(const std::string& path,scan_state state);

    bool    m_pauseflag;
    bool    m_scanflag;
    bool    m_exitScanThread;
    std::map<std::string,std::list<VideoInfo>> m_VideosMap;
    std::map<std::string,std::string> m_MD5sMap;
    std::map<std::string, scan_state> m_StateMap;
    static bool m_firstvideo;
    std::list<std::unique_ptr<Command>> m_cmdList;
    pthread_t         m_cmdThread;
    bool                       m_cmdThreadExit;
    std::mutex           m_cmdMutex;
    std::mutex           m_statusMutex;
    std::mutex m_listMutex;
    ATCSEMA  *cmd_sema;
    ATCSEMA  *reply_sema;
};

#endif
