#ifndef CLUSTERAPPUPDATEINFO_H
#define CLUSTERAPPUPDATEINFO_H

#include <QObject>
#include <QPainter>
#include <QString>
class ClusterAppUpdateInfo : public QObject
{
    Q_OBJECT
public:
    static ClusterAppUpdateInfo* getInstance();
    void draw(QPainter & painter);
    void setProgress(int progress);
    void setState(int state);
    void setLastUpdateInfo(int lastStatus, const QString &version);
    int m_progress = 0;
signals:

private:
    QString getStateString();

    explicit ClusterAppUpdateInfo(QObject *parent = nullptr);
    static ClusterAppUpdateInfo* s_instance;

    static const int NO_UPDATE = 2;
    int m_state = -1;
    int m_lastUpdateStatus = NO_UPDATE; //2--> NO_UPDATE
    QString m_version = "";
};

#endif // CLUSTERAPPUPDATEINFO_H
