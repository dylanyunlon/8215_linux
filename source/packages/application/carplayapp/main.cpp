#include <QApplication>

#include "managerservice.h"

// int main(int argc, char *argv[])
// {
//     universal_utils::CLog::setLogLevel(UNIVERSAL_UTILE_LOG_LEVEL_DEBUG);
//     //QApplication app(argc, argv);
//     QQmlApplicationEngine engine;

//     ManagerService::getInstance()->onStart(nullptr, nullptr, nullptr, nullptr, &engine);


//     return (0);
// }

static QQmlApplicationEngine engine;
extern "C" int so_main(int argc, char *argv[], soapp_exit_handler exit_handler, void *handle, void *param)
{
    universal_utils::CLog::setLogLevel(UNIVERSAL_UTILE_LOG_LEVEL_DEBUG);
    LOGD("jianjie", "teset1234");
    ManagerService::getInstance()->onStart(nullptr, nullptr, nullptr, nullptr, &engine);

    return(0);

}
