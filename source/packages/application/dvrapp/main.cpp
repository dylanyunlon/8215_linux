/*
 * Copyright (c) 2025 AutoChips Inc.
 *
 * DVR Application - Main entry point (Refactored)
 * Dynamic library mode only - loaded by AppManager
 */

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "backend/dvrbackend.h"
#include "backend/settingsmanager.h"
#include "backend/filemanager.h"
#include "backend/playbackmanager.h"
#include "backend/dvrqobjlistener.h"
#include "backend/dvrlog.h"

static const char* TAG = "DVR_APP][Main";

typedef void (*soapp_exit_handler)(void *handle, void *arg);

extern "C" int so_main(int argc, char *argv[], soapp_exit_handler exit_handler, void *handle, void *param)
{
    int fd;
    printf("so_main enter\n");

    // Redirect stdout/stderr to log file
    fd = open("/data/atclog/dvrlog.txt", O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd >= 0) {
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);
        close(fd);
    }

    LOGI(TAG, "========== so_main started, DVR Refactor QML ==========");

    // Create and initialize DVRBackend BEFORE QML loads
    // This ensures settingsManager is ready when QML components access it
    LOGI(TAG, "Creating DVRBackend...");
    DVRBackend *backend = new DVRBackend();
    if (!backend) {
        LOGE(TAG, "Failed to create DVRBackend");
        return -1;
    }
    LOGI(TAG, "DVRBackend instance created at %p", backend);

    // Initialize backend (creates SettingsManager and other managers)
    // Note: This does NOT require QML window - only initListener() needs the window
    LOGI(TAG, "Initializing DVRBackend (creates managers including SettingsManager)...");
    if (!backend->initialize()) {
        LOGE(TAG, "Failed to initialize DVRBackend");
        delete backend;
        return -1;
    }
    LOGI(TAG, "DVRBackend initialized successfully (SettingsManager ready for QML)");

    // Create QML engine
    LOGI(TAG, "Creating QQmlApplicationEngine...");
    QQmlApplicationEngine *engine = new QQmlApplicationEngine();
    if (!engine) {
        LOGE(TAG, "Failed to create QQmlApplicationEngine");
        delete backend;
        return -1;
    }
    LOGI(TAG, "QQmlApplicationEngine created at %p", engine);

    // Register SettingsManager type for Qt meta-object system
    qmlRegisterUncreatableType<SettingsManager>("DVR.Backend", 1, 0, "SettingsManager",
                                                 "SettingsManager is managed by DVRBackend");
    LOGI(TAG, "SettingsManager type registered to Qt meta-object system");

    // Register FileManager type for Qt meta-object system
    qmlRegisterUncreatableType<FileManager>("DVR.Backend", 1, 0, "FileManager",
                                            "FileManager is managed by DVRBackend");
    LOGI(TAG, "FileManager type registered to Qt meta-object system");

    // Register PlaybackManager type for Qt meta-object system
    qmlRegisterUncreatableType<PlaybackManager>("DVR.Backend", 1, 0, "PlaybackManager",
                                                 "PlaybackManager is managed by DVRBackend");
    LOGI(TAG, "PlaybackManager type registered to Qt meta-object system");

    // Expose backend to QML (SettingsManager accessible via backend.settingsManager)
    engine->rootContext()->setContextProperty("dvrBackend", backend);
    LOGI(TAG, "DVRBackend exposed to QML context (includes settingsManager property)");

    // Load QML
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    LOGI(TAG, "Loading QML from: %s", url.toString().toStdString().c_str());

    engine->load(url);
    LOGI(TAG, "QML load() call completed");

    if (engine->rootObjects().isEmpty()) {
        LOGE(TAG, "No root objects created from QML - QML load failed!");
        LOGE(TAG, "Possible reasons:");
        LOGE(TAG, "  1. resources.qrc not compiled into the .so");
        LOGE(TAG, "  2. qml/main.qml missing in resources");
        LOGE(TAG, "  3. QML syntax error");
        delete engine;
        delete backend;
        return -1;
    }
    LOGI(TAG, "QML loaded successfully, %d root objects created", engine->rootObjects().count());

    // Get window for listener initialization
    QObject *rootObj = engine->rootObjects().first();
    LOGI(TAG, "Root object: %p, type: %s",
         rootObj, rootObj ? rootObj->metaObject()->className() : "null");

    QQuickWindow *mainWindow = qobject_cast<QQuickWindow*>(rootObj);
    if (!mainWindow) {
        LOGE(TAG, "Root object is not a QQuickWindow!");
        LOGE(TAG, "Expected: QQuickWindow, Got: %s",
             rootObj ? rootObj->metaObject()->className() : "null");
        delete engine;
        delete backend;
        return -1;
    }
    LOGI(TAG, "Main QQuickWindow obtained at %p", mainWindow);
    LOGI(TAG, "Window geometry: %dx%d at (%d,%d)",
         mainWindow->width(), mainWindow->height(),
         mainWindow->x(), mainWindow->y());

    // Initialize listener with window object
    DVRQObjListener *listener = backend->getQObjListener();
    if (!listener) {
        LOGE(TAG, "Failed to get DVRQObjListener from backend");
        delete engine;
        delete backend;
        return -1;
    }
    LOGI(TAG, "DVRQObjListener obtained at %p", listener);

    // CRITICAL:
    // 1. For QQuickWindow, pass the window directly (not windowHandle())
    // 2. DO NOT call show() here - let DVRQObjListener::doShowFrontUI() handle it
    // 3. AppManager will call doShowFront() -> doShowFrontUI() to show the window
    LOGI(TAG, "Calling initListener with window=%p, exit_handler=%p, handle=%p, param=%p",
         mainWindow, exit_handler, handle, param);

    bool initResult = listener->initListener(mainWindow, exit_handler, handle, param);
    if (!initResult) {
        LOGE(TAG, "Failed to initialize CQObjListener");
        delete engine;
        delete backend;
        return -1;
    }
    LOGI(TAG, "CQObjListener initialized successfully");

    LOGI(TAG, "========== so_main initialization complete ==========");
    LOGI(TAG, "DVR Application registered with APPID_DVR (19)");
    LOGI(TAG, "Backend: %p, Engine: %p, Listener: %p, Window: %p",
         backend, engine, listener, mainWindow);
    LOGI(TAG, "NOT calling show() - waiting for AppManager to call doShowFrontUI()");

    return 0;
}
