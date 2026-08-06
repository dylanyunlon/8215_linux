#ifndef TRANSPORT_H
#define TRANSPORT_H
#include <list>
#include <atomic>
#include "chararray.h"
#include "callback.h"
using namespace std;
class ITransport
{
public:
    virtual ~ITransport() = default;
    virtual CharArray read() = 0;
    virtual int write(const CharArray& array) = 0;
    virtual void close() = 0;
    virtual void setTransportCallback(ITranportCallback *callback) {
        m_callback = callback;
    }

protected:
    ITranportCallback *m_callback = nullptr;
};

class Transport : public ITransport
{
public:
    Transport(int fd);
    CharArray read() override;
    int write(const CharArray& array) override;

private:
    int read(char *data, size_t len);
    int write(char *data, size_t len);
    void close();
    void notifyTransforError();

    std::atomic_bool m_run {false};
    int m_fd = -1;
};

#endif // TRANSPORT_H
