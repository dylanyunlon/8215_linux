#include <QApplication>
#include <QQmlApplicationEngine>

#include "managerservice.h"

// int main(int argc, char *argv[])
// {
// #if USE_SYSTEM_LOG
//     OPENLOG("[AAApp]ManagerService");
// #endif
//     universal_utils::CLog::setLogLevel(UNIVERSAL_UTILE_LOG_LEVEL_DEBUG);
//     QApplication app(argc, argv);
//     QQmlApplicationEngine engine;
//
//     ManagerService::getInstance()->onStart(nullptr, nullptr, nullptr, &app, &engine);
//
//
//     return app.exec();
// }

static QQmlApplicationEngine engine;

extern "C" int so_main(int argc, char *argv[], soapp_exit_handler exit_handler, void *handle, void *param)
{
    universal_utils::CLog::setLogLevel(UNIVERSAL_UTILE_LOG_LEVEL_DEBUG);
    LOGD("[AAApp]ManagerService", "so_main called");
    ManagerService::getInstance()->onStart(nullptr, nullptr, nullptr, nullptr, &engine);

    return(0);

}
