#include "clusterappicon.h"
#include "clusterappconstant.h"
#include <QPixmap>
#include "GlobalScreenConfig.h"


ClusterAppIcon::ClusterAppIcon(const QString& name)
    : m_status(false)
    , m_name(name)
{
    if (GlobalScreenConfig::getLargeScreen()) {
        m_path =QString(":/images_1024x600/icon/%1.png").arg(name);
        m_statusOffPixmap = new QPixmap(QString(":/images_1024x600/icon/%1.png").arg(m_name));
        m_statusOnPixmap = new QPixmap(QString(":/images_1024x600/icon/%1_%2.png").arg(m_name).arg("ind"));
    }else {
        m_path =QString(":/images/icon/%1.png").arg(name);
        m_statusOffPixmap = new QPixmap(QString(":/images/icon/%1.png").arg(m_name));
        m_statusOnPixmap = new QPixmap(QString(":/images/icon/%1_%2.png").arg(m_name).arg("ind"));
    }

    m_pixmap = m_statusOffPixmap;
}

QString ClusterAppIcon::getName() const
{
    return m_name;
}

QPixmap* ClusterAppIcon::getPixmap() const
{
    return m_pixmap;
}

void ClusterAppIcon::setStatus(bool status)
{
    m_status = status;
    m_pixmap = status ? m_statusOnPixmap : m_statusOffPixmap;
}

bool ClusterAppIcon::getStatus() const
{
    return m_status;
}

ClusterAppFlashIcon::ClusterAppFlashIcon(const QString& name)
    : ClusterAppIcon(name)
    , m_flashing(false)
{
    m_flashTimer.setInterval(TIME_FLASH);
    connect(&m_flashTimer, SIGNAL(timeout()), this, SLOT(slotFlash()));
}

void ClusterAppFlashIcon::setStatus(bool status)
{
    if (status && !m_flashTimer.isActive()) {
        ClusterAppIcon::setStatus(status);
        m_flashing = true;
        m_flashTimer.start();
    } else if (!status && m_flashTimer.isActive()) {
        m_flashTimer.stop();
        ClusterAppIcon::setStatus(status);
        m_flashing = false;
    }
}

void ClusterAppFlashIcon::slotFlash()
{
     m_flashing = !m_flashing;
     m_pixmap = m_flashing ? m_statusOnPixmap : m_statusOffPixmap;
}
