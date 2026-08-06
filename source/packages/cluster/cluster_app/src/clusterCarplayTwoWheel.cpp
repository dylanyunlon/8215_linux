#include "clusterCarplayTwoWheel.h"
#include "clusterappconstant.h"
#include "clog.h"
#include <math.h>
#include <QDateTime>
#include <QTransform>
#include <QPixmap>
#include <QPainter>

#include <chrono>
#include "clog.h"

using cluster_utils::CLog;

const static char *TAG = "clusterCarplayTwoWheel";

clusterCarplayTwoWheel::clusterCarplayTwoWheel(const QRect &showRect)
    : ClusterAppPainter(showRect)
{
    if (checkFirstDraw()) {
        m_backgroundPixmap.load(":/images/zoomout/background1.png");
        m_statusBlutoothPixmap.load(":/images/zoomout/bluethooth.png");
        m_statusWifiPixmap.load(":/images/zoomout/wifi.png");
        m_statusGpsPixmap.load(":/images/zoomout/gps.png");
     // m_menuMusicPixmap.load(":/images/zoomout/menu_music.png");
     // m_menuGpsPixmap.load(":/images/zoomout/menu_gps.png");
     // m_menuDialerPixmap.load(":/images/zoomout/menu_dialer.png");
     // m_menuSettingsPixmap.load(":/images/zoomout/menu_set.png");
        m_bottomWaterTmp.load(":/images/zoomout/water_temp.png");
        m_bottomWaterTmp_ind.load(":/images/zoomout/water_temp_ind.png");
        m_fuelTank.load(":/images/zoomout/fuel_tank.png");
        m_fuelTank_ind.load(":/images/zoomout/fuel_tank_ind.png");
        m_highbeamPixmap.load(":/images/zoomout/highbeam.png");
        m_highbeamindPixmap.load(":/images/zoomout/highbeam_ind.png");
        m_lowbeamPixmap.load(":/images/zoomout/lowbeam.png");
        m_lowbeamindPixmap.load(":/images/zoomout/lowbeam_ind.png");
    }

}

void clusterCarplayTwoWheel::draw(QPaintDevice *device)
{
    m_painter.begin(device);
    m_painter.save();
    m_painter.drawPixmap(0, 360, m_backgroundPixmap);
    m_painter.restore();

    m_painter.setRenderHint(QPainter::SmoothPixmapTransform);
    m_painter.setRenderHint(QPainter::HighQualityAntialiasing);
    m_painter.setRenderHint(QPainter::TextAntialiasing);

    drawInfo();
    drawMenu();
    drawStatus();
    drawIcon();
    drawLight();
    drawSpeed();
    drawFuel();
    drawTmpProgress();
    drawFuelProgress();
    drawBigSpeedValue();

   // drawUpdateInfo();

    m_painter.end();
}

void clusterCarplayTwoWheel::drawTmpProgress()
{
    m_painter.save();
    m_painter.translate(76, 99 + 360);
    int segmentWith = 30;
    int segmentHight = 10;
    m_painter.setPen(QPen(Qt::white,6,Qt::SolidLine));
    m_painter.setBrush(Qt::white);
    int speed = m_value->getSpeed();
    if(speed <= 20) {
        QRect rect1(0, 0, speed*1.5, segmentHight);
        m_painter.fillRect(rect1, QColor("#80eee1"));
    } else if(speed > 20 && speed <= 40) {
        QRect rect1(0, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect1, QColor("#80eee1"));
        QRect rect2(segmentWith+3, 0, (speed - 20)*1.5, segmentHight);
        m_painter.fillRect(rect2, QColor("#80eee1"));
    } else if(speed > 40 && speed <= 60) {
        QRect rect1(0, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect1, QColor("#80eee1"));
        QRect rect2(segmentWith+3, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect2, QColor("#80eee1"));
        QRect rect3(segmentWith*2+6, 0,  (speed - 40)*1.5, segmentHight);
        m_painter.fillRect(rect3, QColor("#80eee1"));
    } else if(speed > 60 && speed <= 80) {
        QRect rect1(0, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect1, QColor("#80eee1"));
        QRect rect2(segmentWith+3, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect2, QColor("#80eee1"));
        QRect rect3(segmentWith*2+6, 0,  segmentWith, segmentHight);
        m_painter.fillRect(rect3, QColor("#80eee1"));
        QRect rect4(segmentWith*3+9, 0,  (speed - 60)*1.5, segmentHight);
        m_painter.fillRect(rect4, QColor("#80eee1"));
    } else if(speed > 80 && speed <= 100) {
        QRect rect1(0, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect1, QColor("#80eee1"));
        QRect rect2(segmentWith+3, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect2, QColor("#80eee1"));
        QRect rect3(segmentWith*2+6, 0,  segmentWith, segmentHight);
        m_painter.fillRect(rect3, QColor("#80eee1"));
        QRect rect4(segmentWith*3+9, 0,  segmentWith, segmentHight);
        m_painter.fillRect(rect4, QColor("#80eee1"));
        QRect rect5(segmentWith*4+12, 0, (speed - 80)*1.5, segmentHight);
        m_painter.fillRect(rect5, QColor("#80eee1"));
    } else if(speed > 100 && speed <= 120) {
        QRect rect1(0, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect1, Qt::red);
        QRect rect2(segmentWith+3, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect2, Qt::red);
        QRect rect3(segmentWith*2+6, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect3, Qt::red);
        QRect rect4(segmentWith*3+9, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect4, Qt::red);
        QRect rect5(segmentWith*4+12, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect5, Qt::red);
        QRect rect6(segmentWith*5+15, 0, (speed - 100)*1.5, segmentHight);
        m_painter.fillRect(rect6, Qt::red);
    }

    m_painter.restore();
  
}

void clusterCarplayTwoWheel::drawFuelProgress()
{
    m_painter.save();
    m_painter.translate(725, 99 + 360);
    int segmentWith = 30;
    int segmentHight = 10;
    m_painter.setPen(QPen(Qt::white,6,Qt::SolidLine));
    m_painter.setBrush(Qt::white);
    int fuel = m_value->getFuel();
    if(fuel <= 25) {
        QRect rect1(-(fuel*1.2), 0, fuel*1.2, segmentHight);
        m_painter.fillRect(rect1,  Qt::red);
    } else if(fuel > 25 && fuel <= 50) {
        QRect rect1(-segmentWith, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect1, QColor("#80eee1"));
        QRect rect2(-(segmentWith+(fuel-25)*1.2+3), 0, (fuel-25)*1.2, segmentHight);
        m_painter.fillRect(rect2, QColor("#80eee1"));
    } else if(fuel > 50 && fuel <= 75) {
        QRect rect1(-segmentWith, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect1, QColor("#80eee1"));
        QRect rect2(-(segmentWith*2 + 3), 0, segmentWith, segmentHight);
        m_painter.fillRect(rect2, QColor("#80eee1"));
        QRect rect3(-(segmentWith*2 + (fuel-50)*1.2 +6), 0, (fuel-50)*1.2, segmentHight);
        m_painter.fillRect(rect3, QColor("#80eee1"));
    } else if(fuel > 75 && fuel <= 100) {
        QRect rect1(-segmentWith, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect1, QColor("#80eee1"));
        QRect rect2(-(segmentWith*2 + 3), 0, segmentWith, segmentHight);
        m_painter.fillRect(rect2, QColor("#80eee1"));
        QRect rect3(-(segmentWith*3 +6), 0, segmentWith, segmentHight);
        m_painter.fillRect(rect3, QColor("#80eee1"));
        QRect rect4(-(segmentWith*3 +(fuel - 75)*1.2 +9), 0,  (fuel - 75)*1.2, segmentHight);
        m_painter.fillRect(rect4, QColor("#80eee1"));
    } else if(fuel > 100 && fuel <= 125) {
        QRect rect1(-segmentWith, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect1, QColor("#80eee1"));
        QRect rect2(-(segmentWith*2 + 3), 0, segmentWith, segmentHight);
        m_painter.fillRect(rect2, QColor("#80eee1"));
        QRect rect3(-(segmentWith*3 +6), 0, segmentWith, segmentHight);
        m_painter.fillRect(rect3, QColor("#80eee1"));
        QRect rect4(-(segmentWith*4 +9), 0, segmentWith, segmentHight);
        m_painter.fillRect(rect4, QColor("#80eee1"));
        QRect rect5(-(segmentWith*4 +12 + (fuel - 100)*1.2), 0,  (fuel - 100)*1.2, segmentHight);
        m_painter.fillRect(rect5, QColor("#80eee1"));
    } else if(fuel > 125 && fuel <= 150) {
        QRect rect1(-segmentWith, 0, segmentWith, segmentHight);
        m_painter.fillRect(rect1, QColor("#80eee1"));
        QRect rect2(-(segmentWith*2 + 3), 0, segmentWith, segmentHight);
        m_painter.fillRect(rect2, QColor("#80eee1"));
        QRect rect3(-(segmentWith*3 +6), 0, segmentWith, segmentHight);
        m_painter.fillRect(rect3, QColor("#80eee1"));
        QRect rect4(-(segmentWith*4 +9), 0, segmentWith, segmentHight);
        m_painter.fillRect(rect4,QColor("#80eee1"));
        QRect rect5(-(segmentWith*5 +12), 0, segmentWith, segmentHight);
        m_painter.fillRect(rect5,QColor("#80eee1"));
        QRect rect6(-(segmentWith*5 +15 + (fuel - 125)*1.2), 0, (fuel - 125)*1.2, segmentHight);
        m_painter.fillRect(rect6, QColor("#80eee1"));
    }

    m_painter.restore();
  
}

void clusterCarplayTwoWheel::drawBackground(QPaintDevice * device)
{
    m_fristBackgroundPixmap.load(":/images/zoomout/background.png");
    m_painter.begin(device);
    m_painter.drawPixmap(0, 360, m_fristBackgroundPixmap);
    m_painter.end();
}

void clusterCarplayTwoWheel::drawStatus()
{
    m_painter.save();
    m_painter.translate(622, 7 + 360);
    m_painter.drawPixmap(QPoint(0, 0), m_statusBlutoothPixmap);

    m_painter.translate(44, 0);
    m_painter.drawPixmap(QPoint(0, 0), m_statusGpsPixmap);

    m_painter.translate(44, 0);
    m_painter.drawPixmap(QPoint(0, 0), m_statusWifiPixmap);

    m_painter.restore();
  
}

void clusterCarplayTwoWheel::drawIcon()
{
    m_painter.save();
    m_painter.translate(60, 7 + 360);
    int span = 14;
    int width = 30;

    for(int i = 1; i < SMALLICONNAME_List.size() -1; i++) {
        QPixmap *pix = m_value->getIndicationIcon(SMALLICONNAME_List[i]);
      //  UTILS_LOGI(TAG, "clusterCarplayTwoWheel pix %p",pix );
        if (pix) {
            m_painter.drawPixmap(QPoint(0, 0), *pix);
        }
        m_painter.translate(span + width, 0);
    }
    m_painter.restore();

    m_painter.save();
    m_painter.translate(291, 4 + 360);
    QPixmap *leftLightPix = m_value->getIndicationIcon(SMALLICONNAME_List.first());
    //UTILS_LOGI(TAG, "clusterCarplayTwoWheel leftLightPix %p",leftLightPix );
    if (leftLightPix) {
           m_painter.drawPixmap(QPoint(0, 0), *leftLightPix);
    }

    m_painter.translate(180, 0);
    QPixmap *rightLightPix = m_value->getIndicationIcon(SMALLICONNAME_List.last());
    if (rightLightPix) {
           m_painter.drawPixmap(QPoint(0, 0), *rightLightPix);
    }
    m_painter.restore();
}

void clusterCarplayTwoWheel::drawLight()
{
    m_painter.save();

    //draw light
    m_painter.translate(534, 7 + 360);
    int span = 14;
    int width = 30;
    for(int i = 0; i < SMALLLIGHT_List.size(); i++) {
        QPixmap *pix = m_value->getLightIcon(SMALLLIGHT_List[i]);
        if (pix) {
            m_painter.drawPixmap(QPoint(0, 0), *pix);
        }
        m_painter.translate(span + width, 0);
    }

    m_painter.restore();
}

void clusterCarplayTwoWheel::drawSpeed()
{
    m_painter.save();
    m_painter.translate(138, 62 + 360);

    drawSpeedValue();
    /**
    m_painter.translate(18, 0);
    font.setPixelSize(20);
    m_painter.setFont(font);
    m_painter.setPen(QPen(QColor("#80eee1")));
    m_painter.drawText(0, 0, "km/h");
    **/
    m_painter.restore();

}

void clusterCarplayTwoWheel::drawFuel()
{
    m_painter.save();
    m_painter.translate(608, 62 + 360);

    drawFuelValue();
    /**
    m_painter.translate(18, 0);
    font.setPixelSize(20);
    m_painter.setFont(font);
    m_painter.setPen(QPen(QColor("#80eee1")));
    m_painter.drawText(0, 0, "ECO");
    **/
    m_painter.restore();

}

void clusterCarplayTwoWheel::drawBigSpeedValue()
{
    int speed = m_value->getSpeed();
    double scaleFactor = 2.0;
    m_painter.save();
    m_painter.translate(400, 60 + 360);

    if (speed < 10) {
        QPixmap *pix = m_pixmapManager->getBigNumberPixmap(speed);
       // QPixmap scaledPix1 = pix->scaled(pix->width() * scaleFactor, pix->height() * scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_painter.drawPixmap(-pix->width() / 2, -pix->height() / 2, *pix);
    } else if (speed < 100) {
        QPixmap *pix1 = m_pixmapManager->getBigNumberPixmap(speed / 10);
        QPixmap *pix2 = m_pixmapManager->getBigNumberPixmap(speed % 10);
        m_painter.drawPixmap(-(pix1->width() + pix2->width()) / 2, -pix1->height() / 2, *pix1);
        m_painter.drawPixmap((pix1->width() - pix2->width()) / 2,  -pix2->height() / 2, *pix2);
    } else {
        QPixmap *pix1 = m_pixmapManager->getBigNumberPixmap(speed / 100);
        QPixmap *pix2 = m_pixmapManager->getBigNumberPixmap(speed / 10 % 10);
        QPixmap *pix3 = m_pixmapManager->getBigNumberPixmap(speed % 10);

        m_painter.drawPixmap(-(pix1->width() + pix2->width() + pix3->width()) / 2, -pix1->height() / 2, *pix1);
        m_painter.drawPixmap((pix1->width() - pix2->width() - pix3->width()) / 2, -pix2->height() / 2, *pix2);
        m_painter.drawPixmap((pix1->width() + pix2->width() - pix3->width()) / 2 , -pix3->height() / 2, *pix3);
    }
    m_painter.restore();
}

void clusterCarplayTwoWheel::drawSpeedValue()
{
    int rpm = m_value->getRpm();
    if (rpm < 10) {
        QPixmap *pix = m_pixmapManager->getNumberPixmap(rpm);
        m_painter.drawPixmap(-pix->width() / 2, -pix->height() / 2, *pix);
    } else if (rpm < 100) {
        QPixmap *pix1 = m_pixmapManager->getNumberPixmap(rpm / 10);
        QPixmap *pix2 = m_pixmapManager->getNumberPixmap(rpm % 10);
        m_painter.drawPixmap(-(pix1->width() + pix2->width()) / 2, -pix1->height() / 2, *pix1);
        m_painter.drawPixmap((pix1->width() - pix2->width()) / 2,  -pix2->height() / 2, *pix2);
    } else {
        QPixmap *pix1 = m_pixmapManager->getNumberPixmap(rpm / 100);
        QPixmap *pix2 = m_pixmapManager->getNumberPixmap(rpm / 10 % 10);
        QPixmap *pix3 = m_pixmapManager->getNumberPixmap(rpm % 10);

        m_painter.drawPixmap(-(pix1->width() + pix2->width() + pix3->width()) / 2, -pix1->height() / 2, *pix1);
        m_painter.drawPixmap((pix1->width() - pix2->width() - pix3->width()) / 2, -pix2->height() / 2, *pix2);
        m_painter.drawPixmap((pix1->width() + pix2->width() - pix3->width()) / 2 , -pix3->height() / 2, *pix3);
    }
}

void clusterCarplayTwoWheel::drawFuelValue()
{
    int gear = m_value->getGear();
    if (gear < 10) {
        QPixmap *pix = m_pixmapManager->getNumberPixmap(gear);
        m_painter.drawPixmap(-pix->width() / 2, -pix->height() / 2, *pix);
    } else if (gear < 100) {
        QPixmap *pix1 = m_pixmapManager->getNumberPixmap(gear / 10);
        QPixmap *pix2 = m_pixmapManager->getNumberPixmap(gear % 10);
        m_painter.drawPixmap(-(pix1->width() + pix2->width()) / 2, -pix1->height() / 2, *pix1);
        m_painter.drawPixmap((pix1->width() - pix2->width()) / 2,  -pix2->height() / 2, *pix2);
    } else {
        QPixmap *pix1 = m_pixmapManager->getNumberPixmap(gear / 100);
        QPixmap *pix2 = m_pixmapManager->getNumberPixmap(gear / 10 % 10);
        QPixmap *pix3 = m_pixmapManager->getNumberPixmap(gear % 10);

        m_painter.drawPixmap(-(pix1->width() + pix2->width() + pix3->width()) / 2, -pix1->height() / 2, *pix1);
        m_painter.drawPixmap((pix1->width() - pix2->width() - pix3->width()) / 2, -pix2->height() / 2, *pix2);
        m_painter.drawPixmap((pix1->width() + pix2->width() - pix3->width()) / 2 , -pix3->height() / 2, *pix3);
    }
}

void clusterCarplayTwoWheel::drawMenu()
{
    m_painter.save();
/**
    m_painter.translate(5, 15 + 360);
    m_painter.drawPixmap(QPoint(0, 0), m_menuMusicPixmap);
    m_painter.translate(759, 0);
    m_painter.drawPixmap(QPoint(0, 0), m_menuSettingsPixmap);
    m_painter.translate(-759, 57);
    m_painter.drawPixmap(QPoint(0, 0), m_menuGpsPixmap);
    m_painter.translate(759, 0);
    m_painter.drawPixmap(QPoint(0, 0), m_menuDialerPixmap);
**/
    int fuel = m_value->getFuel();
    int speed = m_value->getSpeed();
    if (speed > 100 && speed <= 120) {
        m_painter.translate(8, 69 + 360);
        m_painter.drawPixmap(QPoint(0, 0), m_bottomWaterTmp_ind);
    }else {
        m_painter.translate(8, 69 + 360);
        m_painter.drawPixmap(QPoint(0, 0), m_bottomWaterTmp);
    }
    m_painter.restore();
    m_painter.save();
    if(fuel <= 25){
        m_painter.translate(747, 69 + 360);
        m_painter.drawPixmap(QPoint(0, 0), m_fuelTank_ind);
    }else {
        m_painter.translate(747, 69 + 360);
        m_painter.drawPixmap(QPoint(0, 0), m_fuelTank);
    }
    m_painter.restore();
}





