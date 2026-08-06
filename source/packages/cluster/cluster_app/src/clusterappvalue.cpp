#include "clusterappvalue.h"
#include "clusterappconstant.h"
#include "clusterappconfigure.h"
#include <QPixmap>

static const double FUEL_EMPTY_VALUE = 0.2;
static const double HIGH_TEMPERATURE_VALUE = 0.8;
ClusterAppValue *ClusterAppValue::m_instance = nullptr;

ClusterAppValue::ClusterAppValue()
{
    initPixmaps();
    reset();
    setIndicationStatus(ICON_TRUN_LEFT, true);
    setIndicationStatus(ICON_TRUN_RIGHT, true);
}

void ClusterAppValue::reset()
{
    m_speed = 0;
    m_rpm = 0;
    m_fuel = 0.8;
    m_tempareture = 0.4;
    m_totalMilage = 0;
    m_gearPosition = GEAR_P;
    m_cardoorStatus = 0;
    m_airtemp = 24;

    for (int i = 0; i < TIRE_COUNT; ++i) {
            m_tirePressureValues[i] = 230;
    }

    for (const QString &indiction : ICONNAME_List) {
        setIndicationStatus(indiction, false);
    }
}

ClusterAppValue* ClusterAppValue::getInstance()
{
    if (m_instance == nullptr) {
        m_instance = new ClusterAppValue();
    }

    return m_instance;
}

ClusterAppValue::~ClusterAppValue()
{
    SAFE_DELETE(m_instance);
}

void ClusterAppValue::setSpeed(double speed)
{
    if (speed >= 0 && speed <= 120){
        m_speed = speed;
    }

    if (speed > 120){
        m_speed = 120;
    }

}

void ClusterAppValue::setRpm(double rpm)
{
    if (rpm >= 0 && rpm <= 8)
        m_rpm = rpm;
}

void ClusterAppValue::setFuel(double fuel)
{
    if (fuel >= 0 && fuel <= 150)
        m_fuel = fuel;
    //setIndicationStatus(ICON_REFUEL, m_fuel > HIGH_TEMPERATURE_VALUE);
}

void ClusterAppValue::setTempareture(double temperature)
{
    if (temperature >= 0 && temperature <= 1)
        m_tempareture = temperature;
    //setIndicationStatus(ICON_TEMPERATURE, m_tempareture > FUEL_EMPTY_VALUE);
}

void ClusterAppValue::setAirTemp(int airTemp)
{
    m_airtemp = airTemp;
}

void ClusterAppValue::setTotalMilage(int totalMilage)
{
    m_totalMilage = totalMilage;
}

void ClusterAppValue::setGear(int gear)
{
    m_gear = gear;
}

int ClusterAppValue::getGear() const
{
    return m_gear;
}


void ClusterAppValue::setGearPosition(GearPos gearPos)
{
    m_gearPosition = gearPos;
}

void ClusterAppValue::setIndicationStatus(const QString &indictionKey, bool status)
{
    QList<ClusterAppIcon*>::iterator it = m_indicationIcons.begin();
    QString iconName = indictionKey.toLower();
    while(it != m_indicationIcons.end()) {
        if ((*it)->getName().toLower() == iconName) {
            (*it)->setStatus(status);
            if (ClusterAppConfigure::getInstance()->isPlaySound()
                    && (ICON_TRUN_LEFT == iconName || ICON_TRUN_RIGHT == iconName
                    || SMALL_ICON_TRUN_RIGHT == iconName || SMALL_ICON_TRUN_RIGHT == iconName)) {
               //readme hase cluster-app do not play sound
               // static ClusterAppSoundPlayer player("/data/cluster/cluster-app_turnsignal.wav");
               // getTurnSignalsStatus() ? player.play() : player.stop();
            }
            break;
        }
        ++it;
    }
}

void ClusterAppValue::setLightStatus(const QString &lightKey, bool status)
{
    QList<ClusterAppIcon*>::iterator it = m_lightIcons.begin();
    QString iconName = lightKey.toLower();
    while(it != m_lightIcons.end()) {
        if ((*it)->getName().toLower() == iconName) {
            (*it)->setStatus(status);
            break;
        }
        ++it;
    }
}

void ClusterAppValue::setCardoorStatus(int status)
{
    m_cardoorStatus = status;
}

void ClusterAppValue::setCardoorStatus(int index, bool open)
{
   if (index < 0 || index > 3) {
       return;
   }

   if (open) {
        m_cardoorStatus = m_cardoorStatus | (1 << index);
   } else  {
        m_cardoorStatus = m_cardoorStatus & ~(1 << index);
   }
}

void ClusterAppValue::setTirePressure(int index, int value)
{
   if (index >= 0 && index  < 4) {
        m_tirePressureValues[index] = value;
   }
}

double ClusterAppValue::getSpeed() const
{
    return m_speed;
}

double ClusterAppValue::getRpm() const
{
    return m_rpm;
}

double ClusterAppValue::getFuel() const
{
    return m_fuel;
}

double ClusterAppValue::getTempareture() const
{
    return m_tempareture;
}

int ClusterAppValue::getAirTemp() const
{
    return m_airtemp;
}

int ClusterAppValue::getTotalMilage() const
{
    return m_totalMilage;
}

GearPos ClusterAppValue::getGearPosition() const
{
    return m_gearPosition;
}

QPixmap* ClusterAppValue::getIndicationIcon(const QString &indictionKey) const
{
    QPixmap* pix = nullptr;
    foreach (ClusterAppIcon *icon, m_indicationIcons) {
        if (icon->getName() == indictionKey) {
            pix = icon->getPixmap();
            break;
        }
    }

    return pix;
}

QPixmap* ClusterAppValue::getLightIcon(const QString &lightKey) const
{
    QPixmap* pix = nullptr;
    foreach (ClusterAppIcon *icon, m_lightIcons) {
        if (icon->getName() == lightKey) {
            pix = icon->getPixmap();
            break;
        }
    }

    return pix;
}


bool ClusterAppValue::getTurnSignalsStatus()
{
    bool ret = false;

    QList<ClusterAppIcon*>::iterator it = m_indicationIcons.begin();

    while (it != m_indicationIcons.end()) {
        if ((ICON_TRUN_LEFT == (*it)->getName().toLower() || ICON_TRUN_RIGHT == (*it)->getName().toLower()) && (*it)->getStatus()) {
            ret = true;
            break;
        }
        ++it;
    }

    return ret;
}

int ClusterAppValue::getCardoorStatus() const
{
    return m_cardoorStatus;
}

int ClusterAppValue::getTirePressure(int i) const
{
    return m_tirePressureValues[i];
}

void ClusterAppValue::initPixmaps()
{
    for (const QString& iconName : ICONNAME_List) {
        if (ICON_TRUN_LEFT == iconName || ICON_TRUN_RIGHT == iconName)
            m_indicationIcons.push_back(new ClusterAppFlashIcon(iconName));
        else
            m_indicationIcons.push_back(new ClusterAppIcon(iconName));
    }

    for (const QString& lightName : LIGHT_List) {
        m_lightIcons.push_back(new ClusterAppIcon(lightName));
    }

    for (const QString& iconName : SMALLICONNAME_List) {
        if (SMALL_ICON_TRUN_LEFT == iconName || SMALL_ICON_TRUN_RIGHT == iconName)
            m_indicationIcons.push_back(new ClusterAppFlashIcon(iconName));
        else
            m_indicationIcons.push_back(new ClusterAppIcon(iconName));
    }

    for (const QString& lightName : SMALLLIGHT_List) {
        m_lightIcons.push_back(new ClusterAppIcon(lightName));
    }
}


