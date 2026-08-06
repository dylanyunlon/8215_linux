#include "clusterapppixmapmanager.h"
#include "clog.h"

#include "GlobalScreenConfig.h"
#include "clog.h"


using cluster_utils::CLog;

const static char *TAG = "ClusterAppPixmapManager";

ClusterAppPixmapManager *ClusterAppPixmapManager::s_instance = nullptr;
ClusterAppPixmapManager *ClusterAppPixmapManager::getInstance()
{
    if (s_instance == nullptr) {
        s_instance = new ClusterAppPixmapManager();
    }

    return s_instance;
}

QPixmap *ClusterAppPixmapManager::getNumberPixmap(int i) const
{
    return m_numberPixmaps.at(i);
}

QPixmap *ClusterAppPixmapManager::getBigNumberPixmap(int i) const
{
    return m_bignumberPixmaps.at(i);
}

QPixmap *ClusterAppPixmapManager::getCarDoorPixmap(int i) const
{
    return m_carDoorPixmaps.at(i);
}

QPixmap *ClusterAppPixmapManager::getLinePixmap(int i) const
{
    return m_linePixmaps.at(i);
}

QPixmap *ClusterAppPixmapManager::getPixmap(PixmapRole i) const
{
    return m_pixmaps[i];
}

QPixmap *ClusterAppPixmapManager::getGearPixmap(GearPos pos) const
{
    return m_gearPixmaps[pos];
}

ClusterAppPixmapManager::ClusterAppPixmapManager()
{
    const int NUMBER_PIXMAP_COUNT = 10;
    for (int i = 0; i < NUMBER_PIXMAP_COUNT; ++i) {
        if (GlobalScreenConfig::getLargeScreen()) {
            m_numberPixmaps.append(new QPixmap(QString(":/images_1024x600/number/%1.png").arg(i)));
        }else {
            m_numberPixmaps.append(new QPixmap(QString(":/images/number/%1.png").arg(i)));
        }
    }

    for (int i = 0; i < NUMBER_PIXMAP_COUNT; ++i) {
        if (GlobalScreenConfig::getLargeScreen()) {
           // m_bignumberPixmaps.append(new QPixmap(QString(":/images_1024x600/number/big%1.png").arg(i)));
        }else {
            m_bignumberPixmaps.append(new QPixmap(QString(":/images/number/big%1.png").arg(i)));
        }
    }

    const int CAR_PIXMAP_COUNT = 16;
    for (int i = 0; i < CAR_PIXMAP_COUNT; ++i) {
        if (GlobalScreenConfig::getLargeScreen()) {
         //   m_carDoorPixmaps.append(new QPixmap(QString(":/images_1024x600/car/%1.png").arg(i)));
        }else {
            m_carDoorPixmaps.append(new QPixmap(QString(":/images/car/%1.png").arg(i)));
        }

    }

    const int LINE_PIXMAP_COUNT = 5;
    for (int i = 0; i < LINE_PIXMAP_COUNT; ++i) {
        if (GlobalScreenConfig::getLargeScreen()) {
            m_linePixmaps.append(new QPixmap(QString(":/images_1024x600/line/%1.png").arg(i)));
        }else {
            m_linePixmaps.append(new QPixmap(QString(":/images/line/%1.png").arg(i)));
        }
    }

    if (GlobalScreenConfig::getLargeScreen()) {
        m_pixmaps[CallPerson] = new QPixmap(":/images_1024x600/twowheel/middle_tel.png");
        m_pixmaps[MusicIndication] = new QPixmap(":/images_1024x600/twowheel/middle_music.png");
        m_pixmaps[CarPixmap] = new QPixmap(":/images_1024x600/car.png");
        m_pixmaps[MusicArtist] = new QPixmap(":/images_1024x600/twowheel/middle_music.png");
        m_pixmaps[navleft] = new QPixmap(":/images_1024x600/twowheel/nav_turnleft.png");
        m_pixmaps[navright] = new QPixmap(":/images_1024x600/twowheel/nav_turnright.png");
        m_pixmaps[navhead] = new QPixmap(":/images_1024x600/twowheel/nav_gohead.png");
        m_pixmaps[NavIndication] = new QPixmap(":/images_1024x600/twowheel/middle_gps.png");

    }else {
        m_pixmaps[CallPerson] = new QPixmap(":/images/twowheel/middle_tel.png");
        m_pixmaps[MusicIndication] = new QPixmap(":/images/twowheel/middle_music.png");
        m_pixmaps[CarPixmap] = new QPixmap(":/images/car.png");
        m_pixmaps[MusicArtist] = new QPixmap(":/images/twowheel/middle_music.png");
        m_pixmaps[navleft] = new QPixmap(":/images/twowheel/nav_turnleft.png");
        m_pixmaps[navright] = new QPixmap(":/images/twowheel/nav_turnright.png");
        m_pixmaps[navhead] = new QPixmap(":/images/twowheel/nav_gohead.png");
        m_pixmaps[NavIndication] = new QPixmap(":/images/twowheel/middle_gps.png");
    }

    m_pixmaps[MusicArtist] = new QPixmap(":/images/iviinfo/artist.png");
    m_pixmaps[CallHangup] = new QPixmap(":/images/iviinfo/hangup.png");
    m_pixmaps[CallAnswer] = new QPixmap(":/images/iviinfo/answer.png");
    m_pixmaps[CallPerson] = new QPixmap(":/images/iviinfo/person.png");

    m_pixmaps[LogoPixmap] = new QPixmap(":/images/logo.png");

    m_gearPixmaps[GEAR_P] = new QPixmap(":/images/gear/P.png");
    m_gearPixmaps[GEAR_R] = new QPixmap(":/images/gear/R.png");
    m_gearPixmaps[GEAR_N] = new QPixmap(":/images/gear/N.png");
    m_gearPixmaps[GEAR_D] = new QPixmap(":/images/gear/D1.png");

}
