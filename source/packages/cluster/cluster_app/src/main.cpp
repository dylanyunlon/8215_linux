
#include <QGuiApplication>
#include "clusterappwindow.h"
#include "clog.h"

#include <iostream>
#include <math.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>

#include <csignal>
#include <execinfo.h>
#include <pthread.h>
#include <fcntl.h>

#include <QApplication>
//#include <QQmlApplicationEngine>

//using universal_utils::CLog;

const static char *TAG = "cluter-app main";


void writeBootProf(const QString &text)
{
    int fd = open("/proc/bootprof", O_CREAT | O_RDWR | O_APPEND);
    if (fd < 0) {
        return;
    }
    write(fd, text.toStdString().c_str(), text.length());
    ::close(fd);
}

#ifndef WITH_SOAPP

int main(int argc, char *argv[])
{
    writeBootProf("start cluster main1 ");
    int ret = 0;
    QGuiApplication app(argc, argv);    //仪表App

    ClusterAppWindow window;
    window.show();

    return app.exec();
}

#else
static  ClusterAppWindow* window = NULL;

extern "C" int so_main(int argc, char *argv[], soapp_exit_handler exit_handler, void *handle, void *param)
{
    writeBootProf("start cluster main1 ");
    int ret = 0;
    if(!window) {
        window = new ClusterAppWindow();
        if(!window) {
            return(-1);
        }
    }
    //initListener(window, exit_handler, handle, param);
    window->show();
    return(0);

}
#endif



