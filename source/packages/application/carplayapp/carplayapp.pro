TARGET = carplayapp
TEMPLATE = lib
DESTDIR = lib

DEFINES += WITH_SOAPP

QT += qml quick widgets
CONFIG += c++11

RESOURCES += qml.qrc

DA_TOP = ../../../../source/packages/application
CARLINCK_TOP = ../../../../source/packages/connectivity/carlink
ATCSURFACE_TOP = ../../../../source/packages/graphics/surface/surface/

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.

SOURCES += main.cpp \
    managerservice.cpp \
    $$DA_TOP/common/qobjlistener.cpp \

INCLUDEPATH += $$DA_TOP/apputils/
INCLUDEPATH += $$DA_TOP/common/
INCLUDEPATH += $$DA_TOP/appobj/include/
INCLUDEPATH += $$DA_TOP/globalbus/include/
INCLUDEPATH += $$CARLINCK_TOP/interfaces/carplay/
INCLUDEPATH += $$CARLINCK_TOP/interfaces/accessory/
INCLUDEPATH += $$CARLINCK_TOP/../bluetooth/include
INCLUDEPATH += $$CARLINCK_TOP/../wifi/public/include/wifi/client/
INCLUDEPATH += $$ATCSURFACE_TOP/include/


HEADERS += \
    managerservice.h \
    $$DA_TOP/common/qobjlistener.h \
    $$CARLINCK_TOP/interfaces/carplay/carplaycallback.h \
    $$CARLINCK_TOP/interfaces/carplay/icarplayclient.h \
    $$CARLINCK_TOP/carlink/interfaces/accessory/iaccessoryinfo.h \
    $$CARLINCK_TOP/carlink/interfaces/accessory/iaccessoryinfocallback.h \
    $$CARLINCK_TOP/carlink/interfaces/carplay/carplaymodestate.h


LIBS += -lglobalbus -lapputils -lappobj -luniversal_utils -lcarplayclient -laccessoryinfo -lcarplaycommon -lcarlinkutils -lsurface_atc -lbluetoothcommon -lbluetoothclient -lwificommon -lwificlient
