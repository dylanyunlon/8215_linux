#include "cprocess.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/prctl.h>

CProcess::CProcess()
{
    m_waitThread = new ThreadProc(std::bind(&CProcess::waitProc, this));
    m_waitThread->threadStart();
}

CProcess::~CProcess()
{
    if (m_waitThread) {
        m_waitThread->threadStop();
        delete m_waitThread;
        m_waitThread = nullptr;
    }
}

int CProcess::start(char* const argv[])
{
    int ret = 0;
    pid_t pid = ::fork();
    if (0 == pid) { //child
        char logBuff[255] = {0};
        write(1, logBuff, snprintf(logBuff, sizeof(logBuff), "CProcess child start: %d\n", getPid()));
        prctl(PR_SET_PDEATHSIG, SIGKILL); //when the father process dies, child process exits
        ret = execv(argv[0], argv);
        if (ret < 0) {
            write(1, logBuff, snprintf(logBuff, sizeof(logBuff), "CProcess start error: %s\n", strerror(errno)));
            ::exit(ret);
            return ret;
        }
    } else if (pid > 0) {
        m_pid = pid;
        if (m_waitThread) {
            m_waitThread->triggerProc();
        }
        return pid;
    } else {
        //error
        return -1;
    }
    return ret;
}

int CProcess::kill()
{
    if (0 != m_pid) {
        ::kill(m_pid, SIGKILL);
        m_pid = 0;
    }

    return 0;
}

pid_t CProcess::getPid() const
{
    return m_pid;
}

void CProcess::regDeathListener(CProcess::ProcessDeathListener *listener)
{
    m_listener = listener;
}

bool CProcess::waitProc()
{
    int status = 0;
    if (m_pid == 0) {
        return false;
    }

    while (true) {
        int ret = ::waitpid(m_pid, &status, 0);
        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            }
        }
        break;
    }

    if (NULL != m_listener) {
        m_listener->onProcessDeath(status);
    }

    return true;
}
