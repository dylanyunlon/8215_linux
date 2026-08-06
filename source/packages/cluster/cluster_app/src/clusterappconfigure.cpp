#include "clusterappconfigure.h"
#include "clog.h"
#include <fcntl.h>
#include <unistd.h>
//using universal_utils::CLog;
const static char *TAG = "ClusterAppConfigure";
ClusterAppConfigure *ClusterAppConfigure::s_instance = new ClusterAppConfigure();
ClusterAppConfigure::ClusterAppConfigure()
    : m_setting("/data/cluster/cluster-app.cfg", QSettings::IniFormat)

{
    testConfigure();
}

ClusterAppConfigure *ClusterAppConfigure::getInstance()
{
    return s_instance;
}

void ClusterAppConfigure::setVolume(int volume)
{
    m_volume = volume;
    m_setting.setValue(KEY_VOLUME, volume);
    syncFile();
}


bool ClusterAppConfigure::isShowPerformance() const
{
    return m_isShowPerformance;
}

bool ClusterAppConfigure::isPlaySound() const
{
    return m_isPlaySound;
}

bool ClusterAppConfigure::isShowIVIProjection() const
{
    return m_iviProjection;
}

bool ClusterAppConfigure::isOTTest() const
{
    return m_isOTTest;
}

int ClusterAppConfigure::getVolume() const
{
    return m_volume;
}

int ClusterAppConfigure::getTransitionFps() const
{
    return m_transitionFps;
}

void ClusterAppConfigure::testConfigure()
{
    m_isShowPerformance = m_setting.value(KEY_SHOWPERFORMANCE, true).toBool();
    m_isPlaySound = m_setting.value(KEY_PLAYSOUND, true).toBool();
    m_volume = m_setting.value(KEY_VOLUME, 10).toInt();
    m_iviProjection = m_setting.value(KEY_SHOW_IVI_PROJECTION, true).toBool();
    m_isOTTest = m_setting.value(KEY_OT_TEST, false).toBool();
    m_transitionFps = m_setting.value(KEY_TRANSITION_FPS, 50).toInt();
/**
    UTILS_LOGD(TAG, "showPerformace %d", m_isShowPerformance);
    UTILS_LOGD(TAG, "isPlaySound %d", m_isPlaySound);
    UTILS_LOGD(TAG, "volume %d", m_volume);
    UTILS_LOGD(TAG, "iviProjection %d", m_iviProjection);
    UTILS_LOGD(TAG, "isOTTest %d", m_isOTTest);
    UTILS_LOGD(TAG, "transitionFps %d", KEY_TRANSITION_FPS);
    **/
}
void ClusterAppConfigure::syncFile()
{
 //   UTILS_LOGI(TAG, "syncFile enter");
    int fd = open("/data/cluster/cluster-app.cfg", O_RDWR);
    if (fd > 0) {
        fsync(fd);
        close(fd);
    }
 //   UTILS_LOGI(TAG, "syncFile leave");
}
