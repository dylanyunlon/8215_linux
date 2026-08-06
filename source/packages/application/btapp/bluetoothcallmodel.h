#ifndef BLUETOOTHCALLMODEL_H
#define BLUETOOTHCALLMODEL_H
#include <QAbstractListModel>
#include <QStringList>
#include "singleton.h"
#include "bluetoothhfclientcall.h"

#define STRING_CALL_TEXT                       "bluetoothCallText"
#define STRING_CALL_STATUS                     "bluetoothCallStatus"
#define STRING_CALL_ADDRESS                    "bluetoothCallAddress"

typedef enum {
       CALL_OUTGOING = 0,
       CALL_INCOMING,
       CALL_MISSING
    } E_CALL_LIST_ROLES;

class CBluetoothCall {
public:
    CBluetoothCall(const BluetoothHfClientCall &call, const QString &name);
    BluetoothHfClientCall getCall() const;
    int getCallRecordStatus();
    void setCallRecordStatus(int callRecordStatus);
    QString getName() const;

private:
    BluetoothHfClientCall m_call;
    QString m_name;
    int m_callRecordStatus;
};

class CBluetoothCallListModel : public QAbstractListModel , public universal_utils::Singleton<CBluetoothCallListModel>
{
    Q_OBJECT

public:
    CBluetoothCallListModel(QObject *parent = 0);
    ~CBluetoothCallListModel();
    void addCall(const CBluetoothCall &call);
    void clearCalls();
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
public slots:
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    bool hasWattingCall();
    bool hasIncomingCall();
    bool hasHoldCall();
    bool isAllHoldStatus();
    QVariant get(int index, int role) const;


protected:
    QHash<int, QByteArray> roleNames() const;

    typedef enum {
       CALL_NUMBER = 0,
       CALL_NAME,
       CALL_TEXT,
       CALL_STATUS,
       CALL_ADDRESS
    } E_CALL_LIST_ROLES;

private:
    QList<CBluetoothCall> m_calls;
};
#endif


