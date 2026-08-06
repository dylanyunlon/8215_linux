
TARGET = apputils
TEMPLATE = lib

SOURCES += cautoudpsocket.cpp cudpsocket.cpp serial.cpp utils.cpp messagequeue.cpp

LIBS += -luniversal_utils -lpthread -ldl 

DESTDIR = lib

#unix {
#    CONFIG += unversioned_libname
#    QMAKE_LFLAGS += -Wl,-soname,libapputils.so
#}
