#ifndef CLUSTERAPPIVIINFO_H
#define CLUSTERAPPIVIINFO_H

#include <QObject>
#include <QPixmap>
class ClusterAppIVIInfo : public QObject
{
    Q_OBJECT
public:
    enum CallStatus {
        Incoming = 1,     //来电
        Dialing,          //拨打中
        Calling,          //通话中
        Idle,             //空闲,挂断
    };

    enum PlayStatus {
        Stop = 0,
        Playing,
        Paused,
    };

    enum DirectionStatus {
        LEFT = 0,
        RIGHT,
        GOHEAD,
    };

    static ClusterAppIVIInfo *getInstance();

    void setPlayStatus(int playStatus);
    void setMediaName(const QString &musicName);
    void setMediaPixmap(const QPixmap &pixmap);

    void setCallStatus(int callStatus);
    void setCallPersonName(const QString &personName);
    void setPersonPixmap(const QPixmap &pixmap);
    void setCallNumber(const QString &callNumber);
    void setCallTime(const QString &callTime);
    void setNavgation(const bool isNavgation);
    void setDirectionStatus(int directionStatus);

    DirectionStatus getDirectionStatus() const;
    bool getNavgation() const;

    PlayStatus getPlayStatus() const;
    QString getMusicName() const;

    CallStatus getCallStatus() const;
    QString getPersonName() const;
    const QPixmap &getPersonPixmap() const;
    const QPixmap &getMediaPixmap() const;

    QString getCallNumber() const;
    QString getCallTime() const;

    void clearCallInfo();
    void clearMusicInfo();
    void clear();

signals:

public slots:

private:
    explicit ClusterAppIVIInfo(QObject *parent = nullptr);
    ClusterAppIVIInfo(const ClusterAppIVIInfo &) = delete;
    const ClusterAppIVIInfo &operator=(const ClusterAppIVIInfo &) = delete;

    CallStatus m_callstatus = Idle;
    PlayStatus m_playstatus = Stop;
    DirectionStatus m_directionsstatus = LEFT;
    bool m_isNavgation;
    QString m_personName;
    QString m_callNumber;
    QString m_callTime = "00:00";
    QString m_musicName;
    QPixmap m_personPixmap;
    QPixmap m_mediaPixmap;
    QPixmap m_navPixmap;

    static ClusterAppIVIInfo *s_instance;
};

#endif // CLUSTERAPPIVIINFO_H
