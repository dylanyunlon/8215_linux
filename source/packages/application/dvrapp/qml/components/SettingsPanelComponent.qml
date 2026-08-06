/*
 * SettingsPanelComponent.qml - Settings overlay panel
 * Based on original DVR app settings: Resolution, Duration, USB Mode
 */

import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

Rectangle {
    id: root

    readonly property bool isSmallScreen: parent.width <= 800
    width: isSmallScreen ? 400 : 500
    height: isSmallScreen ? 280 : 360

    color: "#cc000000"  // Semi-transparent black
    border.color: "#666666"
    border.width: 2
    radius: 10

    visible: false  // Hidden by default

    // Connection to SettingsManager backend (set by parent)
    property var settingsManager: null

    // Settings state
    property int selectedResolution: 1  // 0: 1920x1080, 1: 1280x720
    property int selectedDuration: 0    // 0: 3 mins, 1: 5 mins, 2: 10 mins (default: 3 mins)

    // Toggle visibility
    function toggle() {
        visible = !visible

        // Start/stop performance monitoring based on visibility
        if (settingsManager) {
            if (visible) {
                settingsManager.startPerformanceMonitoring()
            } else {
                settingsManager.stopPerformanceMonitoring()
            }
        }
    }

    // Handle USB mode switch results
    Connections {
        target: settingsManager !== null ? settingsManager : null
        onUsbModeChanged: {
            console.log("[SettingsPanel] USB mode changed to:", newMode)
        }
        onUsbModeSwitchFailed: {
            console.log("[SettingsPanel] USB mode switch failed:", errorMessage)
            // TODO: Show error notification to user
        }
        onPerformanceStatsUpdated: {
            // Performance text automatically updates via binding
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12

        // Resolution setting
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: "Resolution:"
                color: "white"
                font.pixelSize: isSmallScreen ? 14 : 16
                font.bold: true
                Layout.preferredWidth: 120
            }

            ExclusiveGroup { id: resolutionGroup }

            RadioButton {
                text: "1920X1080"
                checked: selectedResolution === 0
                exclusiveGroup: resolutionGroup
                enabled: false  // Feature not implemented yet
                style: RadioButtonStyle {
                    label: Text {
                        text: control.text
                        color: control.enabled ? "white" : "#808080"  // Gray when disabled
                        font.pixelSize: isSmallScreen ? 12 : 14
                        font.bold: true
                    }
                    indicator: Rectangle {
                        implicitWidth: 24  // Increased from 18 for easier clicking
                        implicitHeight: 24
                        radius: 12
                        border.color: control.enabled ? "white" : "#808080"  // Gray when disabled
                        border.width: 2
                        color: "transparent"

                        Rectangle {
                            anchors.centerIn: parent
                            width: 14  // Increased from 10
                            height: 14
                            radius: 7
                            color: control.enabled ? "white" : "#808080"  // Gray when disabled
                            visible: control.checked
                        }
                    }
                    spacing: 8  // Add spacing between indicator and label
                }
                onClicked: {
                    selectedResolution = 0
                    console.log("[Settings] Resolution changed to 1920x1080")
                }
            }

            RadioButton {
                text: "1280X720"
                checked: selectedResolution === 1
                exclusiveGroup: resolutionGroup
                enabled: false  // Feature not implemented yet
                style: RadioButtonStyle {
                    label: Text {
                        text: control.text
                        color: control.enabled ? "white" : "#808080"  // Gray when disabled
                        font.pixelSize: isSmallScreen ? 12 : 14
                        font.bold: true
                    }
                    indicator: Rectangle {
                        implicitWidth: 24  // Increased from 18 for easier clicking
                        implicitHeight: 24
                        radius: 12
                        border.color: control.enabled ? "white" : "#808080"  // Gray when disabled
                        border.width: 2
                        color: "transparent"

                        Rectangle {
                            anchors.centerIn: parent
                            width: 14  // Increased from 10
                            height: 14
                            radius: 7
                            color: control.enabled ? "white" : "#808080"  // Gray when disabled
                            visible: control.checked
                        }
                    }
                    spacing: 8  // Add spacing between indicator and label
                }
                onClicked: {
                    selectedResolution = 1
                    console.log("[Settings] Resolution changed to 1280x720")
                }
            }
        }

        // Duration setting
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: "Duration:"
                color: "white"
                font.pixelSize: isSmallScreen ? 14 : 16
                font.bold: true
                Layout.preferredWidth: 120
            }

            ExclusiveGroup { id: durationGroup }

            RadioButton {
                text: "3 Mins"
                checked: selectedDuration === 0
                exclusiveGroup: durationGroup
                enabled: false  // Feature not implemented yet
                style: RadioButtonStyle {
                    label: Text {
                        text: control.text
                        color: control.enabled ? "white" : "#808080"  // Gray when disabled
                        font.pixelSize: isSmallScreen ? 12 : 14
                        font.bold: true
                    }
                    indicator: Rectangle {
                        implicitWidth: 24  // Increased from 18 for easier clicking
                        implicitHeight: 24
                        radius: 12
                        border.color: control.enabled ? "white" : "#808080"  // Gray when disabled
                        border.width: 2
                        color: "transparent"

                        Rectangle {
                            anchors.centerIn: parent
                            width: 14  // Increased from 10
                            height: 14
                            radius: 7
                            color: control.enabled ? "white" : "#808080"  // Gray when disabled
                            visible: control.checked
                        }
                    }
                    spacing: 8  // Add spacing between indicator and label
                }
                onClicked: {
                    selectedDuration = 0
                    console.log("[Settings] Duration changed to 3 mins")
                }
            }

            RadioButton {
                text: "5 Mins"
                checked: selectedDuration === 1
                exclusiveGroup: durationGroup
                enabled: false  // Feature not implemented yet
                style: RadioButtonStyle {
                    label: Text {
                        text: control.text
                        color: control.enabled ? "white" : "#808080"  // Gray when disabled
                        font.pixelSize: isSmallScreen ? 12 : 14
                        font.bold: true
                    }
                    indicator: Rectangle {
                        implicitWidth: 24  // Increased from 18 for easier clicking
                        implicitHeight: 24
                        radius: 12
                        border.color: control.enabled ? "white" : "#808080"  // Gray when disabled
                        border.width: 2
                        color: "transparent"

                        Rectangle {
                            anchors.centerIn: parent
                            width: 14  // Increased from 10
                            height: 14
                            radius: 7
                            color: control.enabled ? "white" : "#808080"  // Gray when disabled
                            visible: control.checked
                        }
                    }
                    spacing: 8  // Add spacing between indicator and label
                }
                onClicked: {
                    selectedDuration = 1
                    console.log("[Settings] Duration changed to 5 mins")
                }
            }

            RadioButton {
                text: "10 Mins"
                checked: selectedDuration === 2
                exclusiveGroup: durationGroup
                enabled: false  // Feature not implemented yet
                style: RadioButtonStyle {
                    label: Text {
                        text: control.text
                        color: control.enabled ? "white" : "#808080"  // Gray when disabled
                        font.pixelSize: isSmallScreen ? 12 : 14
                        font.bold: true
                    }
                    indicator: Rectangle {
                        implicitWidth: 24  // Increased from 18 for easier clicking
                        implicitHeight: 24
                        radius: 12
                        border.color: control.enabled ? "white" : "#808080"  // Gray when disabled
                        border.width: 2
                        color: "transparent"

                        Rectangle {
                            anchors.centerIn: parent
                            width: 14  // Increased from 10
                            height: 14
                            radius: 7
                            color: control.enabled ? "white" : "#808080"  // Gray when disabled
                            visible: control.checked
                        }
                    }
                    spacing: 8  // Add spacing between indicator and label
                }
                onClicked: {
                    selectedDuration = 2
                    console.log("[Settings] Duration changed to 10 mins")
                }
            }
        }

        // USB Mode setting
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: "USB Mode:"
                color: "white"
                font.pixelSize: isSmallScreen ? 14 : 16
                font.bold: true
                Layout.preferredWidth: 120
            }

            ExclusiveGroup { id: usbModeGroup }

            RadioButton {
                text: "Host"
                checked: settingsManager ? settingsManager.usbMode === 0 : false
                exclusiveGroup: usbModeGroup
                style: RadioButtonStyle {
                    label: Text {
                        text: control.text
                        color: "white"
                        font.pixelSize: isSmallScreen ? 12 : 14
                        font.bold: true
                    }
                    indicator: Rectangle {
                        implicitWidth: 24  // Increased from 18 for easier clicking
                        implicitHeight: 24
                        radius: 12
                        border.color: "white"
                        border.width: 2
                        color: "transparent"

                        Rectangle {
                            anchors.centerIn: parent
                            width: 14  // Increased from 10
                            height: 14
                            radius: 7
                            color: "white"
                            visible: control.checked
                        }
                    }
                    spacing: 8  // Add spacing between indicator and label
                }
                onClicked: {
                    console.log("[SettingsPanel] USB Host mode selected")
                    if (settingsManager) {
                        settingsManager.switchUsbMode(0)
                    }
                }
            }

            RadioButton {
                text: "Device"
                checked: settingsManager ? settingsManager.usbMode === 1 : false
                exclusiveGroup: usbModeGroup
                style: RadioButtonStyle {
                    label: Text {
                        text: control.text
                        color: "white"
                        font.pixelSize: isSmallScreen ? 12 : 14
                        font.bold: true
                    }
                    indicator: Rectangle {
                        implicitWidth: 24  // Increased from 18 for easier clicking
                        implicitHeight: 24
                        radius: 12
                        border.color: "white"
                        border.width: 2
                        color: "transparent"

                        Rectangle {
                            anchors.centerIn: parent
                            width: 14  // Increased from 10
                            height: 14
                            radius: 7
                            color: "white"
                            visible: control.checked
                        }
                    }
                    spacing: 8  // Add spacing between indicator and label
                }
                onClicked: {
                    console.log("[SettingsPanel] USB Device mode selected")
                    if (settingsManager) {
                        settingsManager.switchUsbMode(1)
                    }
                }
            }
        }

        // Performance Monitor Text Area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 120
            color: "#1a1a1a"
            border.color: "#555555"
            border.width: 1
            radius: 5

            ScrollView {
                anchors.fill: parent
                anchors.margins: 5

                // Custom scrollbar style for Qt 5.6.3
                style: ScrollViewStyle {
                    scrollBarBackground: Rectangle {
                        implicitWidth: 8
                        color: "transparent"
                    }

                    handle: Rectangle {
                        implicitWidth: 8
                        radius: 4
                        color: control.pressed ? "#888888" : "#555555"

                        // Smooth transition on hover
                        Behavior on color {
                            ColorAnimation { duration: 150 }
                        }
                    }

                    decrementControl: Rectangle {
                        visible: false
                    }

                    incrementControl: Rectangle {
                        visible: false
                    }
                }

                TextEdit {
                    id: performanceText
                    width: parent.width
                    color: "#00ff00"  // Green text like monitor
                    font.family: "Courier New"
                    font.pixelSize: isSmallScreen ? 11 : 12  // Increased from 10:11 to 11:12
                    readOnly: true
                    wrapMode: TextEdit.NoWrap
                    text: settingsManager ? settingsManager.performanceStats : "=== DVR Performance Monitor ===\nFPS: --\nGPU: N/A\nDVR CPU: --%\nTotal CPU: --%\nDVR Memory: -- MB\nTotal Memory: -- MB\nTime: 00:00:00"
                    selectByMouse: true
                }
            }
        }
    }
}
