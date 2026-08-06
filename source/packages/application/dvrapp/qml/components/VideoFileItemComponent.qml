/*
 * VideoFileItemComponent.qml - Individual video file item
 * Displays single video file with thumbnail, filename, duration, and delete button
 */

import QtQuick 2.5
import QtQuick.Layouts 1.1

Rectangle {
    id: root
    color: "#2a2a2a"
    border.color: thumbnailMouseArea.pressed ? "#00aaff" : "#444444"
    border.width: 2
    radius: 5

    // Screen size detection
    readonly property bool isSmallScreen: parent.parent.parent.parent.width <= 800

    // Model properties (from parent's model)
    property string filePath: model.filePath || ""
    property string fileName: model.filename || ""
    property string duration: model.duration || ""
    property string fileSize: model.fileSize ? (model.fileSize / (1024 * 1024)).toFixed(1) + " MB" : ""
    property string cameraLabel: model.cameraLabel || "?"
    property string cameraColor: model.cameraColor || "#c8646464"

    // Signals
    signal clicked()
    signal deleteClicked()

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Thumbnail area
        Rectangle {
            id: thumbnailArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#4a6b8a"
            radius: 5

            // Camera type badge (top-right corner)
            Rectangle {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 3
                width: isSmallScreen ? 24 : 28
                height: isSmallScreen ? 24 : 28
                color: root.cameraColor
                radius: 3

                Text {
                    anchors.centerIn: parent
                    text: root.cameraLabel
                    color: "white"
                    font.pixelSize: isSmallScreen ? 14 : 16
                    font.bold: true
                }
            }

            // Duration badge (bottom-right corner)
            Rectangle {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.rightMargin: 4
                anchors.bottomMargin: 4
                width: durationText.width + 8
                height: 18
                color: "#b3000000"
                radius: 3
                visible: root.duration !== "" && root.duration !== "--:--" && root.duration !== "??:??"

                Text {
                    id: durationText
                    anchors.centerIn: parent
                    text: root.duration
                    color: "white"
                    font.pixelSize: 10
                    font.bold: true
                }
            }

            // Play icon (center)
            Column {
                anchors.centerIn: parent
                spacing: 8

                Image {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Math.min(thumbnailArea.width, thumbnailArea.height) * 0.55
                    height: width
                    source: "qrc:/images/file_play_icon.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: root.fileSize
                    color: "#cccccc"
                    font.pixelSize: 10
                    visible: root.fileSize !== ""
                }
            }

            MouseArea {
                id: thumbnailMouseArea
                anchors.fill: parent
                onClicked: root.clicked()
            }
        }

        // Bottom bar (filename and delete button)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: isSmallScreen ? 26 : 30
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                spacing: 5

                // Filename
                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: root.fileName
                    color: "white"
                    font.pixelSize: isSmallScreen ? 9 : 11
                    elide: Text.ElideMiddle
                    verticalAlignment: Text.AlignVCenter
                }

                // Delete button
                Rectangle {
                    Layout.preferredWidth: isSmallScreen ? 28 : 32
                    Layout.preferredHeight: isSmallScreen ? 28 : 32
                    color: deleteMouseArea.pressed ? "#555555" : "transparent"
                    radius: 3

                    Image {
                        anchors.centerIn: parent
                        width: parent.width * 0.75
                        height: parent.height * 0.75
                        source: "qrc:/images/file_delete.png"
                        fillMode: Image.PreserveAspectFit
                    }

                    MouseArea {
                        id: deleteMouseArea
                        anchors.fill: parent
                        onClicked: {
                            console.log("[VideoFileItem] Delete clicked:", root.fileName)
                            root.deleteClicked()
                        }
                    }
                }
            }
        }
    }
}
