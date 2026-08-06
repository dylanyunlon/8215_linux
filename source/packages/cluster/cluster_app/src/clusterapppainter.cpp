#include "clusterapppainter.h"
#include "clusterappupdateinfo.h"
#include <QTime>
#include "clog.h"

using cluster_utils::CLog;

const static char *TAG = "ClusterAppPainter";

ClusterAppPainter::ClusterAppPainter(const QRect &showRect)
    : m_showRect(showRect)
{
    m_needScale = !(m_showRect.width() == 800 && m_showRect.height() == 480);
    UTILS_LOGI(TAG, "ClusterAppPainter show %d %d %d", m_showRect.width(), m_showRect.height(), m_needScale);
}

bool ClusterAppPainter::checkFirstDraw()
{
    bool ret = false;
    if (m_iviInfo == nullptr) {
        m_iviInfo = ClusterAppIVIInfo::getInstance();
    }

    if (m_value == nullptr) {
        m_value = ClusterAppValue::getInstance();
    }

    if (m_pixmapManager == nullptr) {
        m_pixmapManager = ClusterAppPixmapManager::getInstance();
    }

    if (m_performance == nullptr) {
        m_performance = ClusterAppPerformance::getInstance();
    }

    if (m_configure == nullptr) {
        m_configure = ClusterAppConfigure::getInstance();
        ret = true;
    }

    return ret;
}

void ClusterAppPainter::drawIVIInfo(int x, int yoff)
{
    m_painter.save();
    if(m_iviInfo == nullptr) {
           m_painter.restore();
           return;
    }
    if (m_iviInfo->getCallStatus() != ClusterAppIVIInfo::Idle) {
        drawCallInfo(x , yoff);
    } else if (m_iviInfo->getPlayStatus() == ClusterAppIVIInfo::Playing) {
        drawMusicInfo(x , yoff);
    } else if(m_iviInfo->getNavgation()) {
        drawNavInfo(x , yoff);
    }

    m_painter.restore();

}

void ClusterAppPainter::drawCarLine(int x1, int y1, int x2, int y2)
{
    m_painter.save();

    static int i = 0;
    static int count = 0;
    if (m_value->getSpeed() > 0) {
        ++count;
        if (count > 3 - m_value->getSpeed() / 35 || m_needScale) {
            ++i;
            if (i >= 5) {
                i = 0;
            }
            count = 0;
        }
    }
    if(m_pixmapManager == nullptr) {
           m_painter.restore();
           return;
    }
    if(m_pixmapManager->getLinePixmap(i) == nullptr) {
          m_painter.restore();
           return;
    }
    m_painter.drawPixmap(x1, y1, *m_pixmapManager->getLinePixmap(i));
    if(m_pixmapManager->getPixmap(CarPixmap) == nullptr) {
         m_painter.restore();
          return;
    }
    m_painter.drawPixmap(x2, y2, *m_pixmapManager->getPixmap(CarPixmap));
    m_painter.restore();
}

void ClusterAppPainter::drawCarDoorStatus(int x, int y)
{
    m_painter.translate(x, y);
    if(m_pixmapManager == nullptr) {
           m_painter.restore();
           return;
    }
    m_painter.drawPixmap(0, 0, 150, 140, *m_pixmapManager->getCarDoorPixmap(m_value->getCardoorStatus()));
    QFont font;
    font.setPixelSize(12);
    m_painter.setFont(font);
    m_painter.setPen(Qt::green);
    m_painter.drawText(30, 35,  QString::number(m_value->getTirePressure(0)));
    m_painter.drawText(98, 35, QString::number(m_value->getTirePressure(1)));
    m_painter.drawText(30, 115,  QString::number(m_value->getTirePressure(2)));
    m_painter.drawText(98, 115, QString::number(m_value->getTirePressure(3)));
    m_painter.setPen(Qt::white);
    m_painter.drawText(47, 140,  "TPMS(KPa)");
    m_painter.restore();
    m_painter.save();
}

//drawUpdateInfo
void ClusterAppPainter::drawUpdateInfo()
{
    m_painter.save();

    m_painter.translate(1400, 10);
    ClusterAppUpdateInfo::getInstance()->draw(m_painter);
    m_painter.restore();

}

void ClusterAppPainter::drawCallInfo(int x, int yoff)
{
    /**
    if(m_pixmapManager == nullptr) {
           return;
    }

    QPixmap pix = m_iviInfo->getPersonPixmap();
    m_painter.translate(x, yoff);
    m_painter.drawPixmap(0, 0, pix);

    if(pix.isNull()){
         m_painter.drawPixmap(-40, -20, *(m_pixmapManager->getPixmap(CallPerson)));
    } else {
         m_painter.drawPixmap(-40, -20, pix);
    }

    if (m_iviInfo->getCallStatus() == ClusterAppIVIInfo::Incoming) {
        m_painter.drawPixmap(-40, 90, *(m_pixmapManager->getPixmap(CallHangup)));
        m_painter.drawPixmap(35, 90, *(m_pixmapManager->getPixmap(CallAnswer)));
    } else {
        m_painter.drawPixmap(0, 100, *(m_pixmapManager->getPixmap(CallHangup)));
    }

    QFont font;
    font.setPixelSize(25);
    m_painter.setFont(font);
    m_painter.setPen(Qt::white);
    m_painter.drawText(64, -10, m_iviInfo->getPersonName());
    m_painter.drawText(64, 30, m_iviInfo->getCallNumber());
    m_painter.drawText(64, 70, m_iviInfo->getCallTime());
    **/

    if(m_pixmapManager == nullptr) {
           return;
    }

    QPixmap pix = m_iviInfo->getPersonPixmap();
    m_painter.translate(x, yoff);
    m_painter.drawPixmap(0, 0, pix);

    if(pix.isNull()){
       //  m_painter.drawPixmap(-40, -20, *(m_pixmapManager->getPixmap(CallPerson)));
    } else {
       //  m_painter.drawPixmap(-40, -20, pix);
    }

    if (m_iviInfo->getCallStatus() == ClusterAppIVIInfo::Incoming) {
        m_painter.drawPixmap(22, 86, *(m_pixmapManager->getPixmap(CallHangup)));
        m_painter.drawPixmap(77, 86, *(m_pixmapManager->getPixmap(CallAnswer)));
    } else {
        m_painter.drawPixmap(45, 86, *(m_pixmapManager->getPixmap(CallHangup)));
    }

    QFont font;
    font.setPixelSize(25);
    QFontMetrics fm(font);
    int textWidth = fm.width(m_iviInfo->getPersonName());
    int widgetWidth = m_showRect.width();
    int left = (widgetWidth - textWidth) /2;
    m_painter.setFont(font);
    m_painter.setPen(Qt::white);
    m_painter.drawText(left - x, 20, m_iviInfo->getPersonName());
    m_painter.drawText(left - x, 47, m_iviInfo->getCallNumber());

    font.setPixelSize(15);
    m_painter.setFont(font);
    m_painter.drawText(left - x, 75, m_iviInfo->getCallTime());

}


void ClusterAppPainter::drawNavInfo(int x, int yoff)
{
    bool isNav = m_iviInfo->getNavgation();
    if(m_pixmapManager == nullptr) {
           return;
    }
    if(isNav) {
        m_painter.translate(x, yoff);
        if(m_pixmapManager->getPixmap(NavIndication) == nullptr) {
           return;
        }
        m_painter.drawPixmap(0, 0, *(m_pixmapManager->getPixmap(NavIndication)));
        int driection = m_iviInfo->getDirectionStatus();
        if(driection == ClusterAppIVIInfo::LEFT) {
            if(m_pixmapManager->getPixmap(navleft) == nullptr) {
                return;
            }
            m_painter.drawPixmap(45, 153, *(m_pixmapManager->getPixmap(navleft)));
        }else if(driection == ClusterAppIVIInfo::RIGHT) {
            if(m_pixmapManager->getPixmap(navright) == nullptr) {
                return;
            }

            m_painter.drawPixmap(45, 153, *(m_pixmapManager->getPixmap(navright)));
        }else {
            if(m_pixmapManager->getPixmap(navhead) == nullptr) {
                return;
            }
            m_painter.drawPixmap(53.5, 153, *(m_pixmapManager->getPixmap(navhead)));
        }
    }
}


void ClusterAppPainter::drawMusicInfo(int x, int yoff)
{
    /**
    if(m_pixmapManager == nullptr) {
           return;
    }

    QPixmap pix = m_iviInfo->getMediaPixmap();
    m_painter.translate(x, yoff);

    if(pix.isNull()){
         if(m_pixmapManager->getPixmap(MusicArtist) == nullptr) {
             return;
         }
         QPixmap scaled = m_pixmapManager->getPixmap(MusicArtist)->scaled(50, 50,
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
         m_painter.drawPixmap(-40, -10, scaled);
    } else {
         m_painter.drawPixmap(-40, -40, pix);
    }

    QFont font;
    font.setPixelSize(20);
    m_painter.setFont(font);
    m_painter.setPen(Qt::white);
    static QString musicText;
    static int loop = 0;
    static int SHOW_LENGTH = 20;
    static int loopTimes = 0;
    if (musicText != m_iviInfo->getMusicName()) {
        musicText = m_iviInfo->getMusicName();
        loop = 0;
        QString t;
        for (int i = loop; i < musicText.size(); ++i) {
            t.append(musicText.at(i));
            if (QFontMetrics(font).width(t) > 400) {
                break;
            }
        }

        SHOW_LENGTH = t.size();
    }

    if (musicText.size() < SHOW_LENGTH) {
        m_painter.drawText(10, 15, musicText);
    } else {
        if (loopTimes > 50) {
            loopTimes = 0;
            loop += 1;
        } else {
             ++loopTimes;
             m_painter.drawText(10, 15, musicText.mid(loop, SHOW_LENGTH));
             return;
        }

        if (loop + SHOW_LENGTH > musicText.size()) {
            loop = 0;
        }
        QString t;
        for (int i = loop; i < musicText.size(); ++i) {
            t.append(musicText.at(i));
            if (QFontMetrics(font).width(t) > 400) {
                break;
            }
        }

        SHOW_LENGTH = t.size();
        m_painter.drawText(10, 15, musicText.mid(loop, SHOW_LENGTH));
    }
   **/
    if(m_pixmapManager == nullptr) {
           return;
    }

    QPixmap pix = m_iviInfo->getMediaPixmap();
    m_painter.translate(x, yoff);

    if(pix.isNull()){
         if(m_pixmapManager->getPixmap(MusicArtist) == nullptr) {
             return;
         }
         QPixmap scaled = m_pixmapManager->getPixmap(MusicArtist)->scaled(60, 60,
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
         m_painter.drawPixmap(-20, 10, scaled);
    } else {
         m_painter.drawPixmap(-20, 10, pix);
    }

    QFont font;
    font.setPixelSize(25);
    m_painter.setFont(font);
    m_painter.setPen(Qt::white);
    static QString musicText;
    static int loop = 0;
    static int SHOW_LENGTH = 20;
    static int loopTimes = 0;
    if (musicText != m_iviInfo->getMusicName()) {
        musicText = m_iviInfo->getMusicName();
        loop = 0;
        QString t;
        for (int i = loop; i < musicText.size(); ++i) {
            t.append(musicText.at(i));
            if (QFontMetrics(font).width(t) > 130) {
                break;
            }
        }

        SHOW_LENGTH = t.size();
    }

    if (musicText.size() < SHOW_LENGTH) {
        m_painter.drawText(43, 40, musicText);
    } else {
        if (loopTimes > 50) {
            loopTimes = 0;
            loop += 1;
        } else {
             ++loopTimes;
             m_painter.drawText(43, 40, musicText.mid(loop, SHOW_LENGTH));
             return;
        }

        if (loop + SHOW_LENGTH > musicText.size()) {
            loop = 0;
        }
        QString t;
        for (int i = loop; i < musicText.size(); ++i) {
            t.append(musicText.at(i));
            if (QFontMetrics(font).width(t) > 130) {
                break;
            }
        }

        SHOW_LENGTH = t.size();
        m_painter.drawText(43, 40, musicText.mid(loop, SHOW_LENGTH));
    }

}

void ClusterAppPainter::drawInfo()
{
    m_painter.save();
    if(!m_configure) {
        m_painter.restore();
        return;
    }
    if (m_configure->isShowPerformance()) {
        font.setPixelSize(15);
        m_painter.setFont(font);
        m_painter.setPen(Qt::green);
        int span = 0;
        for (const QString &text : m_performance->getPerformance()) {
            m_painter.drawText(10, 17 + span, text);
            span += 17;
        }
    }
    m_painter.restore();
}

void ClusterAppPainter::drawFocusPath(const QPainterPath& path, bool focused)
{
    if (!focused) return;

    m_painter.save();
    m_painter.setRenderHint(QPainter::Antialiasing, true);

    QRectF bounds = path.boundingRect();

    QLinearGradient grad(bounds.topLeft(), bounds.bottomLeft());
    grad.setColorAt(0, QColor(0, 200, 255, 80));
    grad.setColorAt(1, QColor(0, 200, 255, 10));

    m_painter.fillPath(path, grad);

    for (int i = 0; i < 4; i++) {
        QPen glow(QColor(0, 200, 255, 60 - i * 12));
        glow.setWidth(6 + i * 2);
        m_painter.setPen(glow);
        m_painter.drawPath(path);
    }

    QPen pen(QColor(0, 220, 255, 220));
    pen.setWidth(2);
    m_painter.setPen(pen);
    m_painter.drawPath(path);

    m_painter.restore();
}

