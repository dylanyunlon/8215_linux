#ifndef BTPTSGAPPHANDLER_H
#define BTPTSGAPPHANDLER_H
#include "BtPtsHandler.h"
#include "bluetoothgattindications.h"

class BtPtsGATTCallBack;
class BtPtsGattHandler : public BtPtsHandler
{
public:
    BtPtsGattHandler(IBluetoothClient *client);
    ~BtPtsGattHandler();
private:
    const map<string, HANDLE_FUN> &getHandleFunMap();
    void registerCallBack();
    void deregisterCallBack();

    int addService();
    int removeService();
    int clearServices();

    int startAdvertising();
    int stopAdvertising();

    static const map<string, HANDLE_FUN> GATT_HANDLE_MAP;
    IBluetoothGattServer *m_gattInterface;
    BtPtsGATTCallBack *m_gattCallback;
};

class BtPtsGATTCallBack : public BtPtsCallBack , public universal_utils::Singleton<BtPtsGATTCallBack>
{
public:
    BtPtsGATTCallBack(IBluetoothGattServer *gattInterface) {m_gattInterface = gattInterface;};
    int onIndication(const universal_utils::CMessage &message);
    IBluetoothGattServer *m_gattInterface;

};

#endif // BTPTSGAPPHANDLER_H
