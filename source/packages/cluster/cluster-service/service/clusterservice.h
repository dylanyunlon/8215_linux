#ifndef CLUSTERSERVICE_H
#define CLUSTERSERVICE__H
#include "icluster.h"
#include "callback.h"
#include "cprocess.h"
//#include "ATCUpdateClient.hpp"
#include <mutex>
#include "clusterthreadproc.h"
#include <idc_server.h>
#include <idc_info.h>
#include <chrono>
#include <thread>
#include <clog.h>


class ClusterManager;
class ClusterService : public ICluster, public DataReceiveCallback, 
public IIDCMonitorListener, public IIDCProxyListener
{
public:
    static ICluster *getInstance();
    void registerCallBack(IClusterCallBack *callback) override;
    void mediaPlayPause() override;
    void mediaPre() override;
    void mediaNext() override;

    void call() override;
    void handup() override;
    void onReceiveData(int fd, unsigned char cmd, const char* data, unsigned int length);
    void onReceiveReplyTimeOut(int fd, unsigned char cmd);

    void startService() override;
    int startUpdateService(std::string &lastVersion) override;

    void startIVIProjection(int x, int y, int w, int h) override;
    void stopIVIProjection() override;
    enum
    {
        RECEIVE_SONG_NAME = 0x01,
        RECEIVE_PLAY_STATE,
        RECEIVE_MEDIA_STATE,
        RECEIVE_PHONE_NUM,
        RECEIVE_CONTACT_NAME,
        RECEIVE_PHONE_TIME,
        RECEIVE_PHONE_STATE,
        RECEIVE_CONTACT_IMAGE,
        RECEIVE_ALBUM_IMAGE,
        RECEIVE_PLAYBACK_STATE,
        RECEIVE_MEDIA_METADATA,
        RECEIVE_ROUTE_GUIDANCE_STATE,
        RECEIVE_ROUTE_GUIDANCE_INFORMATION,
    };

    enum
    {
        MEDIA_STATE_STOPPED= 0x00,
        MEDIA_STATE_PLAYING,
        MEDIA_STATE_PAUSED,
        MEDIA_STATE_NEXT_PLAYING,
        MEDIA_STATE_PREVIOUS_PLAYING,
    };

    enum CallStatus {
        Incoming = 1,     //来电
        Dialing,          //拨打中
        Calling,          //通话中
        Idle,             //空闲,挂断
    };

    enum
    {
        MEDIA_ACTION_NONE = 0x00,
        MEDIA_ACTION_NEXT,
        MEDIA_ACTION_PREVIOUS,
    };

private:
    ClusterService();
    ClusterService(const ClusterService&) = delete;
    ClusterService &operator=(const ClusterService&) = delete;
    void onEvent(const char *domain, const char *channel, idc_event_t *event);
    void onEvent(IInterdomainChannelProxy *proxy, idc_event_t *event, void *reply);

    void startUpdate(const std::string &path);
    void checkUpdateState(int fd, const char* data, unsigned int length);
    void syncFile(const char* path);
    void showIVIProjection();

    IClusterCallBack *m_callback = nullptr;
    ClusterManager *m_manager;
  //  ATCUpdateClient::ptr m_updateClient = nullptr;
    int m_updateProgress = 0;
    int m_state = -1;
    int m_iviProjectionPosition[4];
 //   AtcRvc *m_iviProjectionDisplay = nullptr;
    ThreadProc *m_iviProjectionThread = nullptr;
    std::mutex m_mutex;
    bool m_showIVIProjection = false;
    IInterdomainChannelProxy* pProjectionChannel = nullptr;
    IInterdomainChannelProxy* pIviTransportChannel = nullptr;
    IInterdomainChannelProxy* pAdasTransportChannel = nullptr;
    int mCurrentMediaId;
    int callState;
    unsigned char *contactRgbData = nullptr;
    unsigned char *ablumRgbData = nullptr;
    std::atomic<bool> callRunning{false};
};

#endif // CLUSTERSERVICE_H
