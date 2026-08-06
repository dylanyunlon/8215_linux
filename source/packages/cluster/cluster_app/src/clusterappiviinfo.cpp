#include "clusterappiviinfo.h"
#include "clusterapppixmapmanager.h"

ClusterAppIVIInfo *ClusterAppIVIInfo::s_instance = nullptr;
ClusterAppIVIInfo::ClusterAppIVIInfo(QObject *parent)
    : QObject(parent)
{

}

ClusterAppIVIInfo *ClusterAppIVIInfo::getInstance()
{
    if (!s_instance) {
        s_instance = new ClusterAppIVIInfo();
    }

    return s_instance;
}

void ClusterAppIVIInfo::setCallStatus(int callStatus)
{
    m_callstatus = (CallStatus)callStatus;
    if (callStatus == Idle) {
        clearCallInfo();
    }
}

void ClusterAppIVIInfo::setPlayStatus(int playStatus)
{
    m_playstatus = (PlayStatus)playStatus;
    if (playStatus == Stop || playStatus == Paused ) {
        clearMusicInfo();
    }
}

void ClusterAppIVIInfo::setMediaName(const QString &musicName)
{
    m_musicName = musicName;
}

void ClusterAppIVIInfo::setMediaPixmap(const QPixmap &pixmap)
{
    m_mediaPixmap = pixmap;
}


void ClusterAppIVIInfo::setCallPersonName(const QString &personName)
{
    m_personName = personName;
}

void ClusterAppIVIInfo::setPersonPixmap(const QPixmap &pixmap)
{
    m_personPixmap = pixmap;
}

void ClusterAppIVIInfo::setCallNumber(const QString &callNumber)
{
    m_callNumber = callNumber;
}

void ClusterAppIVIInfo::setCallTime(const QString &callTime)
{
    m_callTime = callTime;
}

ClusterAppIVIInfo::CallStatus ClusterAppIVIInfo::getCallStatus() const
{
    return m_callstatus;
}

ClusterAppIVIInfo::PlayStatus ClusterAppIVIInfo::getPlayStatus() const
{
    return m_playstatus;
}

QString ClusterAppIVIInfo::getMusicName() const
{
    return m_musicName;
}

QString ClusterAppIVIInfo::getPersonName() const
{
    return m_personName;
}

const QPixmap &ClusterAppIVIInfo::getPersonPixmap() const
{
    return m_personPixmap;
}

const QPixmap &ClusterAppIVIInfo::getMediaPixmap() const
{
    return m_mediaPixmap;
}

QString ClusterAppIVIInfo::getCallNumber() const
{
    return m_callNumber;
}

QString ClusterAppIVIInfo::getCallTime() const
{
    return m_callTime;
}

void ClusterAppIVIInfo::clearCallInfo()
{
    m_personName.clear();
    m_callNumber.clear();
    m_callTime = "00:00";
    m_personPixmap = QPixmap();
}

void ClusterAppIVIInfo::clearMusicInfo()
{
    m_musicName.clear();
    m_mediaPixmap = QPixmap();
}

void ClusterAppIVIInfo::clear()
{
    m_playstatus = Stop;
    m_callstatus = Idle;
    clearCallInfo();
    clearMusicInfo();
}

void ClusterAppIVIInfo::setNavgation(const bool isNavgation){
       m_isNavgation = isNavgation;
}

void ClusterAppIVIInfo::setDirectionStatus(int directionStatus)
{
        m_directionsstatus = (ClusterAppIVIInfo::DirectionStatus)directionStatus;
}

ClusterAppIVIInfo::DirectionStatus ClusterAppIVIInfo::getDirectionStatus() const
{
     return m_directionsstatus;
}

bool ClusterAppIVIInfo::getNavgation() const
{
    return m_isNavgation;

}

