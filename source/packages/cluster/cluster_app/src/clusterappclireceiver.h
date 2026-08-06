#ifndef CLUSTERAPPCLIRECEIVER_H
#define CLUSTERAPPCLIRECEIVER_H

#include <QThread>
#include "clusterappvalue.h"


class ClusterAppCLIReceiver : public QThread{
    Q_OBJECT

public:
    ClusterAppCLIReceiver();

signals:
    void sigSwitchTestMode(bool flag);
    void sigMusicControl(int cmd);
    void sigCallControl(bool flag);
    void sigSwitchTheme(int theme);
    void sigSetVolume(int volume);

    void sigContinusEventUpdate(double speed, double rpm, double fuel, double temperature);
    void sigSwitchEventUpdate(QStringList data);


private:
    void continusEventCallback(char *data);
    void switchEventCallback(char *data);
    void run() override;
    void slotContinusEventUpdate(double speed, double rpm, double fuel, double temperature);
    void slotSwitchEventUpdate(QStringList data);

    ClusterAppValue *m_value;

};

#endif // CLUSTERAPPCLIRECEIVER_H
