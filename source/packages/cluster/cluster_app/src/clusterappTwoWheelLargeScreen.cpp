#include "clusterappTwoWheelLargeScreen.h"
#include "clusterappconstant.h"
#include "clog.h"
#include <math.h>
#include <QDateTime>
#include <QTransform>

#include <chrono>
#include <unistd.h>
#include <fcntl.h>
#include "clog.h"
#include <QThread>
#include <QObject>

using cluster_utils::CLog;
const static char *TAG = "clusterappTwoWheelLargeScreen";


clusterappTwoWheelLargeScreen::clusterappTwoWheelLargeScreen(const QRect &showRect)
    : ClusterAppPainter(showRect)
{



}

void clusterappTwoWheelLargeScreen::draw(QPaintDevice *device)
{
    m_painter.begin(device);
    m_painter.save();
    m_painter.drawPixmap(0, 0, m_backgroundPixmap);
    m_painter.restore();

    m_painter.setRenderHint(QPainter::SmoothPixmapTransform);
    m_painter.setRenderHint(QPainter::HighQualityAntialiasing);
    m_painter.setRenderHint(QPainter::TextAntialiasing);

    drawInfo();
    drawStatus();

    drawTopTip();
    drawIcon();
    drawLight();
    drawCarLine(313, 434, 477, 463);
    drawSpeed();
    drawRpm();
    drawBottomInfo();
    drawIVIInfo(446, 217);
   // drawUpdateInfo();

    m_painter.end();
}

void clusterappTwoWheelLargeScreen::drawBackground(QPaintDevice * device)
{

    QThread *thread = new QThread();
    QObject::connect(thread, &QThread::started, [=](){
       if (checkFirstDraw()) {
           m_backgroundPixmap.load(":/images_1024x600/twowheel/background1.png");
           m_pointerPixmap.load(":/images_1024x600/twowheel/pointer.png");
           m_pointerPixmap1.load(":/images_1024x600/twowheel/pointer1.png");
           m_pointerShadePixmap.load(":/images_1024x600/twowheel/pointershade.png");
           m_pointerShadePixmap2.load(":/images_1024x600/twowheel/pointershade2.png");
           m_statusBlutoothPixmap.load(":/images_1024x600/twowheel/bluethooth.png");
           m_statusWifiPixmap.load(":/images_1024x600/twowheel/wifi.png");
           m_statusGpsPixmap.load(":/images_1024x600/twowheel/gps.png");
           m_statusMusicPixmap.load(":/images_1024x600/twowheel/music.png");
           m_bottomGpsPixmap.load(":/images_1024x600/twowheel/bottom_gps.png");
           m_bottomDialerPixmap.load(":/images_1024x600/twowheel/bottom_dialer.png");
           m_bottomSettingsPixmap.load(":/images_1024x600/twowheel/bottom_settings.png");
           m_bottomDVRPixmap.load(":/images_1024x600/twowheel/bottom_dvr.png");
           m_highbeamPixmap.load(":/images_1024x600/twowheel/highbeam.png");
           m_highbeamindPixmap.load(":/images_1024x600/twowheel/highbeam_ind.png");
           m_lowbeamPixmap.load(":/images_1024x600/twowheel/lowbeam.png");
           m_lowbeamindPixmap.load(":/images_1024x600/twowheel/lowbeam_ind.png");
           m_blueBottomPixmap.load(":/images_1024x600/twowheel/blueBottom.png");

       }
       thread->quit();
     });
    thread->start();
    draw(device);
}

void clusterappTwoWheelLargeScreen::drawStatus()
{
    m_painter.save();
    m_painter.translate(36, 19);
    
    if (!m_statusBlutoothPixmap.isNull())
        m_painter.drawPixmap(QPoint(0, 0), m_statusBlutoothPixmap);

    m_painter.translate(85, 0);
    
    if (!m_statusWifiPixmap.isNull())
        m_painter.drawPixmap(QPoint(0, 0), m_statusWifiPixmap);

    m_painter.translate(745, 0);
    
    if (!m_statusGpsPixmap.isNull())
        m_painter.drawPixmap(QPoint(0, 0), m_statusGpsPixmap);

    m_painter.translate(85, 0);
    
    if (!m_statusMusicPixmap.isNull())
        m_painter.drawPixmap(QPoint(0, 0), m_statusMusicPixmap);

    m_painter.restore();
}

void clusterappTwoWheelLargeScreen::drawTopTip()
{
    m_painter.save();
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString formattedTime = currentDateTime.toString("HH:mm");
    QDate currentDate = currentDateTime.date();
    QStringList weekDays = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
    int dayOfWeek = currentDate.dayOfWeek(); // 1 = Monday, 7 = Sunday
    QString formatDayOfWeek = weekDays[dayOfWeek % 7];
    m_painter.translate(359, 108);

    font.setPixelSize(20);
    m_painter.setFont(font);
    m_painter.setPen(QPen(QColor("#80eee1")));
    m_painter.drawText(0, 0, formatDayOfWeek);

    font.setPixelSize(24);
    m_painter.setFont(font);

    m_painter.translate(116, 0);
    m_painter.drawText(0, 0, formattedTime);

    font.setPixelSize(20);
    m_painter.setFont(font);
    m_painter.translate(151, 0);
    m_painter.drawText(0, 0, QString("%1°C").arg(m_value->getAirTemp()));

    m_painter.restore();
}

void clusterappTwoWheelLargeScreen::drawIcon()
{
    m_painter.save();

    m_painter.translate(242, 15);
    int span = 20;
    int width = 60;
    for (int i = 0; i < ICONNAME_List.size(); i++) {
        QPixmap *pix = m_value->getIndicationIcon(ICONNAME_List[i]);
        if (pix && !pix->isNull())
            m_painter.drawPixmap(QPoint(0, 0), *pix);

        m_painter.translate(span + width, 0);
    }

    m_painter.restore();
}

void clusterappTwoWheelLargeScreen::drawSpeed()
{
    m_painter.save();

    //draw speed
    m_painter.translate(806, 312);

    drawSpeedValue();

    m_painter.rotate(-155 +m_value->getSpeed() * 2.32);

    if (!m_pointerShadePixmap.isNull())
         m_painter.drawPixmap(-140, -140, m_pointerShadePixmap);

    m_painter.rotate(3);

    if (!m_pointerPixmap.isNull())
         m_painter.drawPixmap(-149.5, -202.5, m_pointerPixmap);

    m_painter.restore();
}

void clusterappTwoWheelLargeScreen::drawLight()
{
    m_painter.save();

    //draw light
    m_painter.translate(428, 140);
    int span = 48;
    int width = 60;
    for (int i = 0; i < LIGHT_List.size(); i++) {
        QPixmap *pix = m_value->getLightIcon(LIGHT_List[i]);
        if (!pix->isNull())
            m_painter.drawPixmap(QPoint(0, 0), *pix);

        m_painter.translate(span + width, 0);
    }

    m_painter.restore();
}

void clusterappTwoWheelLargeScreen::drawRpm()
{
    m_painter.save();
    //draw rpm
    m_painter.translate(217, 312);
    drawRpmValue();
    m_painter.rotate(-138 +m_value->getRpm() * 33.75);

    if (!m_pointerShadePixmap.isNull())
        m_painter.drawPixmap(-140, -140, m_pointerShadePixmap);


    m_painter.rotate(3);

    if (!m_pointerPixmap.isNull())
        m_painter.drawPixmap(-149.5, -202.5, m_pointerPixmap);

    m_painter.restore();
}

void clusterappTwoWheelLargeScreen::drawSpeedValue()
{
    if(m_pixmapManager == nullptr) {
        return;
    }
    int speed = m_value->getSpeed();
    if (speed < 10) {
        QPixmap *pix = m_pixmapManager->getNumberPixmap(speed);
        if (pix && !pix->isNull())
            m_painter.drawPixmap(-pix->width() / 2, -pix->height() / 2, *pix);
    } else if (speed < 100) {
        QPixmap *pix1 = m_pixmapManager->getNumberPixmap(speed / 10);
        QPixmap *pix2 = m_pixmapManager->getNumberPixmap(speed % 10);
        if (pix1 && !pix1->isNull() && pix2 && !pix2->isNull()) {
            m_painter.drawPixmap(-(pix1->width() + pix2->width()) / 2, -pix1->height() / 2, *pix1);
            m_painter.drawPixmap((pix1->width() - pix2->width()) / 2, -pix2->height() / 2, *pix2);
        }
    } else {
        QPixmap *pix1 = m_pixmapManager->getNumberPixmap(speed / 100);
        QPixmap *pix2 = m_pixmapManager->getNumberPixmap(speed / 10 % 10);
        QPixmap *pix3 = m_pixmapManager->getNumberPixmap(speed % 10);
        if (pix1 && !pix1->isNull() && pix2 && !pix2->isNull() && pix3 && !pix3->isNull()) {
            m_painter.drawPixmap(-(pix1->width() + pix2->width() + pix3->width()) / 2, -pix1->height() / 2, *pix1);
            m_painter.drawPixmap((pix1->width() - pix2->width() - pix3->width()) / 2, -pix2->height() / 2, *pix2);
            m_painter.drawPixmap((pix1->width() + pix2->width() - pix3->width()) / 2, -pix3->height() / 2, *pix3);
        }
    }
}

void clusterappTwoWheelLargeScreen::drawRpmValue()
{
    if(m_pixmapManager == nullptr) {
        return;
    }
    int rpm = m_value->getRpm();
    if (rpm < 10) {
        QPixmap *pix = m_pixmapManager->getNumberPixmap(rpm);
        if (pix && !pix->isNull())
            m_painter.drawPixmap(-pix->width() / 2, -pix->height() / 2, *pix);
    } else if (rpm < 100) {
        QPixmap *pix1 = m_pixmapManager->getNumberPixmap(rpm / 10);
        QPixmap *pix2 = m_pixmapManager->getNumberPixmap(rpm % 10);
        if (pix1 && !pix1->isNull() && pix2 && !pix2->isNull()) {
            m_painter.drawPixmap(-(pix1->width() + pix2->width()) / 2, -pix1->height() / 2, *pix1);
            m_painter.drawPixmap((pix1->width() - pix2->width()) / 2, -pix2->height() / 2, *pix2);
        }
    } else {
        QPixmap *pix1 = m_pixmapManager->getNumberPixmap(rpm / 100);
        QPixmap *pix2 = m_pixmapManager->getNumberPixmap(rpm / 10 % 10);
        QPixmap *pix3 = m_pixmapManager->getNumberPixmap(rpm % 10);
        if (pix1 && !pix1->isNull() && pix2 && !pix2->isNull() && pix3 && !pix3->isNull()) {
            m_painter.drawPixmap(-(pix1->width() + pix2->width() + pix3->width()) / 2, -pix1->height() / 2, *pix1);
            m_painter.drawPixmap((pix1->width() - pix2->width() - pix3->width()) / 2, -pix2->height() / 2, *pix2);
            m_painter.drawPixmap((pix1->width() + pix2->width() - pix3->width()) / 2 , -pix3->height() / 2, *pix3);
        }
    }
}

void clusterappTwoWheelLargeScreen::drawBottomInfo()
{
    m_painter.save();

    QPoint basePos(20, 496);
    QPoint currentPos = basePos;

    // ================= DVR =================
    if (!m_bottomDVRPixmap.isNull()) {
        m_dvrRect = QRect(currentPos, m_bottomDVRPixmap.size());
        m_painter.drawPixmap(currentPos, m_bottomDVRPixmap);

        if (m_focus == FOCUS_DVR) {
            drawFocusEffect(m_dvrRect);
        }

        currentPos.setX(currentPos.x() + 142);
    }

    // ================= GPS =================
    if (!m_bottomGpsPixmap.isNull()) {
        m_gpsRect = QRect(currentPos, m_bottomGpsPixmap.size());
        m_painter.drawPixmap(currentPos, m_bottomGpsPixmap);

        if (m_focus == FOCUS_GPS) {
            drawFocusEffect(m_gpsRect);
        }

        currentPos.setX(currentPos.x() + 605);
    }

    // ================= 拨号 =================
    if (!m_bottomDialerPixmap.isNull()) {
        m_dialerRect = QRect(currentPos, m_bottomDialerPixmap.size());
        m_painter.drawPixmap(currentPos, m_bottomDialerPixmap);

        if (m_focus == FOCUS_DIALER) {
            drawFocusEffect(m_dialerRect);
        }

        currentPos.setX(currentPos.x() + 142);
    }

    // ================= 设置 =================
    if (!m_bottomSettingsPixmap.isNull()) {
        m_settingsRect = QRect(currentPos, m_bottomSettingsPixmap.size());
        m_painter.drawPixmap(currentPos, m_bottomSettingsPixmap);

        if (m_focus == FOCUS_SETTINGS) {
            drawFocusEffect(m_settingsRect);
        }
    }

    m_painter.restore();

    // ================= 底部蓝色背景 =================
    m_painter.save();
    if (!m_blueBottomPixmap.isNull()) {
        m_painter.drawPixmap(393, 431, m_blueBottomPixmap);
    }
    m_painter.restore();
}

void clusterappTwoWheelLargeScreen::drawFocusEffect(const QRect& rect)
{
    m_painter.save();

    QRect focusRect = rect.adjusted(-2, -2, 2, 2);

    QRect screenRect(0, 0, 1024, 600);
    focusRect = focusRect.intersected(screenRect);


    QRect overlayRect = rect.adjusted(1, 1, -1, -1);
    QColor overlayColor(0, 150, 255, 60);
    m_painter.fillRect(overlayRect, overlayColor);

    QPen pen(QColor(0, 200, 255));
    pen.setWidth(2);

    m_painter.setPen(pen);
    m_painter.setBrush(Qt::NoBrush);

    m_painter.drawRoundedRect(focusRect, 6, 6);

    m_painter.restore();
}




