/*
 * ToastComponent.qml - Toast message notification
 * Phase 1: Basic toast with auto-hide
 */

import QtQuick 2.5

Rectangle {
    id: root

    readonly property bool isSmallScreen: parent.width <= 800
    width: isSmallScreen ? 400 : 450
    height: isSmallScreen ? 70 : 80

    // Message type property (info or error)
    property string messageType: "info"

    color: "#cc000000"  // Semi-transparent black
    border.color: messageType === "error" ? "#ff4444" : "white"
    border.width: 2
    radius: 8

    visible: false
    opacity: 0

    // Message text
    Text {
        id: messageText
        anchors.centerIn: parent
        anchors.margins: 10
        color: messageType === "error" ? "#ff4444" : "white"
        font.pixelSize: isSmallScreen ? 16 : 18
        font.bold: messageType === "error"
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        width: parent.width - 20
    }

    // Fade in/out animation
    Behavior on opacity {
        NumberAnimation { duration: 300 }
    }

    // Auto-hide timer
    Timer {
        id: hideTimer
        interval: 3000
        onTriggered: hide()
    }

    // Show toast with message
    // type: "info" (default) or "error"
    function show(message, duration, type) {
        if (duration === undefined) {
            duration = 3000
        }
        if (type === undefined) {
            type = "info"
        }

        messageType = type
        messageText.text = message
        visible = true
        opacity = 1
        hideTimer.interval = duration
        hideTimer.restart()

        console.log("[Toast] Show:", message, "duration:", duration, "type:", type)
    }

    // Hide toast
    function hide() {
        opacity = 0
        hideTimer.stop()
        // Delay visibility change until fade out completes
        Qt.callLater(function() {
            if (opacity === 0) {
                visible = false
            }
        })
    }
}
