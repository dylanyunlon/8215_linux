#include "clusterappupdateinfo.h"
#include <unistd.h>
#include <sys/reboot.h>
#include <QMap>
#include <QTimer>
ClusterAppUpdateInfo* ClusterAppUpdateInfo::s_instance = nullptr;
ClusterAppUpdateInfo *ClusterAppUpdateInfo::getInstance()
{
    if (s_instance == nullptr) {
        s_instance = new ClusterAppUpdateInfo();
    }

    return s_instance;
}

void ClusterAppUpdateInfo::draw(QPainter & painter)
{
    QFont font = painter.font();
    font.setPixelSize(20);
    painter.setFont(font);

    const int WIDTH = 350;
    const int HEIGHT = 36;
    const int RADIS = 5;

    if (m_state != -1) {
        int frontWidth = WIDTH * m_progress / 100.0;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(50, 136, 232, 200)));
        QPainterPath path;
        path.addRoundedRect(0, 0, WIDTH, HEIGHT, RADIS, RADIS);
        painter.setClipPath(path);
        painter.drawRoundedRect(0, 0, frontWidth, HEIGHT, RADIS, RADIS);

        const int UPDATE_ERROR = 2;
        m_state == UPDATE_ERROR ? painter.setPen(Qt::red) : painter.setPen(Qt::green);
        painter.drawText(0, 0, WIDTH, HEIGHT, Qt::AlignCenter, QString("%1 %2%").arg(getStateString()).arg(m_progress));

    } else if (m_lastUpdateStatus != NO_UPDATE) {
        const int UPDATE_OK = 0;
        const int UPDATE_FAILE = 1;
        const int UPDATE_UNFINSHED = 3;
        m_lastUpdateStatus == UPDATE_OK ? painter.setPen(Qt::green) : painter.setPen(Qt::red);
/**
        if (m_lastUpdateStatus == UPDATE_OK)
            painter.drawText(0, 0, WIDTH, HEIGHT, Qt::AlignCenter, QString("升级成功，版本号:%1").arg(m_version));
        else if (m_lastUpdateStatus == UPDATE_FAILE)
            painter.drawText(0, 0, WIDTH, HEIGHT, Qt::AlignCenter, QString("升级失败"));
        else if (m_lastUpdateStatus == UPDATE_UNFINSHED)
            painter.drawText(0, 0, WIDTH, HEIGHT, Qt::AlignCenter, QString("升级未完成"));
**/
    }
}

void ClusterAppUpdateInfo::setProgress(int progress)
{
    m_progress = progress;
}

void ClusterAppUpdateInfo::setState(int state)
{
    m_state = state;
    const int UPDATE_FINISHED = 3;
    if (state == UPDATE_FINISHED) {
        QTimer::singleShot(3000, [] {
            sync();
            reboot(RB_AUTOBOOT);
        });
    }
}

void ClusterAppUpdateInfo::setLastUpdateInfo(int lastStatus, const QString &version)
{
    m_version = version;
    m_lastUpdateStatus = lastStatus;
}

QString ClusterAppUpdateInfo::getStateString()
{
    static const QStringList stateStrings = {"升级暂停", "升级就绪", "升级错误", "升级完成,等待重启", "正在升级...", "磁盘未挂载", "正在校验..."};

    return QString("Cluster%1").arg(stateStrings[m_state]);
}

ClusterAppUpdateInfo::ClusterAppUpdateInfo(QObject *parent) : QObject(parent)
{

}
