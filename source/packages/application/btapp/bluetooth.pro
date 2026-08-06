TARGET = btapp
TEMPLATE = lib
DEFINES += WITH_SOAPP
CONFIG += unversioned_libname

QT += qml quick widgets

SOURCES += main.cpp \
    bluetoothapplication.cpp \
    bluetoothcallpage.cpp \
    bluetoothdialpage.cpp \
    bluetoothcallrecordsbookspage.cpp \
    bluetoothmusicpage.cpp \
    bluetoothpairrecordspage.cpp \
    bluetoothsettingpage.cpp \
    bluetoothgapcallback.cpp \
    bluetoothhfpcallback.cpp \
    bluetoothpbapcallback.cpp \
    bluetoothavrcpcallback.cpp \
    bluetootha2dpcallback.cpp \
    bluetoothcallrecordsmodel.cpp \
    bluetoothphonebookmodel.cpp \
    bluetoothavailabledevicemodel.cpp \
    bluetoothpaireddevicemodel.cpp \
    bluetoothphoneapplication.cpp \
    bluetoothfilesync.cpp \
    bluetoothhidcallback.cpp \
    bluetoothcallmodel.cpp \
    bluetoothutils.cpp \
    ../../../../source/packages/application/common/qobjlistener.cpp \

HEADERS += \
    bluetoothapplication.h \
    bluetoothcallpage.h \
    bluetoothdialpage.h \
    bluetoothcallrecordsbookspage.h \
    bluetoothmusicpage.h \
    bluetoothpairrecordspage.h \
    bluetoothsettingpage.h \
    bluetoothgapcallback.h \
    bluetoothhfpcallback.h \
    bluetoothpbapcallback.h \
    bluetoothavrcpcallback.h \
    bluetootha2dpcallback.h \
    bluetoothcallrecordsmodel.h \
    bluetoothphonebookmodel.h \
    bluetoothavailabledevicemodel.h \
    bluetoothpaireddevicemodel.h \
    bluetoothphoneapplication.h \
    bluetoothfilesync.h \
    bluetoothhidcallback.h \
    bluetoothcallmodel.h \
    bluetoothutils.h \
    ../../../../source/packages/application/common/qobjlistener.h \

message(Current path: $$PWD)

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

INCLUDEPATH += ../../../../source/packages/application/apputils/
INCLUDEPATH += ../../../../source/packages/application/common/
INCLUDEPATH += ../../../../source/packages/application/appobj/include/
INCLUDEPATH += ../../../../source/packages/application/globalbus/include/
INCLUDEPATH += ../../../../source/packages/connectivity/bluetooth/include/
INCLUDEPATH += ../../../../source/packages/connectivity/universal_utils/include/

LIBS += -lbluetoothcommon -lbluetoothclient -lglobalbus -lappobj  -luniversal_utils -ltinyxml2 -lsqlite3
