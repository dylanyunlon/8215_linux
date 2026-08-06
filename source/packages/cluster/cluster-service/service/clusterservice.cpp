#include "clusterservice.h"
#include "clustermanager.h"
#include "common.h"
#include <sys/statfs.h>
#include <dirent.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define ARG1 "arg1"
#define ARG2 "arg2"
#define ARG3 "arg3"
#define ARG4 "arg4"
#define ARG5 "arg5"
#define ARG6 "arg6"
#define ARG7 "arg7"
#define ARG8 "arg8"

#define MAP_CHANNEL_NAME "map.projection"
#define IVI_CHANNEL_NAME "cluster.transport"
#define ADAS_CHANNEL_NAME "idc.dms_server_cluster"
#define ADAS_CHANNEL_NAME "idc.dms_server_cluster"

#define SOCKET_PATH "/tmp/cluster_unix_socket"
#include "stb_image.h"

const static char *TAG = "ClusterService";

using cluster_utils::CLog;
#define ANCIL_FD_BUFFER(n) \
    struct { \
      struct cmsghdr h; \
      int fd[n]; \
    }

ClusterService::ClusterService()
    : m_manager(new ClusterManager(this))
{

}

ICluster *ClusterService::getInstance()
{
    static ClusterService *instance = new ClusterService;
    return instance;
}

void ClusterService::registerCallBack(IClusterCallBack *callback)
{
    m_callback = callback;
    IInterdomainChannelMonitor *idc_monitor = NULL;

    idc_monitor = IInterdomainChannelMonitor::get();
    if (!idc_monitor) {
        UTILS_LOGI(TAG, "ChannelMonitor::get() fail");
        return ;
    }

    pProjectionChannel = IInterdomainChannelProxy::get(MAP_CHANNEL_NAME);
    if (!pProjectionChannel) {
        UTILS_LOGI(TAG, "Failed to get map.projection channel proxy!\n");
        return ;
    }

    pIviTransportChannel = IInterdomainChannelProxy::get(IVI_CHANNEL_NAME);
    if (!pIviTransportChannel) {
        UTILS_LOGI(TAG, "Failed to get cluster.transport channel proxy!\n");
        return ;
    }

    pAdasTransportChannel = IInterdomainChannelProxy::get(ADAS_CHANNEL_NAME);
    if (!pAdasTransportChannel) {
        UTILS_LOGI(TAG, "Failed to get idc.dms_server_cluster channel proxy!\n");
        return ;
    }

    pProjectionChannel->addListener(this);
    pIviTransportChannel->addListener(this);
    pAdasTransportChannel->addListener(this);

}

void ClusterService::mediaPlayPause()
{
    UTILS_LOGI(TAG, "mediaPlayPause");
    m_manager->sendData(CommonConstant::MediaPlayPause);
}

void ClusterService::mediaPre()
{
    UTILS_LOGI(TAG, "mediaPre");
    m_manager->sendData(CommonConstant::MediaPre);
}

void ClusterService::mediaNext()
{
    UTILS_LOGI(TAG, "mediaNext");
    m_manager->sendData(CommonConstant::MediaNext);
}

void ClusterService::call()
{
    UTILS_LOGI(TAG, "call");
    m_manager->sendData(CommonConstant::Call);
}

//挂断电话
void ClusterService::handup()
{
    UTILS_LOGI(TAG, "handup");
    m_manager->sendData(CommonConstant::HandUp);
}

void ClusterService::onReceiveData(int fd, unsigned char cmd, const char *data, unsigned int length)
{
    UTILS_LOGI(TAG, "onReceiveData fd=%d, cmd=%d len=%d", fd, cmd, length);
    switch (cmd) {
    case CommonConstant::MusicStatus:
        m_callback->onMusicStateChanged(data[0]);
        break;
    case CommonConstant::MusicName:
        m_callback->onMusicNameChanged(std::string(data, length));
        break;
    case CommonConstant::CallStatus:
        m_callback->onCallStatusChanged(data[0]);
        break;
    case CommonConstant::CallNumber:
        m_callback->onCallNumberChanged(std::string(data, length));
        break;
    case CommonConstant::CallPersonName:
        m_callback->onCallPersonChanged(std::string(data, length));
        break;
    case CommonConstant::CallTime:
        m_callback->onCallTimeChanged(std::string(data, length));
        break;
    case CommonConstant::CallPicture:
        m_callback->onCallPixmapChanged(reinterpret_cast<const unsigned char*>(data), length);
        break;

    case CommonConstant::DownloadUpdatePackage: {
        syncFile(data);
        startUpdate(std::string(data, length));
        break;
        }

    case CommonConstant::DownloadUpdatePackageCheck:
        checkUpdateState(fd, data, length);
        break;
    }
}

void ClusterService::onReceiveReplyTimeOut(int fd, unsigned char cmd)
{
    UTILS_LOGI(TAG, "onReceiveReplyTimeOut %d, %d", fd, cmd);
    m_callback->onDisconnect();
}

void ClusterService::startService()
{
    UTILS_LOGI(TAG, "startService");
    m_manager->start();
}

void ClusterService::onEvent(const char *domain, const char *channel,
                                     idc_event_t *event) {
    printf("[raul] server test app %s -> domain: %s, channel: %s, event id: %d\n",
           __func__, domain, channel, (int)event->id);
}

void ClusterService::onEvent(IInterdomainChannelProxy *proxy,
                                   idc_event_t *event, void *reply) {

    UTILS_LOGI(TAG, "------------------------------onEvent(): %s -> event: %u\n", __func__, event->id);
    if (IDC_EVENT_CONNECTED == event->id) {

    }else if(IDC_EVENT_DISCONNECTED == event->id){
        m_callback->onDisconnect();
    }else if (IDC_EVENT_MESSAGE == event->id) {
        char channelName[64] = {0};
        proxy->getName(channelName);
            UTILS_LOGI(TAG, " received idc message event, channelName:%s\n", channelName);
            IDCMessage *message = (IDCMessage*)event->param2;
            UTILS_LOGI(TAG, " message->m_msg.what:%d\n", message->m_msg.what);
            switch (message->m_msg.what) {
            case RECEIVE_PLAYBACK_STATE: {
                int state = message->getIntExtra(ARG1);
                int mediaId = message->getIntExtra(ARG2);
                UTILS_LOGI(TAG, " handleAsyncMessage RECEIVE_PLAYBACK_STATE state=%d ",state);
                if (state == MEDIA_STATE_PAUSED || state == MEDIA_STATE_STOPPED) {
                    if(mCurrentMediaId == mediaId){
                        m_callback->onMusicStateChanged(state);
                    }
                }else{
                    mCurrentMediaId = mediaId;
                    m_callback->onMusicStateChanged(MEDIA_STATE_PLAYING);
                }
            }
                break;
            case RECEIVE_MEDIA_METADATA: {
                std::string mediaName = message->getStringExtra(ARG2);
                //int mediaId = std::stoi(message->getStringExtra(ARG1));

                //mCurrentMediaId = mediaId;
                unsigned int dataLength = 0;
                int width,height,channels = 0;

                const unsigned char* data = message->getArrayExtra(ARG5, &dataLength);
                UTILS_LOGI(TAG, "handleAsyncMessage, RECEIVE_MEDIA_METADATA dataLength:%d", dataLength);
                m_callback->onMusicNameChanged(mediaName);
                m_callback->onCurrentAlbumPixmapChanged(data, dataLength);
            }
                break;
            case RECEIVE_ALBUM_IMAGE: {
                std::string mediaId = message->getStringExtra(ARG1);
                unsigned int dataLength = 0;
                const unsigned char* data = message->getArrayExtra(ARG2, &dataLength);
                UTILS_LOGI(TAG, "handleAsyncMessage, RECEIVE_ALBUM_IMAGE, mediaId:%s, dataLength:%d", mediaId.c_str(), dataLength);
                m_callback->onCurrentAlbumPixmapChanged(data, dataLength);
            }
                break;
            case RECEIVE_MEDIA_STATE: {

            }
                break;
            case RECEIVE_PHONE_NUM: {
                std::string number = message->getStringExtra(ARG1);
                UTILS_LOGI(TAG, "handleAsyncMessage,RECEIVE_PHONE_NUM  onCallNumberChanged number:%s", number.c_str());
                if (m_callback)
                    m_callback->onCallNumberChanged(number);
                }
                break;
            case RECEIVE_CONTACT_NAME: {
                std::string name = message->getStringExtra(ARG1);
                UTILS_LOGI(TAG, "handleAsyncMessage RECEIVE_CONTACT_NAME, ContactName:%s", name.c_str());
                if (m_callback)
                    m_callback->onCallPersonChanged(name);
                }
                break;
            case RECEIVE_PHONE_TIME: {
                std::string time = message->getStringExtra(ARG1);
                UTILS_LOGI(TAG, "handleAsyncMessage, CallTime:%s", time.c_str());
                if (m_callback)
                    m_callback->onCallTimeChanged(time);
                }
                break;
            case RECEIVE_PHONE_STATE: {
                callState = message->getIntExtra(ARG1);
                UTILS_LOGI(TAG, "handleAsyncMessage, CallStatus:%d", callState);
                if (m_callback){
                    m_callback->onCallStatusChanged(callState);
                }
                if(callState == Calling) {
                       if(!callRunning) {
                           callRunning = true;
                           std::thread callTimeThread([&](){
                               int callingHours = 0;
                               int callingMinutes = 0;
                               int callingSeconds = 0;
                               while(callState == Calling){
                                   UTILS_LOGI(TAG, "callState=%d",callState);
                                    callingSeconds++;
                                    if (callingSeconds == 60) {
                                        callingSeconds = 0;
                                        callingMinutes++;
                                        if (callingMinutes == 60) {
                                            callingMinutes = 0;
                                            callingHours = (callingHours + 1) % 24;
                                        }
                                    }
                                    std::ostringstream timeStringStream;
                                    if (callingHours == 0) {
                                        timeStringStream << std::setfill('0')<<std::setw(2) << callingMinutes<<":"<<std::setw(2)<<callingSeconds;
                                    } else {
                                        timeStringStream << std::setfill('0')<<std::setw(2)<<callingHours<<":"<< std::setw(2) << callingMinutes<<":"<<std::setw(2)<<callingSeconds;
                                    }
                                    UTILS_LOGI(TAG, "handleAsyncMessage, onCallTimeChanged:time:%s", timeStringStream.str().c_str());
                                    if (m_callback){
                                        m_callback->onCallTimeChanged(timeStringStream.str());
                                    }
                                    std::this_thread::sleep_for(std::chrono::seconds(1));
                                }
                            });
                        callTimeThread.detach();
                        }
                } else {
                        if(callRunning){
                            callRunning = false;
                        }
                    }
                }
                break;
            case RECEIVE_CONTACT_IMAGE: {
                unsigned int dataLength = 0;
                const unsigned char* data = message->getArrayExtra(ARG1, &dataLength);
                if (m_callback)
                    m_callback->onCallPixmapChanged(data, dataLength);
                }
                break;

            default:
                break;
            }
    }
}

int ClusterService::startUpdateService(std::string &lastVersion)
{
    /**
    UTILS_LOGI(TAG, "startUpdateService");
    if (m_updateClient == nullptr) {
        m_updateClient = ATCUpdateClient::getInstance();

        m_updateClient->registerMessageCb([this] (UpdateMessageType type, const std::string &msg) {
            if (m_state != (int)type) {
                UTILS_LOGD(TAG, "update message callback %d, %s", type, msg.c_str());
                m_callback->onUpdateStateChanged((int)type);
                m_state = (int)type;
            }
        });

        m_updateClient->registerProgressCb([this] (uint32_t progress) {
            if (m_updateProgress != progress) {
                UTILS_LOGD(TAG, "update progress callback %d", progress);
                m_callback->onUpdateProgressChanged(progress);
                m_updateProgress = progress;
            }
        });
    }

    m_updateClient->startService();
    int lastState =  m_updateClient->getLastStatus();
    if (lastState == LAST_UPDATE_OK) {
        lastVersion = m_updateClient->getSystemVersion();
    } else if (lastState == LAST_UNFINISHED) {
          char path[255] = {0};
          snprintf(path, 255, "%s/clustertemp%d", CommonConstant::FILE_PATH, CommonConstant::DownloadUpdatePackage);
          startUpdate(path);
    }

    return lastState;
    **/
    return 0;
}

void ClusterService::syncFile(const char* path)
{
    UTILS_LOGI(TAG, "syncFile enter");
    int fd = open(path, O_RDWR);
    if (fd > 0) {
        fsync(fd);
        close(fd);
    }

    DIR *dir = opendir(CommonConstant::FILE_PATH);
    if (dir) {
        fsync(dirfd(dir));
        closedir(dir);
    }
    UTILS_LOGI(TAG, "syncFile leave");
}


void ClusterService::startIVIProjection(int x, int y, int w, int h)
{
    UTILS_LOGI(TAG, "startIVIProjection x=%d y=%d w=%d h=%d", x, y, w, h);

    std::lock_guard<std::mutex> lock(m_mutex);

    m_iviProjectionPosition[0] = x;
    m_iviProjectionPosition[1] = y;
    m_iviProjectionPosition[2] = w;
    m_iviProjectionPosition[3] = h;
    m_showIVIProjection = true;

    if (m_iviProjectionThread == nullptr) {
        m_iviProjectionThread = new ThreadProc(std::bind(&ClusterService::showIVIProjection, this));
        m_iviProjectionThread->threadStart();
    }
    m_iviProjectionThread->triggerProc();
}

void ClusterService::stopIVIProjection()
{
    UTILS_LOGI(TAG, "stopIVIProjection");

    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_iviProjectionThread) {
        m_showIVIProjection = false;
        m_iviProjectionThread->triggerProc();
    }
}

void ClusterService::showIVIProjection()
{
    UTILS_LOGI(TAG, "showIVIProjection");

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_showIVIProjection) {
      //  if (m_iviProjectionDisplay == nullptr) {
          //  m_iviProjectionDisplay = AtcRvc::getInstance();
          //  m_iviProjectionDisplay->setCameraDevice(5);
          //  m_iviProjectionDisplay->setWindowZOrder(2);
           // m_iviProjectionDisplay->setWindow(m_iviProjectionPosition[0],  m_iviProjectionPosition[1],  m_iviProjectionPosition[2],  m_iviProjectionPosition[3]);
            //m_iviProjectionDisplay->startPreview();
       // } else {
           // m_iviProjectionDisplay->setWindow(m_iviProjectionPosition[0],  m_iviProjectionPosition[1],  m_iviProjectionPosition[2],  m_iviProjectionPosition[3]);
            //m_iviProjectionDisplay->showWindow();
     //   }
    } else {
       // if (m_iviProjectionDisplay)
           // m_iviProjectionDisplay->hideWindow();
    }
}

void ClusterService::startUpdate(const std::string &path)
{
    /**
    UTILS_LOGI(TAG, "startUpdate path: %s", path.c_str());
    if (m_updateClient == nullptr) {
        UTILS_LOGE(TAG, "m_updateClient not init");
        return;
    }

    std::string msg;
    m_updateProgress = -1;
    m_state = -1;
    bool ret = m_updateClient->beginUpdate(path, msg);
    UTILS_LOGD(TAG, "startUpdate path:%s, ret:%d, msg:%s", path.c_str(), ret, msg.c_str());
    **/
}

void ClusterService::checkUpdateState(int fd, const char* data, unsigned int length)
{
    /**
    char result = CommonConstant::CmdSucess;
    if (m_updateClient->checkUpdating()) {
        result = CommonConstant::CmdBusy;
    } else {
        //check dir space
        if (length >= sizeof(int)) {
            char path[255] = {0};
            snprintf(path, 255, "%s/clustertemp%d", CommonConstant::FILE_PATH, CommonConstant::DownloadUpdatePackage);
            remove(path);
            int size = 0;
            memcpy(&size, data, sizeof(int));
            struct statfs diskInfo;
            statfs(CommonConstant::FILE_PATH, &diskInfo);
            unsigned long long availableSpace = diskInfo.f_bavail * diskInfo.f_bsize;
            if (availableSpace < size) {
                UTILS_LOGE(TAG, "available space not enough, available space %llu Byte %.3f MB, file need size %d %.3f Byte", availableSpace, availableSpace / (1024 * 1024.0),
                           size, size / (1024 * 1024.0)); //KBs
                result = CommonConstant::CmdFailed;
            }
        }
    }

    m_manager->replyData(fd, CommonConstant::DownloadUpdatePackageCheck, result);
    **/
}

