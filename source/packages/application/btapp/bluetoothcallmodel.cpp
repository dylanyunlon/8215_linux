#include "bluetoothcallmodel.h"
#include <QDebug>
#include <QStringList>
#include "bluetoothhfpcallback.h"
using namespace universal_utils;

#define UNKNOWN_CALLNAME CBluetoothCallListModel::trUtf8("Unknown")
#define STATUS_SPEAKING CBluetoothCallListModel::trUtf8("speaking")
#define STATUS_HOLD CBluetoothCallListModel::trUtf8("hold")
#define STATUS_OUTGOING CBluetoothCallListModel::trUtf8("outgoing")
#define STATUS_ALERTING CBluetoothCallListModel::trUtf8("alerting")
#define STATUS_INCOMING CBluetoothCallListModel::trUtf8("incoming")
#define STATUS_WAITING CBluetoothCallListModel::trUtf8("waiting")
#define STATUS_UNKONW CBluetoothCallListModel::trUtf8("unkonw status")

template<> CBluetoothCallListModel* Singleton<CBluetoothCallListModel>::msSingleton = NULL;
CBluetoothCall::CBluetoothCall(const BluetoothHfClientCall &call, const QString &name)
    : m_name("")
    , m_callRecordStatus(0)
{
    m_call = call;
    m_name = name;
}

BluetoothHfClientCall CBluetoothCall::getCall() const
{
    return m_call;
}

void CBluetoothCall::setCallRecordStatus(int callRecordStatus)
{
    m_callRecordStatus = callRecordStatus;
}

int CBluetoothCall::getCallRecordStatus()
{
    return m_callRecordStatus;
}

QString  CBluetoothCall::getName() const
{
    return m_name;
}

CBluetoothCallListModel::CBluetoothCallListModel(QObject *parent)
    : QAbstractListModel(parent)
{


}

CBluetoothCallListModel::~CBluetoothCallListModel()
{
    clearCalls();
}


void CBluetoothCallListModel::addCall(const CBluetoothCall &call)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_calls << call;
    endInsertRows();
}

void CBluetoothCallListModel::clearCalls()
{
    if (m_calls.size() > 0) {
    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    m_calls.clear();
    endRemoveRows();

   }
}
int CBluetoothCallListModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return m_calls.count();
}

QVariant CBluetoothCallListModel::data(const QModelIndex & index, int role) const
{
    return get(index.row(), role);
}


bool CBluetoothCallListModel::hasWattingCall()
{
    bool ret = false;
    for (CBluetoothCall call : m_calls) {
        if (call.getCall().getState() == HF_CALLSTATE_WAITING) {
            ret = true;
            break;
        }
    }

    return ret;
}

bool CBluetoothCallListModel::hasIncomingCall()
{
    bool ret = false;
    for (CBluetoothCall call : m_calls) {
        if (call.getCall().getState() == HF_CALLSTATE_INCOMING) {
            ret = true;
            break;
        }
    }

    return ret;
}

bool CBluetoothCallListModel::hasHoldCall()
{
    bool ret = false;
    for (CBluetoothCall call : m_calls) {
        if (call.getCall().getState() == HF_CALLSTATE_HELD) {
            ret = true;
            break;
        }
    }

    return ret;
}

bool CBluetoothCallListModel::isAllHoldStatus()
{
    if (m_calls.size() < 1)
        return false;

    bool ret = true;
    for (CBluetoothCall call : m_calls) {
        if (call.getCall().getState() != HF_CALLSTATE_HELD) {
            ret = false;
            break;
        }
    }

    return ret;
}


QHash<int, QByteArray> CBluetoothCallListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[CALL_TEXT]  = STRING_CALL_TEXT ;
    roles[CALL_STATUS] = STRING_CALL_STATUS;
    roles[CALL_ADDRESS] = STRING_CALL_ADDRESS;

    return roles;
}

QVariant CBluetoothCallListModel::get(int index, int role) const
{
    QVariant ret;

    if (index < 0 || index >= m_calls.count()) {
        return ret;
    }

    const CBluetoothCall &call = m_calls[index];
    const BluetoothHfClientCall &hfpCall = call.getCall();
    QString name = call.getName();

    switch (role) {
        case CALL_NUMBER:
            ret = QString::fromStdString(hfpCall.getNumber());
            break;

        case CALL_NAME:
            ret = name;
            break;

        case CALL_TEXT:
            ret = (name == UNKNOWN_CALLNAME ? QString::fromStdString(hfpCall.getNumber()) : name);
            break;

        case CALL_STATUS: {
            QStringList CALL_STATUS= {STATUS_SPEAKING, STATUS_HOLD, STATUS_OUTGOING, STATUS_ALERTING, STATUS_INCOMING, STATUS_WAITING};
            int status = hfpCall.getState();
            ret = (status < CALL_STATUS.size() ? CALL_STATUS[status] : STATUS_UNKONW);
            break;
        }

        case CALL_ADDRESS:
            ret = QString::fromStdString(hfpCall.getAddress().toString());
            break;
    }

    return ret;
}




