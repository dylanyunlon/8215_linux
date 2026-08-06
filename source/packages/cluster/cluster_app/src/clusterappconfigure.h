#ifndef CLUSTERAPPCONFIGURE_H
#define CLUSTERAPPCONFIGURE_H
#include <QSettings>

class ClusterAppConfigure
{
public:
    static ClusterAppConfigure *getInstance();
    void setVolume(int volume);

    bool isShowPerformance() const;
    bool isPlaySound() const;
    bool isShowIVIProjection() const;
    bool isOTTest() const;
    int getVolume() const;
    int getTransitionFps() const;


private:
    ClusterAppConfigure();
    ClusterAppConfigure(const ClusterAppConfigure &) = delete;
    const ClusterAppConfigure &operator=(const ClusterAppConfigure &) = delete;

    void testConfigure();
    void syncFile();

    QSettings m_setting;
    const char* KEY_SHOWPERFORMANCE = "showPerformance";
    const char* KEY_SHOW_IVI_PROJECTION = "iviprojection";
    const char* KEY_PLAYSOUND = "playSound";
    const char* KEY_VOLUME = "volume";
    const char* KEY_TRANSITION_FPS = "transitionFps";
    const char* KEY_OT_TEST = "OT_Test";
    const char* TRUE_FALG = "true";

    bool m_isShowPerformance = true;
    bool m_isPlaySound = false;
    bool m_iviProjection = true;
    bool m_isOTTest = false;
    int m_volume = 10;
    int m_transitionFps = 50;

    static ClusterAppConfigure * s_instance;
};

#endif // CLUSTERAPPCONFIGURE_H
