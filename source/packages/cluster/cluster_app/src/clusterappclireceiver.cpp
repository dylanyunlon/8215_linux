#include "clusterappclireceiver.h"
#include "clusterappconstant.h"
#include "clusterappserviceadapter.h"
#include "clog.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SERV_ADDR "cluster.app.socket"
const static char *TAG = "ClusterAppCLIReceiver";
const static int CMD_MIN_SIZE = 2;

const static char * CMD_GEAR = "gear";
const static char * CMD_TOTAL_MILEAGE = "mileage";
const static char * CMD_AIR_TEMP = "airtemp";
const static char * CMD_TPMS = "tpms";
const static char * CMD_DOOR = "door";

const static char * CMD_SWITH_MODEL = "switch";
const static char * CMD_THEME = "theme";
const static char * CMD_MUSIC_CONTROL = "music";
const static char * CMD_CALL_CONTROL = "call";
const static char * CMD_SET_VOLUME = "volume";

const static double RPM_UINT = 1000;
const static double PERCENT_UNIT = 100;
//using universal_utils::CLog;
ClusterAppCLIReceiver::ClusterAppCLIReceiver()
    : m_value(ClusterAppValue::getInstance())
{
    connect(this, &ClusterAppCLIReceiver::sigContinusEventUpdate, this, &ClusterAppCLIReceiver::slotContinusEventUpdate);
    connect(this, &ClusterAppCLIReceiver::sigSwitchEventUpdate, this, &ClusterAppCLIReceiver::slotSwitchEventUpdate);
    start();
}
void ClusterAppCLIReceiver::continusEventCallback(char *data)
{
    QString receiveData(data);
    receiveData = receiveData.trimmed();

    QStringList dataList = receiveData.split(" ");
    double speeddelta  = 0;
    double rpmdelta = 0;
    double fueldelat = 0;
    double tempdelta = 0;

    const int UPDATE_FREQUENCY = 10;
    int times = dataList.last().toInt() / UPDATE_FREQUENCY;
    dataList.removeLast();

    times = times > 0 ? times : 1;

    double speed = m_value->getSpeed();
    double rpm = m_value->getRpm();
    double fuel = m_value->getFuel();
    double temp = m_value->getTempareture();

    if (dataList.length() >= CMD_MIN_SIZE) {
        speeddelta = (dataList.at(0).toInt() - speed) / times;
        rpmdelta = (dataList.at(1).toInt() / RPM_UINT  - rpm) / times;
    } else {
       // syslog(LOG_ERR, "%s","receiver continues data length error");
        return;
    }

    if (dataList.length() >= 3) {
        fueldelat = (dataList.at(2).toInt() / PERCENT_UNIT  - fuel) / times;
    }

    if (dataList.length() >= 4) {
        tempdelta= (dataList.at(3).toInt() / PERCENT_UNIT  - temp) / times;
    }

    for (int i = 1; i <= times; ++i) {
        emit sigContinusEventUpdate(speed + i * speeddelta, rpm + i * rpmdelta, fuel + i * fueldelat, temp + i * tempdelta);
        msleep(UPDATE_FREQUENCY);
    }

    if (dataList.length() >= 4) {
        emit sigContinusEventUpdate(dataList.at(0).toInt(), dataList.at(1).toInt() / RPM_UINT,  dataList.at(2).toInt() / PERCENT_UNIT, dataList.at(3).toInt() / PERCENT_UNIT);
    } else if (dataList.length() >= 3) {
        emit sigContinusEventUpdate(dataList.at(0).toInt(), dataList.at(1).toInt() / RPM_UINT,  dataList.at(2).toInt() / PERCENT_UNIT, temp);
    } else {
        emit sigContinusEventUpdate(dataList.at(0).toInt(), dataList.at(1).toInt() / RPM_UINT, fuel,  temp);
    }
}

void ClusterAppCLIReceiver::switchEventCallback(char *data)
{
    QString receiveData(data);

    receiveData = receiveData.trimmed();
    QStringList dataList = receiveData.split(" ");
    if (dataList.length() >= CMD_MIN_SIZE) {
        emit sigSwitchEventUpdate(dataList);
    } else {
       // syslog(LOG_ERR, "%s","receiver switch data length error");
    }
}

void ClusterAppCLIReceiver::run()
{
  //  UTILS_LOGD(TAG, "new cluster CLI enter");
    struct sockaddr_un servaddr, cliaddr;
    int socketfd = socket(AF_UNIX, SOCK_DGRAM, 0);  //1.socket

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    servaddr.sun_path[0] = '\0';
    strcpy(servaddr.sun_path + 1, SERV_ADDR);
    int len = offsetof(struct sockaddr_un, sun_path)
            + strlen(SERV_ADDR) + 1;
    unlink(SERV_ADDR);
    bind(socketfd, (struct sockaddr *)&servaddr, len); //2.bind

    char buf[128] = {0};
    int ret  = 0;
    while (1) {
        if ((ret = recvfrom(socketfd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr,
                            (socklen_t*)&len)) > 0) {
              //  UTILS_LOGD(TAG, buf);
                if (buf[0] >= '0' && buf[0] <= '9') {
                    continusEventCallback(buf);
                } else {
                    switchEventCallback(buf);
                }
                memset(&buf, 0, 128);
        } else {
           // syslog(LOG_ERR, "recvfrom error %d %s", ret, strerror(errno));
        }
    }

    close(socketfd);
    //UTILS_LOGD(TAG, "new cluster CLI leave");
}

void ClusterAppCLIReceiver::slotContinusEventUpdate(double speed, double rpm, double fuel, double temperature)
{
    m_value->setSpeed(speed);
    m_value->setRpm(rpm);
    m_value->setFuel(fuel);
    m_value->setTempareture(temperature);
}

void ClusterAppCLIReceiver::slotSwitchEventUpdate(QStringList data)
{
    QString cmd = data.at(0).toLower();
    int value = data.at(1).toInt();
    if (cmd == CMD_GEAR) {   //挡位
        m_value->setGearPosition((GearPos)(value));
    } else if (cmd == CMD_SWITH_MODEL) {   //变成测试模式
        emit sigSwitchTestMode(value);
    } else if (cmd == CMD_AIR_TEMP) {      //空调温度
        m_value->setAirTemp(value);
    } else if (cmd.startsWith(CMD_TPMS)) { //TPMS 胎压
        int index = cmd.mid(4).toInt() - 1;
        m_value->setTirePressure(index, value);
    } else if (cmd.startsWith(CMD_DOOR)) {  //车门状态
        int index = cmd.mid(4).toInt() -1;
        m_value->setCardoorStatus(index, value);
    } else if (cmd == CMD_TOTAL_MILEAGE) {  //里程数
        m_value->setTotalMilage(value);
    } else if (cmd == CMD_CALL_CONTROL) {   //电话接听、拨打、挂断
        emit sigCallControl(value);
    } else if (cmd == CMD_MUSIC_CONTROL) {
        emit sigMusicControl(value);        //播放或暂停
    }  else if (cmd == CMD_THEME) {
        if (value >= 1 && value <= 3)
            emit sigSwitchTheme(value - 1);        //播放或暂停
    } else if (cmd == CMD_SET_VOLUME) {      //设置声音
        if (value >= 0 && value <= 20) {
            emit sigSetVolume(value);
        }
    } else {
        m_value->setIndicationStatus(cmd, value); //指示灯状态
    }
}





