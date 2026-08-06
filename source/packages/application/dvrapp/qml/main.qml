/*
 * main.qml - DVR Application Entry Point (Refactored)
 *
 * Phase 1: Basic window structure with MainWindow component
 */

import QtQuick 2.5
import QtQuick.Window 2.2

Window {
    id: rootWindow
    visible: true
    title: "DVR Refactored"

    // Auto-detect screen size (supports 800x480 and 1024x600)
    width: Screen.width
    height: Screen.height

    // Set window background transparent to allow video layer to show through
    color: "transparent"

    // Fullscreen flag
    flags: Qt.Window | Qt.FramelessWindowHint
    visibility: Window.FullScreen

    Component.onCompleted: {
        console.log("[main.qml] Root window created:", width, "x", height)
        console.log("[main.qml] DVRBackend initialized:", dvrBackend !== null)
    }

    Component.onDestruction: {
        console.log("[main.qml] Root window being destroyed")
    }

    // Main application UI
    Loader {
        id: mainWindow
        anchors.fill: parent
        source: "MainWindow.qml"

        onLoaded: {
            console.log("[main.qml] MainWindow loaded successfully")
        }

        onStatusChanged: {
            if (status === Loader.Error) {
                console.log("[main.qml] ERROR: Failed to load MainWindow.qml")
            }
        }
    }
}
