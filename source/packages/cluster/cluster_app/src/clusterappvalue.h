#ifndef CLUSTERAPPVALUE_H
#define CLUSTERAPPVALUE_H
#include <QList>
#include <QObject>
#include <QMap>
#include "clusterappicon.h"
#include "clusterappconstant.h"


class ClusterAppValue : public QObject
{
     Q_OBJECT
public:
    Q_PROPERTY(double speed READ getSpeed WRITE setSpeed)
    Q_PROPERTY(double rpm READ getRpm WRITE setRpm)
    Q_PROPERTY(double fuel READ getFuel WRITE setFuel)
    ~ClusterAppValue();
    static ClusterAppValue *getInstance();
    void reset();
    void setSpeed(double speed);
    void setRpm(double rpm);
    void setFuel(double fuel);
    void setTempareture(double temperature);
    void setAirTemp(int airTemp);
    void setTotalMilage(int totalMilage);
    void setGearPosition(GearPos gearPos);
    void setGear(int gear);
    void setIndicationStatus(const QString &indictionKey, bool status);
    void setLightStatus(const QString &lightKey, bool status);

    void setCardoorStatus(int status);
    void setCardoorStatus(int index, bool open);
    void setTirePressure(int index, int value);
    double getSpeed() const;
    double getRpm() const;
    double getFuel() const;
    double getTempareture() const;
    int getAirTemp() const;
    int getTotalMilage() const;
    int getGear() const;
    GearPos getGearPosition() const;
    QPixmap* getIndicationIcon(const QString &indictionKey) const;
    QPixmap* getLightIcon(const QString &lightKey) const;

    bool getTurnSignalsStatus();

    int getCardoorStatus() const;
    int getTirePressure(int i) const;
private:
    ClusterAppValue();
    ClusterAppValue(const ClusterAppValue &);
    const ClusterAppValue &operator=(const ClusterAppValue &);
    void initPixmaps();
    QList<ClusterAppIcon*> getIndicationIcons();

private:
    static ClusterAppValue *m_instance;
    double m_speed;
    double m_rpm;
    double m_fuel;
    double m_tempareture;
    int m_airtemp = 24;
    int m_totalMilage;
    int m_cardoorStatus = 0;
    int m_gear = 0;

    QList<ClusterAppIcon*> m_indicationIcons;
    QList<ClusterAppIcon*> m_lightIcons;
    GearPos m_gearPosition;
    static const int TIRE_COUNT = 4;
    int m_tirePressureValues[TIRE_COUNT];


};

#endif // DASHBOARDVALUE_H
