#ifndef CLUSTERAPPSERVICEADAPTER_H
#define CLUSTERAPPSERVICEADAPTER_H
#include "icluster.h"
#include "clusterappiviinfo.h"
#include <QObject>
#include <QString>

class ClusterAppServiceAdapter :  public QObject, public IClusterCallBack
{
    Q_OBJECT
public:
    ClusterAppServiceAdapter();
    void start();
    void playpause();
    void musicPre();
    void musicNext();
    void call();
    void hangup();
    void startIVIProjection(int x, int y, int w, int h);
    void stopIVIProjection();

    void onMusicStateChanged(int status);
    void onMusicNameChanged(const std::string &musicName);
    void onCallStatusChanged(int status);
    void onCallNumberChanged(const std::string &number);
    void onCallPersonChanged(const std::string &person);
    void onCallTimeChanged(const std::string &time);
    void onCallPixmapChanged(const char unsigned *data, unsigned int length);
    void onCurrentAlbumPixmapChanged(const char unsigned *data, unsigned int length);
    void onUpdateStateChanged(int state);
    void onUpdateProgressChanged(int progress);
    void onDisconnect();

signals:
    void sigMusicStateChanged(int status);
    void sigMusicNameChanged(const QString &musicName);
    void sigCurrentAlbumPixmapChanged(const QPixmap &pixmap);
    void sigCallStatusChanged(int status);
    void sigCallNumberChanged(const QString &number);
    void sigCallPersonChanged(const QString &person);
    void sigCallTimeChanged(const QString &time);
    void sigCallPixmapChanged(const QPixmap &pixmap);
    void sigUpdateStateChanged(int state);
    void sigUpdateProgressChanged(int progress);
    void sigDisconnected();

private:
    void initConnect();
    ICluster *m_cluster;
    ClusterAppIVIInfo *m_iviInfo;
};

#endif // CLUSTERAPPSERVICEADAPTER_H
