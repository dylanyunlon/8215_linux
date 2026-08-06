#include "clusterappserviceadapter.h"
#include "clog.h"
#include "clusterappupdateinfo.h"
#include <QPixmap>
const static char *TAG = "ClusterAppReceiver";
using cluster_utils::CLog;

ClusterAppServiceAdapter::ClusterAppServiceAdapter()
    : m_cluster(ICluster::getInstance())
    , m_iviInfo(ClusterAppIVIInfo::getInstance())
{
    m_cluster->registerCallBack(this);
    initConnect();
}

void ClusterAppServiceAdapter::start()
{
    m_cluster->startService();
}

void ClusterAppServiceAdapter::initConnect()
{
    //music
    connect(this, &ClusterAppServiceAdapter::sigMusicStateChanged, m_iviInfo, &ClusterAppIVIInfo::setPlayStatus);
    connect(this, &ClusterAppServiceAdapter::sigMusicNameChanged, m_iviInfo, &ClusterAppIVIInfo::setMediaName);
    connect(this, &ClusterAppServiceAdapter::sigCurrentAlbumPixmapChanged, m_iviInfo, &ClusterAppIVIInfo::setMediaPixmap);
    //call
    connect(this, &ClusterAppServiceAdapter::sigCallStatusChanged, m_iviInfo, &ClusterAppIVIInfo::setCallStatus);
    connect(this, &ClusterAppServiceAdapter::sigCallPersonChanged, m_iviInfo, &ClusterAppIVIInfo::setCallPersonName);
    connect(this, &ClusterAppServiceAdapter::sigCallNumberChanged, m_iviInfo, &ClusterAppIVIInfo::setCallNumber);
    connect(this, &ClusterAppServiceAdapter::sigCallTimeChanged, m_iviInfo, &ClusterAppIVIInfo::setCallTime);
    connect(this, &ClusterAppServiceAdapter::sigCallPixmapChanged, m_iviInfo, &ClusterAppIVIInfo::setPersonPixmap);
    connect(this, &ClusterAppServiceAdapter::sigDisconnected, m_iviInfo, &ClusterAppIVIInfo::clear);
    //update
    connect(this, &ClusterAppServiceAdapter::sigUpdateStateChanged, ClusterAppUpdateInfo::getInstance(), &ClusterAppUpdateInfo::setState);
    connect(this, &ClusterAppServiceAdapter::sigUpdateProgressChanged, ClusterAppUpdateInfo::getInstance(), &ClusterAppUpdateInfo::setProgress);

}

void ClusterAppServiceAdapter::playpause()
{
    m_cluster->mediaPlayPause();
}

void ClusterAppServiceAdapter::musicPre()
{
    m_cluster->mediaPre();
}

void ClusterAppServiceAdapter::musicNext()
{
    m_cluster->mediaNext();
}

void ClusterAppServiceAdapter::call()
{
    m_cluster->call();
}

void ClusterAppServiceAdapter::hangup()
{
    m_cluster->handup();
}

void ClusterAppServiceAdapter::startIVIProjection(int x, int y, int w, int h)
{
    //m_cluster->startIVIProjection(x, y, w, h);
}

void ClusterAppServiceAdapter::stopIVIProjection()
{
    //m_cluster->stopIVIProjection();
}

void ClusterAppServiceAdapter::onMusicStateChanged(int status)
{
    UTILS_LOGI(TAG, "onMusicStateChanged %d", status);
    emit sigMusicStateChanged(status);
}

void ClusterAppServiceAdapter::onMusicNameChanged(const std::string &musicName)
{
    UTILS_LOGI(TAG, "onMusicNameChanged %s %d", musicName.c_str(), musicName.length());
    emit sigMusicNameChanged(QString::fromStdString(musicName));
}

void ClusterAppServiceAdapter::onCallStatusChanged(int status)
{
    UTILS_LOGI(TAG, "onCallStatusChanged %d", status);
    emit sigCallStatusChanged(status);
}

void ClusterAppServiceAdapter::onCallNumberChanged(const std::string &number)
{
    UTILS_LOGI(TAG, "onCallNumberChanged %s", number.c_str());
    emit sigCallNumberChanged(QString::fromStdString(number));
}

void ClusterAppServiceAdapter::onCallPersonChanged(const std::string &person)
{
    UTILS_LOGI(TAG, "onCallPersonChanged %s", person.c_str());
    emit sigCallPersonChanged(QString::fromStdString(person));
}

void ClusterAppServiceAdapter::onCallTimeChanged(const std::string &time)
{
    UTILS_LOGI(TAG, "onCallTimeChanged %s", time.c_str());
    emit sigCallTimeChanged(QString::fromStdString(time));
}

void ClusterAppServiceAdapter::onCallPixmapChanged(const char unsigned *data, unsigned int length)
{
    UTILS_LOGI(TAG, "onCallPixmapChanged %d", length);
    QImage image((const uchar *)data , 100 ,100 , QImage::Format_RGBA8888);
    QPixmap pixmap;
    pixmap = QPixmap::fromImage(image);
    emit sigCallPixmapChanged(pixmap);
}

void ClusterAppServiceAdapter::onCurrentAlbumPixmapChanged(const char unsigned *data, unsigned int length)
{
    UTILS_LOGI(TAG, "onCurrentAlbumPixmapChanged %d", length);
    QByteArray byteArray(reinterpret_cast<const char*>(data),length);

    QImage image;
    image.loadFromData(byteArray);
    QImage scaledImage = image.scaled(100, 100 , Qt::IgnoreAspectRatio);

    QPixmap pixmap;
    pixmap = QPixmap::fromImage(scaledImage);
    emit sigCurrentAlbumPixmapChanged(pixmap);
}

void ClusterAppServiceAdapter::onUpdateStateChanged(int state)
{
    UTILS_LOGI(TAG, "onUpdateStateChanged %d", state);
    emit sigUpdateStateChanged(state);
}

void ClusterAppServiceAdapter::onUpdateProgressChanged(int progress)
{
    UTILS_LOGI(TAG, "onUpdateProgressChanged %d", progress);
    emit sigUpdateProgressChanged(progress);
}

void ClusterAppServiceAdapter::onDisconnect()
{
    UTILS_LOGI(TAG, "onDisconnect");
    emit sigDisconnected();
}


