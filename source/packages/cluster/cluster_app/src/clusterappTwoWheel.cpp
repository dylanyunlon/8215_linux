#include "clusterappTwoWheel.h"
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
const static char *TAG = "ClusterappTwoWheel";

void writeBootProf2(const QString &text)
{
    int fd = open("/proc/bootprof", O_CREAT | O_RDWR | O_APPEND);
    if (fd < 0) {
        //   syslog(LOG_ERR, "open /proc/bootprof failed!\n");
        return;
    }
    write(fd, text.toStdString().c_str(), text.length());
    ::close(fd);
}

clusterappTwoWheel::clusterappTwoWheel(const QRect &showRect)
    : ClusterAppPainter(showRect)
{
}

void clusterappTwoWheel::draw(QPaintDevice *device)
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
    drawCarLine(240, 330, 368.5, 367);
    drawSpeed();
    drawRpm();
    drawBottomInfo();
    drawIVIInfo(326, 161);
   // drawUpdateInfo();

    m_painter.end();
}

void clusterappTwoWheel::drawBackground(QPaintDevice * device)
{
/**
    m_fristBackgroundPixmap.load(":/images/twowheel/background.png");

    m_painter.begin(device);
    if (!m_fristBackgroundPixmap.isNull()) {
        m_painter.drawPixmap(0, 0, m_fristBackgroundPixmap);
    } else {
        writeBootProf2("m_fristBackgroundPixmap is nullptr");
    }
    m_painter.end();
    **/
    QThread *thread = new QThread();
    QObject::connect(thread, &QThread::started, [=](){
       if (checkFirstDraw()) {
           m_backgroundPixmap.load(":/images/twowheel/background1.png");
           m_pointerPixmap.load(":/images/twowheel/pointer.png");
           m_pointerPixmap1.load(":/images/twowheel/pointer1.png");
           m_pointerShadePixmap.load(":/images/twowheel/pointershade.png");
           m_pointerShadePixmap2.load(":/images/twowheel/pointershade2.png");
           m_statusBlutoothPixmap.load(":/images/twowheel/bluethooth.png");
           m_statusWifiPixmap.load(":/images/twowheel/wifi.png");
           m_statusGpsPixmap.load(":/images/twowheel/gps.png");
           m_statusMusicPixmap.load(":/images/twowheel/music.png");
           m_bottomGpsPixmap.load(":/images/twowheel/bottom_gps.png");
           m_bottomDialerPixmap.load(":/images/twowheel/bottom_dialer.png");
           m_bottomSettingsPixmap.load(":/images/twowheel/bottom_settings.png");
           m_bottomDVRPixmap.load(":/images/twowheel/bottom_dvr.png");
           m_highbeamPixmap.load(":/images/twowheel/highbeam.png");
           m_highbeamindPixmap.load(":/images/twowheel/highbeam_ind.png");
           m_lowbeamPixmap.load(":/images/twowheel/lowbeam.png");
           m_lowbeamindPixmap.load(":/images/twowheel/lowbeam_ind.png");
       }
       thread->quit();
     });
    thread->start();
    draw(device);
}

void clusterappTwoWheel::drawStatus()
{
    m_painter.save();
    m_painter.translate(30, 15);
    
    if (!m_statusBlutoothPixmap.isNull())
        m_painter.drawPixmap(QPoint(0, 0), m_statusBlutoothPixmap);

    m_painter.translate(55, 0);
    
    if (!m_statusWifiPixmap.isNull())
        m_painter.drawPixmap(QPoint(0, 0), m_statusWifiPixmap);

    m_painter.translate(607, 0);
    
    if (!m_statusGpsPixmap.isNull())
        m_painter.drawPixmap(QPoint(0, 0), m_statusGpsPixmap);

    m_painter.translate(55, 0);
    
    if (!m_statusMusicPixmap.isNull())
        m_painter.drawPixmap(QPoint(0, 0), m_statusMusicPixmap);

    m_painter.restore();
}

void clusterappTwoWheel::drawTopTip()
{
    m_painter.save();
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString formattedTime = currentDateTime.toString("HH:mm");
    QDate currentDate = currentDateTime.date();
    QStringList weekDays = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
    int dayOfWeek = currentDate.dayOfWeek(); // 1 = Monday, 7 = Sunday
    QString formatDayOfWeek = weekDays[dayOfWeek % 7];
    m_painter.translate(240, 95);

    font.setPixelSize(20);
    m_painter.setFont(font);
    m_painter.setPen(QPen(QColor("#80eee1")));
    m_painter.drawText(0, 0, formatDayOfWeek);

    font.setPixelSize(24);
    m_painter.setFont(font);

    m_painter.translate(135, 0);
    m_painter.drawText(0, 0, formattedTime);

    font.setPixelSize(20);
    m_painter.setFont(font);
    m_painter.translate(135, 0);
    m_painter.drawText(0, 0, QString("%1°C").arg(m_value->getAirTemp()));

    m_painter.restore();
}

void clusterappTwoWheel::drawIcon()
{
    m_painter.save();

    m_painter.translate(164, 8);
    int span = 9;
    int width = 60;
    for (int i = 0; i < ICONNAME_List.size(); i++) {
        QPixmap *pix = m_value->getIndicationIcon(ICONNAME_List[i]);
        if (pix && !pix->isNull())
            m_painter.drawPixmap(QPoint(0, 0), *pix);

        m_painter.translate(span + width, 0);
    }

    m_painter.restore();
}

void clusterappTwoWheel::drawSpeed()
{
    m_painter.save();

    //draw speed
    m_painter.translate(636, 243);

    drawSpeedValue();

    m_painter.rotate(-155 + m_value->getSpeed() * 2.32);

    if (!m_pointerShadePixmap.isNull())
        m_painter.drawPixmap(-114.5, -114.5, m_pointerShadePixmap);

    m_painter.rotate(2);

    if (!m_pointerPixmap.isNull())
        m_painter.drawPixmap(-114.5, -152.5, m_pointerPixmap);

    m_painter.restore();
}

void clusterappTwoWheel::drawLight()
{
    m_painter.save();

    //draw light
    m_painter.translate(343, 132);
    int span = 9;
    int width = 60;
    for (int i = 0; i < LIGHT_List.size(); i++) {
        QPixmap *pix = m_value->getLightIcon(LIGHT_List[i]);
        if (pix && !pix->isNull())
            m_painter.drawPixmap(QPoint(0, 0), *pix);

        m_painter.translate(span + width, 0);
    }

    m_painter.restore();
}

void clusterappTwoWheel::drawRpm()
{
    m_painter.save();
    //draw rpm
    m_painter.translate(163.5, 243);
    drawRpmValue();
    m_painter.rotate(-138 + m_value->getRpm() * 33.75);

    if (!m_pointerShadePixmap.isNull())
        m_painter.drawPixmap(-113.8, -113.8, m_pointerShadePixmap);

    m_painter.rotate(2);

    if (!m_pointerPixmap.isNull())
        m_painter.drawPixmap(-114.5, -151.8, m_pointerPixmap);

    m_painter.restore();
}

void clusterappTwoWheel::drawSpeedValue()
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

void clusterappTwoWheel::drawRpmValue()
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

void clusterappTwoWheel::drawBottomInfo()
{
    m_painter.save();

    QPoint basePos(20, 409);
    QPoint currentPos = basePos;

    // ================= DVR =================
    if (!m_bottomDVRPixmap.isNull()) {
        m_dvrRect = QRect(currentPos, m_bottomDVRPixmap.size());
        m_painter.drawPixmap(currentPos, m_bottomDVRPixmap);

        if (m_focus == FOCUS_DVR) {
            drawFocusEffect(m_dvrRect);
        }

        currentPos.setX(currentPos.x() + 115); // 添加间距

    }

    // ================= GPS =================
    if (!m_bottomGpsPixmap.isNull()) {
        m_gpsRect = QRect(currentPos, m_bottomGpsPixmap.size());
        m_painter.drawPixmap(currentPos, m_bottomGpsPixmap);

        if (m_focus == FOCUS_GPS) {
            drawFocusEffect(m_gpsRect);
        }

        currentPos.setX(currentPos.x() + 466);

    }

    // ================= 拨号 =================
    if (!m_bottomDialerPixmap.isNull()) {
        m_dialerRect = QRect(currentPos, m_bottomDialerPixmap.size());
        m_painter.drawPixmap(currentPos, m_bottomDialerPixmap);

        if (m_focus == FOCUS_DIALER) {
            drawFocusEffect(m_dialerRect);
        }

        currentPos.setX(currentPos.x() + 115);

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

}

void clusterappTwoWheel::drawFocusEffect(const QRect& rect)
{
    m_painter.save();

    QRect focusRect = rect.adjusted(-2, -2, 2, 2);

    QRect screenRect(0, 0, 800, 480);
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


