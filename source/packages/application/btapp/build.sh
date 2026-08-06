#!/bin/bash

QTDIR=$OSS_LIB_TOP/gnuarm-4.8.2_vfp/qt/5.5.0/usr
QMAKE=$QTDIR/bin/qmake

$QMAKE CONFIG+=release -spec $QTDIR/mkspecs/devices/linux-arm-atc-8317-g++
make
