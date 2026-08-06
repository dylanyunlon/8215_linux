#ifndef CPROCESS_H
#define CPROCESS_H
#include <clusterthreadproc.h>
#include <stddef.h>
#include <unistd.h>
#include <string>
using namespace std;

class CProcess
{
public:
    class ProcessDeathListener {
        public:
            virtual void onProcessDeath(int code) = 0;
    };

public:
    CProcess();
    virtual ~CProcess();
    int start(char* const argv[]);
    int kill();
    pid_t getPid() const;
    void regDeathListener(ProcessDeathListener *listener);

protected:
    bool waitProc();

private:
    ProcessDeathListener *m_listener = nullptr;
    ThreadProc *m_waitThread = nullptr;
    pid_t m_pid = 0;
};

#endif // CPROCESS_H
