TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
TARGET = btpts_tool
DEFINES += WITH_SOAPP

QMAKE_CXXFLAGS += -std=c++11

#DEPENDS = "atc-binarys"

SOURCES += main.cpp \
    BtPtsCmdParser.cpp \
    BtPtsAvrcpHandler.cpp \
    BtPtsCmdConsole.cpp \
    BtPtsHfpHandler.cpp \
    BtPtsPbapHandler.cpp \
    BtPtsGapHandler.cpp \
    BtPtsHidHandler.cpp \
    BtPtsGattHandler.cpp \
    BtPtsHandler.cpp \
    BtPtsHandlerManager.cpp

HEADERS += \
    BtPtsConstants.h \
    BtPtsCmdParser.h \
    BtPtsAvrcpHandler.h \
    BtPtsCmdConsole.h \
    BtPtsHfpHandler.h \
    BtPtsPbapHandler.h \
    BtPtsGapHandler.h \
    BtPtsHidHandler.h \
    BtPtsGattHandler.h \
    BtPtsHandler.h \
    BtPtsHandlerManager.h

message(Current path: $$PWD)

INCLUDEPATH += ../utils/
INCLUDEPATH += ../../../../source/packages/connectivity/bluetooth/include/
INCLUDEPATH += ../../../../source/packages/connectivity/universal_utils/include/

LIBS += -lpthread -ldl -lbluetoothcommon -lbluetoothclient -luniversal_utils -ltinyxml2 -lsqlite3 -lprotobuf