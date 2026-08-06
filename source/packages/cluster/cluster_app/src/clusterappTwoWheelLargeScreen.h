#ifndef CLUSTERAPPTWOWHEELARGESCREENL_H
#define CLUSTERAPPTWOWHEELARGESCREENL_H
#include <QPaintDevice>
#include <QPixmap>
#include <QPainter>
#include "clusterappvalue.h"
#include "clusterappiviinfo.h"
#include "clusterapppainter.h"
#include <QVector>
#include <QRect>
#include <array>

class clusterappTwoWheelLargeScreen : public ClusterAppPainter
{
public:

    clusterappTwoWheelLargeScreen(const QRect &showRect);
    void draw(QPaintDevice *device);
    void drawBackground(QPaintDevice * device);
    void drawIcon();
    void drawStatus();
    void drawTopTip();
    void drawRpm();
    void drawSpeed();
    void drawSpeedValue();
    void drawRpmValue();
    void drawBottomInfo();
    void drawNavigationIcons();
    void drawLight();
    QRect m_gpsRect;
    QRect m_dialerRect;
    QRect m_settingsRect;
    QRect m_dvrRect;
private:
    QPixmap m_statusBlutoothPixmap, m_statusWifiPixmap, m_statusGpsPixmap, m_statusMusicPixmap;
    QPixmap m_bottomDVRPixmap, m_bottomGpsPixmap, m_bottomDialerPixmap, m_bottomSettingsPixmap;
    QPixmap m_blueBottomPixmap;
    QPixmap m_highbeamPixmap, m_highbeamindPixmap, m_lowbeamPixmap, m_lowbeamindPixmap;
    QPixmap m_pointerPixmap1;
    QPixmap m_fristBackgroundPixmap;
    QTransform mtransform;
    void drawFocusEffect(const QRect& rect);

    QRect rectMusic;   // 音乐
    QRect rectNavi;  // 导航
    QRect rectPhone;  // 电话
    QRect rectSetting;  // 设置

};

#endif // CLUSTERAPPPAINTERCOMFORT_H
