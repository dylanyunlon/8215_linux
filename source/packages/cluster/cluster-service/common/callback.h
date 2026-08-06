#ifndef INTERFACE_H
#define INTERFACE_H
#include "chararray.h"

class ConnectionCallback {
public:
    virtual void onClientConnected(int fd) = 0;
};

class ITranportCallback
{
public:
    virtual void onTransportError(int fd) = 0;
};

class DataReceiveCallback
{
public:
    virtual void onReceiveData(int fd, unsigned char cmd, const char* data, unsigned int length) = 0;
    virtual void onReceiveReplyTimeOut(int fd, unsigned char cmd) = 0;
};

#endif // INTERFACE_H
