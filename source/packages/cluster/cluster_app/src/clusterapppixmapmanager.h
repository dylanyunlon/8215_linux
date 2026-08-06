#ifndef CLUSTERAPPPIXMAPMANAGER_H
#define CLUSTERAPPPIXMAPMANAGER_H
#include <QPixmap>
#include <QVector>
#include <QMap>
#include "clusterappconstant.h"


enum PixmapRole {
    //call
    CallAnswer,
    CallHangup,
    CallPerson,
    MusicIndication,
    NavIndication,
    CarPixmap,
    LogoPixmap,
    MusicArtist,
    navleft,
    navright,
    navhead,
};

class ClusterAppPixmapManager
{
public:
    static ClusterAppPixmapManager *getInstance();
    QPixmap *getNumberPixmap(int i) const;
    QPixmap *getBigNumberPixmap(int i) const;
    QPixmap *getCarDoorPixmap(int i) const;
    QPixmap *getLinePixmap(int i) const;
    QPixmap *getPixmap(PixmapRole i) const;
    QPixmap* getGearPixmap(GearPos pos) const;

private:
    ClusterAppPixmapManager();
    QVector<QPixmap*> m_numberPixmaps, m_bignumberPixmaps;
    QVector<QPixmap*> m_carDoorPixmaps;
    QVector<QPixmap*> m_linePixmaps;
    QMap<PixmapRole, QPixmap*> m_pixmaps;
    QMap<GearPos, QPixmap*> m_gearPixmaps;

    static ClusterAppPixmapManager *s_instance;
};

#endif // CLUSTERAPPPIXMAPMANAGER_H
