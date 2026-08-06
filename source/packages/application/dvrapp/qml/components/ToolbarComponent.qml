/*
 * ToolbarComponent.qml - Top toolbar with title and exit button
 */

import QtQuick 2.5
import QtQuick.Controls 1.4

Rectangle {
    id: root
    color: "transparent"

    readonly property bool isSmallScreen: width <= 800

    // Signals
    signal homeClicked()
    signal exitClicked()

    // Home button (left side) - with icon
    // Note: Original DVR app has 20px left margin for toolbar content
    Rectangle {
        id: homeButton
        anchors.left: parent.left
        anchors.leftMargin: isSmallScreen ? 5 : 10
        anchors.verticalCenter: parent.verticalCenter
        width: isSmallScreen ? 50 : 60
        height: parent.height

        // Highlight on press with white translucent overlay (matching original DVR app)
        color: homeButtonArea.pressed ? "#64FFFFFF" : "transparent"  // rgba(255, 255, 255, 100)
        // Add visible border for better button recognition
        border.color: homeButtonArea.pressed ? "#96FFFFFF" : "#60FFFFFF"  // rgba(255, 255, 255, 150/96)
        border.width: 1
        radius: 4

        Image {
            anchors.centerIn: parent
            width: parent.width * 0.7
            height: parent.height * 0.7
            source: "qrc:/images/home.png"
            fillMode: Image.PreserveAspectFit
        }

        MouseArea {
            id: homeButtonArea
            anchors.fill: parent
            onClicked: {
                console.log("[Toolbar] Home button clicked")
                root.homeClicked()
            }
        }
    }

    // Exit button (right side) - with icon
    Rectangle {
        id: exitButton
        anchors.right: parent.right
        anchors.rightMargin: isSmallScreen ? 5 : 10
        anchors.verticalCenter: parent.verticalCenter
        width: isSmallScreen ? 50 : 60
        height: parent.height

        // Highlight on press with white translucent overlay (matching original DVR app)
        color: exitButtonArea.pressed ? "#64FFFFFF" : "transparent"  // rgba(255, 255, 255, 100)
        // Add visible border for better button recognition
        border.color: exitButtonArea.pressed ? "#96FFFFFF" : "#60FFFFFF"  // rgba(255, 255, 255, 150/96)
        border.width: 1
        radius: 4

        Image {
            anchors.centerIn: parent
            width: parent.width * 0.7
            height: parent.height * 0.7
            source: "qrc:/images/back.png"
            fillMode: Image.PreserveAspectFit
        }

        MouseArea {
            id: exitButtonArea
            anchors.fill: parent
            onClicked: {
                console.log("[Toolbar] Exit button clicked")
                root.exitClicked()
            }
        }
    }
}
