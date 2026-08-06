#-------------------------------------------------
#
# Project created by QtCreator 2015-09-25T09:20:40
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = mainapplication
TEMPLATE = app
DA_TOP = ../../../../source/packages/application


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

INCLUDEPATH += $$DA_TOP/apputils/ \
              $$DA_TOP/globalbus/include/ \
              $$DA_TOP/appobj/include/ \
			  $$DA_TOP/common/


LIBS +=-ldl -lapputils -lglobalbus -luniversal_utils
