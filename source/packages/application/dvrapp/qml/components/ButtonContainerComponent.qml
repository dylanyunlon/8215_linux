/*
 * ButtonContainerComponent.qml - Bottom button bar with 5 buttons
 * Phase 1: Buttons emit signals, no actual functionality yet
 */

import QtQuick 2.5
import QtQuick.Controls 1.4

Rectangle {
    id: root
    // Container itself is transparent; parent supplies the semi-transparent background
    color: "transparent"

    readonly property bool isSmallScreen: width <= 800
    readonly property int buttonWidth: isSmallScreen ? 75 : 120
    readonly property int buttonHeight: isSmallScreen ? 40 : 50

    // Property to receive current camera state from parent
    property bool isFrontCamera: true

    // Property to receive recording state from parent
    property bool isRecording: false

    // Signals for button clicks
    signal recordClicked()
    signal snapshotClicked()
    signal switchCameraClicked()
    signal settingsClicked()
    signal directoryClicked()

    // Calculate button spacing
    readonly property int totalButtons: 5
    readonly property int totalButtonWidth: totalButtons * buttonWidth
    readonly property int availableSpace: width - totalButtonWidth
    readonly property int spacing: availableSpace / (totalButtons + 1)

    // Button row - Qt Quick Controls 1.4 compatible (using Rectangle + MouseArea)
    // Match original DVR app: button container at (0, Y, fullWidth, height) - no margins
    Row {
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter  // Center the row horizontally
        spacing: root.spacing

        // 1. Record button - with icon (changes based on recording state)
        Rectangle {
            id: recordButton
            width: root.buttonWidth
            height: root.buttonHeight
            // Darker default background (#A0 = 62.7% opacity instead of #3C = 23.5%)
            // Red tint when recording is active
            color: {
                if (root.isRecording) {
                    return recordButtonArea.pressed ? "#B4FF6060" : (recordButtonArea.containsMouse ? "#8CFF6060" : "#A0FF6060")
                } else {
                    return recordButtonArea.pressed ? "#B4FFFFFF" : (recordButtonArea.containsMouse ? "#8CFFFFFF" : "#A0FFFFFF")
                }
            }
            radius: 6
            // Thicker default border (2px instead of 1px)
            border.color: {
                if (root.isRecording) {
                    return recordButtonArea.pressed ? "#DCFF6060" : (recordButtonArea.containsMouse ? "#C8FF6060" : "#B4FF6060")
                } else {
                    return recordButtonArea.pressed ? "#DCFFFFFF" : (recordButtonArea.containsMouse ? "#C8FFFFFF" : "#B4FFFFFF")
                }
            }
            border.width: recordButtonArea.pressed ? 3 : 2

            // Control opacity: blinking when recording, full opacity when not
            opacity: root.isRecording ? 1.0 : 1.0

            // Blinking animation when recording
            SequentialAnimation on opacity {
                id: blinkAnimation
                running: root.isRecording
                loops: Animation.Infinite

                NumberAnimation {
                    from: 1.0
                    to: 0.3
                    duration: 800
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    from: 0.3
                    to: 1.0
                    duration: 800
                    easing.type: Easing.InOutQuad
                }
            }

            // Reset opacity immediately when recording stops
            Connections {
                target: root
                onIsRecordingChanged: {
                    if (!root.isRecording) {
                        blinkAnimation.stop()
                        recordButton.opacity = 1.0
                    }
                }
            }

            Image {
                anchors.centerIn: parent
                width: parent.width * 0.95
                height: parent.height * 0.95
                // Change icon based on recording state
                source: root.isRecording ? "qrc:/images/record_stop.png" : "qrc:/images/record_start.png"
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                id: recordButtonArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: root.recordClicked()
            }
        }

        // 2. Snapshot button - with icon (disabled - feature not implemented)
        Rectangle {
            id: snapshotButton
            width: root.buttonWidth
            height: root.buttonHeight
            color: "#60808080"  // Gray semi-transparent (disabled state)
            radius: 6
            border.color: "#80808080"  // Gray border (disabled state)
            border.width: 2
            opacity: 0.5  // Reduced opacity to show disabled state

            Image {
                anchors.centerIn: parent
                width: parent.width * 0.95
                height: parent.height * 0.95
                source: "qrc:/images/snapshot.png"
                fillMode: Image.PreserveAspectFit
                opacity: 0.5  // Dim the icon to show disabled state
            }

            MouseArea {
                id: snapshotButtonArea
                anchors.fill: parent
                enabled: false  // Disable mouse interaction
                hoverEnabled: false
                onClicked: root.snapshotClicked()
            }
        }

        // 3. Switch camera button - with icon
        Rectangle {
            id: switchButton
            width: root.buttonWidth
            height: root.buttonHeight
            color: switchButtonArea.pressed ? "#B4FFFFFF" : (switchButtonArea.containsMouse ? "#8CFFFFFF" : "#A0FFFFFF")
            radius: 6
            border.color: switchButtonArea.pressed ? "#DCFFFFFF" : (switchButtonArea.containsMouse ? "#C8FFFFFF" : "#B4FFFFFF")
            border.width: switchButtonArea.pressed ? 3 : 2

            Image {
                anchors.centerIn: parent
                width: parent.width * 0.95
                height: parent.height * 0.95
                source: root.isFrontCamera ? "qrc:/images/camera_switch_front.png" : "qrc:/images/camera_switch_rear.png"
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                id: switchButtonArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: root.switchCameraClicked()
            }
        }

        // 4. Settings button - with icon
        Rectangle {
            id: settingsButton
            width: root.buttonWidth
            height: root.buttonHeight
            color: settingsButtonArea.pressed ? "#B4FFFFFF" : (settingsButtonArea.containsMouse ? "#8CFFFFFF" : "#A0FFFFFF")
            radius: 6
            border.color: settingsButtonArea.pressed ? "#DCFFFFFF" : (settingsButtonArea.containsMouse ? "#C8FFFFFF" : "#B4FFFFFF")
            border.width: settingsButtonArea.pressed ? 3 : 2

            Image {
                anchors.centerIn: parent
                width: parent.width * 0.95
                height: parent.height * 0.95
                source: "qrc:/images/settings.png"
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                id: settingsButtonArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: root.settingsClicked()
            }
        }

        // 5. Directory button - with icon
        Rectangle {
            id: directoryButton
            width: root.buttonWidth
            height: root.buttonHeight
            color: directoryButtonArea.pressed ? "#B4FFFFFF" : (directoryButtonArea.containsMouse ? "#8CFFFFFF" : "#A0FFFFFF")
            radius: 6
            border.color: directoryButtonArea.pressed ? "#DCFFFFFF" : (directoryButtonArea.containsMouse ? "#C8FFFFFF" : "#B4FFFFFF")
            border.width: directoryButtonArea.pressed ? 3 : 2

            Image {
                anchors.centerIn: parent
                width: parent.width * 0.95
                height: parent.height * 0.95
                source: "qrc:/images/directory.png"
                fillMode: Image.PreserveAspectFit
            }

            MouseArea {
                id: directoryButtonArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: root.directoryClicked()
            }
        }
    }
}
