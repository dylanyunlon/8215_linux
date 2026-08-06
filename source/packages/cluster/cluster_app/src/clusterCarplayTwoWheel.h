#ifndef CLUSTERCARPLAYTWOWHEEL_H
#define CLUSTERCARPLAYTWOWHEEL_H
#include <QPaintDevice>
#include <QPixmap>
#include <QPainter>
#include "clusterappvalue.h"
#include "clusterappiviinfo.h"
#include "clusterapppainter.h"
#include <QVector>
class clusterCarplayTwoWheel : public ClusterAppPainter
{
public:
    clusterCarplayTwoWheel(const QRect &showRect);
    void draw(QPaintDevice *device);
    void drawBackground(QPaintDevice * device);
    void drawIcon();
    void drawStatus();
    void drawGear();
    void drawSpeedValue();
    void drawBigSpeedValue();
    void drawSpeed();
    void drawTmpProgress();
    void drawFuelValue();
    void drawFuel();
    void drawFuelProgress();
    void drawLight();
    void drawMenu();
private:
    QPixmap m_statusBlutoothPixmap, m_statusWifiPixmap, m_statusGpsPixmap;
  //  QPixmap m_menuMusicPixmap, m_menuGpsPixmap, m_menuDialerPixmap, m_menuSettingsPixmap;
    QPixmap m_highbeamPixmap, m_highbeamindPixmap, m_lowbeamPixmap, m_lowbeamindPixmap;
    QPixmap m_fristBackgroundPixmap;
    QTransform mtransform;
    QPixmap m_bottomWaterTmp, m_bottomWaterTmp_ind, m_fuelTank, m_fuelTank_ind;
};

#endif // CLUSTERCARPLAYTWOWHEEL_H