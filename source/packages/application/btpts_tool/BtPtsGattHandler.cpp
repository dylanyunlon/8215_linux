#include "BtPtsGattHandler.h"
#include "clog.h"
#include "bluetoothgattutil.h"
#include <thread>

using namespace universal_utils;

static const char* TAG = "BtPtsGattHandler";
const map<string, HANDLE_FUN> BtPtsGattHandler::GATT_HANDLE_MAP = {
    {CMD_GATT_ADDSERVICE,     static_cast<HANDLE_FUN>(&BtPtsGattHandler::addService)},
    {CMD_GATT_REMOVESERVICE,     static_cast<HANDLE_FUN>(&BtPtsGattHandler::removeService)},
    {CMD_GATT_CLEARSERVICES,     static_cast<HANDLE_FUN>(&BtPtsGattHandler::clearServices)},
    {CMD_GATT_STARTADVERTISING,     static_cast<HANDLE_FUN>(&BtPtsGattHandler::startAdvertising)},
    {CMD_GATT_STOPADVERTISING,     static_cast<HANDLE_FUN>(&BtPtsGattHandler::stopAdvertising)},

};

BtPtsGattHandler::BtPtsGattHandler(IBluetoothClient *client)
    : BtPtsHandler(PROFILE_GATT)
    , m_gattInterface(NULL)
    , m_gattCallback(NULL)
{
    IBluetoothProfile *profile = NULL;
    client->getProfile(GATTSERVERPROFILENAME, &profile);
    m_gattInterface = dynamic_cast<IBluetoothGattServer*>(profile);
    EXPECT_NOT_NULL(m_gattInterface, "m_gattInterface");
    registerCallBack();
}

BtPtsGattHandler::~BtPtsGattHandler()
{
    deregisterCallBack();
}

const map<string, HANDLE_FUN>& BtPtsGattHandler::getHandleFunMap()
{
    return GATT_HANDLE_MAP;
}

void BtPtsGattHandler::registerCallBack()
{
    m_gattCallback = new BtPtsGATTCallBack(m_gattInterface);
    m_gattInterface->registerCallBack(*m_gattCallback);
}

void BtPtsGattHandler::deregisterCallBack()
{
    m_gattInterface->deregisterCallBack(*m_gattCallback);
    SAFE_DELETE(m_gattCallback);
}

int BtPtsGattHandler::startAdvertising()
{
    int ret = BTPTS_ERROR;

    bluetoothgattserver::AdvertiseSettings settings;
    settings.set_advertise_mode(bluetoothgattserver::AdvertiseSettings::ADVERTISE_MODE_BALANCED);
    settings.set_advertise_connectable(true);
    settings.set_advertise_timeout_millis(0);
    settings.set_advertise_tx_power_level(bluetoothgattserver::AdvertiseSettings::ADVERTISE_TX_POWER_MEDIUM);

    bluetoothgattserver::AdvertiseData advertiseData;
    advertiseData.set_include_device_name(false);
    advertiseData.add_service_uuids("0000FFF8-0000-1000-8000-00805F9B34FB");

    bluetoothgattserver::AdvertiseData scanResponse;
    scanResponse.set_include_device_name(true);
    auto *entry = scanResponse.add_service_data();
    entry->set_uuid("FFF8");
    string data;
    data.push_back(0x00);
    data.push_back(0x08);
    data.push_back(0x22);
    data.push_back(0x1E);
    data.push_back(0xAC);
    data.push_back(0xAD);
    entry->set_data(data.c_str(), data.size());

    ret = m_gattInterface->startAdvertising(settings, advertiseData, scanResponse);
    return  ret;
}

int BtPtsGattHandler::stopAdvertising()
{
    m_gattInterface->stopAdvertising();

    return BTPTS_OK;
}

int BtPtsGattHandler::addService()
{
    int ret = BTPTS_ERROR;

    bluetoothgattserver::BluetoothGattService service;
    service.set_muuid("0000FF10-0000-1000-8000-00805F9B34FB");
    service.set_mservicetype(bluetoothgattserver::BluetoothGattService::SERVICE_TYPE_PRIMARY);

    bluetoothgattserver::BluetoothGattCharacteristic* characteristic = service.add_mcharacteristics();
    characteristic->set_muuid("0000FF11-0000-1000-8000-00805F9B34FB");

    characteristic->set_mproperties(
        bluetoothgattserver::BluetoothGattCharacteristic::PROPERTY_READ |
        bluetoothgattserver::BluetoothGattCharacteristic::PROPERTY_WRITE |
        bluetoothgattserver::BluetoothGattCharacteristic::PROPERTY_NOTIFY
    );


    characteristic->set_mpermissions(
        bluetoothgattserver::BluetoothGattCharacteristic::PERMISSION_READ |
        bluetoothgattserver::BluetoothGattCharacteristic::PERMISSION_WRITE
    );

    m_gattInterface->addService(service);

    return ret;
}

int BtPtsGattHandler::removeService()
{
    int ret = BTPTS_ERROR;
    bluetoothgattserver::BluetoothGattService service;
    service.set_muuid("0000FF10-0000-1000-8000-00805F9B34FB");
    service.set_mservicetype(bluetoothgattserver::BluetoothGattService::SERVICE_TYPE_PRIMARY);

    bluetoothgattserver::BluetoothGattCharacteristic* characteristic = service.add_mcharacteristics();
    characteristic->set_muuid("0000FF11-0000-1000-8000-00805F9B34FB");

    characteristic->set_mproperties(
        bluetoothgattserver::BluetoothGattCharacteristic::PROPERTY_READ |
        bluetoothgattserver::BluetoothGattCharacteristic::PROPERTY_WRITE |
        bluetoothgattserver::BluetoothGattCharacteristic::PROPERTY_NOTIFY
    );

    characteristic->set_mpermissions(
        bluetoothgattserver::BluetoothGattCharacteristic::PERMISSION_READ |
        bluetoothgattserver::BluetoothGattCharacteristic::PERMISSION_WRITE
    );

    ret = m_gattInterface->removeService(service);
    EXPECT_EQ(BTPTS_OK, ret);

    return ret;
}

int BtPtsGattHandler::clearServices()
{
    m_gattInterface->clearServices();

    return BTPTS_OK;
}

template<> BtPtsGATTCallBack* Singleton<BtPtsGATTCallBack>::msSingleton = NULL;
int BtPtsGATTCallBack::onIndication(const CMessage &message)
{
    switch (message.what) {
        case GATT_IND_SERVICE_ADDED: {
            int status = message.getIntExtra(INT_GATT_STATUS);
            unsigned int arrayLength = 0;
            const unsigned char* dataArray = message.getArrayExtra(ARRAY_GATT_PROTOCOL_SERVICE, &arrayLength, nullptr);
            string binaryData(reinterpret_cast<const char*>(dataArray), arrayLength);
            bluetoothgattserver::BluetoothGattService service;
            service.ParseFromString(binaryData);

            PRINTF_TO_CONSOLE(TAG, "indication onServiceAdd status(%d) serviceUuid(%s)", status, service.muuid().c_str());
        }
        break;

        case GATT_IND_START_SUCESS: {
            PRINTF_TO_CONSOLE(TAG, "indication onAdvertisingStartSuccess");
        }
        break;

        case GATT_IND_START_FAILURE: {
            PRINTF_TO_CONSOLE(TAG, "indication onAdvertisingStartFailure");
        }
        break;

        case GATT_IND_CONNECTION_STATE_CHANGED: {
            string address = message.getStringExtra(STRING_ADDRESS);
            int state = message.getIntExtra(INT_CONNECTION_NEW_STATE);

            PRINTF_TO_CONSOLE(TAG, "indication onConnectionStateChanged(%s)(%d)", address.c_str(), state);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            std::vector<uint8_t> packet = {0x01, 0x00, 0x01, 0x00, 0x05};
            {
                bluetoothgattserver::BluetoothGattService service;
                service.set_muuid("0000FF10-0000-1000-8000-00805F9B34FB");
                service.set_mservicetype(bluetoothgattserver::BluetoothGattService::SERVICE_TYPE_PRIMARY);

                bluetoothgattserver::BluetoothGattCharacteristic* characteristic = service.add_mcharacteristics();
                characteristic->set_muuid("0000FF11-0000-1000-8000-00805F9B34FB");

                characteristic->set_mproperties(
                    bluetoothgattserver::BluetoothGattCharacteristic::PROPERTY_READ |
                    bluetoothgattserver::BluetoothGattCharacteristic::PROPERTY_NOTIFY
                );

                characteristic->set_mpermissions(
                    bluetoothgattserver::BluetoothGattCharacteristic::PERMISSION_READ |
                    bluetoothgattserver::BluetoothGattCharacteristic::PERMISSION_WRITE
                );

                BluetoothGattUtil::setValue(*characteristic, packet);
                BluetoothAddress deviceAddress(address);
                m_gattInterface->notifyCharacteristicChanged(deviceAddress, service, *characteristic, false);
            }
        }
        break;

        case GATT_IND_CHARACTERISTIC_WRITE_REQUEST: {
            string address = message.getStringExtra(STRING_ADDRESS);
            int requestId = message.getIntExtra(INT_GATT_REQUEST_ID);
            BluetoothAddress deviceAddress(address);

            PRINTF_TO_CONSOLE(TAG, "indication onCharacteristicWriteRequest(%s)(%d)", address.c_str(), requestId);

            std::vector<char> packet = {0x00};
            m_gattInterface->sendResponse(deviceAddress, requestId, 0, 0, packet);
        }
        break;

        default:
        break;
    }
    checkIndication(message.what);
    return 0;
}

