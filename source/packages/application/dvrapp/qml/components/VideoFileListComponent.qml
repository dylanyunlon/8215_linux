/*
 * VideoFileListComponent.qml - Video file list view
 * Main container component for browsing recorded video files
 * 
 * Features:
 * - 6-column grid layout with responsive sizing
 * - Pull-to-refresh gesture
 * - Loading animation during file scanning
 * - Empty state display
 * - Delete confirmation dialog
 * - Toast notifications for errors/success
 */

import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2

Rectangle {
    id: root
    anchors.fill: parent
    color: "#000000"

    // Screen size detection
    readonly property bool isSmallScreen: width <= 800

    // Grid layout properties
    readonly property int gridColumns: 6
    readonly property int itemSpacing: 10
    // Calculate item width: (total width - (columns+1) gaps) / columns
    readonly property int itemWidth: (width - itemSpacing * (gridColumns + 1)) / gridColumns
    readonly property int itemHeight: itemWidth * 0.75

    // Toolbar height
    readonly property int toolbarHeight: isSmallScreen ? 35 : 45

    // Properties bound from parent
    property var fileManager: null

    // Signals
    signal backClicked()
    signal videoFileSelected(string filePath)

    // File list model
    ListModel {
        id: fileListModel
    }

    // Main layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Toolbar
        Rectangle {
            id: toolbar
            Layout.fillWidth: true
            Layout.preferredHeight: root.toolbarHeight
            color: "#2c2c2c"

            // Title (centered)
            Text {
                anchors.centerIn: parent
                text: qsTr("Recorded Files")
                color: "white"
                font.pixelSize: isSmallScreen ? 14 : 16
                font.bold: true
            }

            // Back button (right side)
            Rectangle {
                id: backButton
                anchors.right: parent.right
                anchors.rightMargin: isSmallScreen ? 5 : 10
                anchors.verticalCenter: parent.verticalCenter
                width: isSmallScreen ? 50 : 60
                height: parent.height
                color: backButtonArea.pressed ? "#555555" : "transparent"
                radius: 0

                Image {
                    anchors.centerIn: parent
                    width: parent.width * 0.7
                    height: parent.height * 0.7
                    source: "qrc:/images/back.png"
                    fillMode: Image.PreserveAspectFit
                }

                MouseArea {
                    id: backButtonArea
                    anchors.fill: parent
                    onClicked: root.backClicked()
                }
            }
        }

        // ScrollView for file grid (GridView handles its own scrolling)
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // Grid container
            Item {
                id: gridContainer
                width: parent.width
                height: parent.height

                // Pull-to-refresh indicator (green circle with rotating arrow icon)
                Rectangle {
                    id: refreshIndicator
                    anchors.horizontalCenter: parent.horizontalCenter
                    // Position just below toolbar (toolbar height + 5px spacing)
                    y: Math.max(root.toolbarHeight + 5, root.toolbarHeight + 5 + gridView.contentY)
                    width: 40
                    height: 40
                    radius: 20
                    color: "#4caf50"
                    opacity: gridView.contentY < -80 ? 1.0 : 0.6
                    visible: gridView.contentY < -30 && !loadingOverlay.visible
                    z: 100

                    // Rotating line indicator for visual feedback
                    Rectangle {
                        anchors.centerIn: parent
                        width: 3
                        height: parent.height * 0.5
                        color: "white"
                        radius: 1.5
                        rotation: -gridView.contentY * 3
                        Behavior on rotation {
                            NumberAnimation { duration: 100 }
                        }
                    }
                }

                // GridView for performance optimization
                GridView {
                    id: gridView
                    anchors.fill: parent
                    anchors.leftMargin: root.itemSpacing
                    anchors.topMargin: root.itemSpacing

                    cellWidth: root.itemWidth + root.itemSpacing
                    cellHeight: root.itemHeight + root.itemSpacing

                    model: fileListModel

                    // Performance optimizations
                    cacheBuffer: Math.max(0, height * 2)  // Prevent negative value
                    clip: true

                    // Kinetic scrolling
                    flickDeceleration: 1500
                    maximumFlickVelocity: 2500
                    boundsBehavior: Flickable.DragAndOvershootBounds
                    
                    // Track if we've pulled far enough to trigger refresh
                    property bool pullRefreshTriggered: false
                    
                    // Real-time contentY monitoring for pull-to-refresh
                    onContentYChanged: {
                        // Detect if pulled beyond threshold
                        if (contentY < -80 && !pullRefreshTriggered && !loadingOverlay.visible) {
                            pullRefreshTriggered = true
                        }
                    }
                    
                    // When user releases, check if refresh was triggered
                    onDraggingChanged: {
                        if (!dragging && pullRefreshTriggered) {
                            triggerRefresh()
                            pullRefreshTriggered = false
                        } else if (!dragging) {
                            pullRefreshTriggered = false
                        }
                    }

                    delegate: VideoFileItemComponent {
                        width: root.itemWidth
                        height: root.itemHeight

                        onClicked: {
                            console.log("[VideoFileList] Video selected:", model.filePath)
                            root.videoFileSelected(model.filePath)
                        }

                        onDeleteClicked: {
                            console.log("[VideoFileList] Delete clicked:", model.filePath)
                            deleteConfirmDialog.filePathToDelete = model.filePath
                            deleteConfirmDialog.fileNameToDelete = model.filename
                            deleteConfirmDialog.visible = true
                        }
                    }
                }

                // Empty state (no files)
                Item {
                    anchors.centerIn: parent
                    width: 200
                    height: 150
                    visible: gridView.count === 0 && !loadingOverlay.visible

                    Column {
                        anchors.centerIn: parent
                        spacing: 15

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "No video files found"
                            color: "white"
                            font.pixelSize: 16
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Start recording to create files"
                            color: "#888888"
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }
    }

    // Loading overlay
    Rectangle {
        id: loadingOverlay
        anchors.fill: parent
        color: "black"
        opacity: 0.6
        visible: false

        Column {
            anchors.centerIn: parent
            spacing: 15

            BusyIndicator {
                id: loadingIndicator
                anchors.horizontalCenter: parent.horizontalCenter
                running: loadingOverlay.visible
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Loading video files...")
                color: "white"
                font.pixelSize: 14
            }
        }
    }

    // Delete confirmation dialog
    Rectangle {
        id: deleteConfirmDialog
        anchors.fill: parent
        color: "transparent"
        visible: false
        z: 1000

        property string filePathToDelete: ""
        property string fileNameToDelete: ""

        // Semi-transparent overlay
        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.6

            MouseArea {
                anchors.fill: parent
                onClicked: deleteConfirmDialog.visible = false
            }
        }

        // Dialog box
        Rectangle {
            anchors.centerIn: parent
            width: isSmallScreen ? 300 : 400
            height: isSmallScreen ? 180 : 200
            color: "white"
            radius: 5
            border.color: "#cccccc"
            border.width: 2

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                // Title
                Text {
                    Layout.fillWidth: true
                    text: qsTr("Delete Video")
                    font.pixelSize: isSmallScreen ? 16 : 18
                    font.bold: true
                    color: "black"
                    horizontalAlignment: Text.AlignHCenter
                }

                // Message
                Column {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8

                    Text {
                        width: parent.width
                        text: qsTr("Are you sure you want to delete this video file?")
                        font.pixelSize: isSmallScreen ? 12 : 14
                        color: "#333333"
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        width: parent.width
                        text: deleteConfirmDialog.fileNameToDelete
                        font.pixelSize: isSmallScreen ? 13 : 15
                        font.bold: true
                        color: "#ff4444"
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        width: parent.width
                        text: qsTr("This action cannot be undone.")
                        font.pixelSize: isSmallScreen ? 11 : 13
                        color: "#666666"
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                // Buttons
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: isSmallScreen ? 35 : 40

                    RowLayout {
                        anchors.centerIn: parent
                        width: parent.width * 0.7
                        height: parent.height
                        spacing: 10

                        // Cancel button
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: cancelButtonArea.pressed ? "#cccccc" : "#f0f0f0"
                            radius: 5
                            border.color: "#999999"
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Cancel")
                                font.pixelSize: isSmallScreen ? 13 : 15
                                color: "black"
                            }

                            MouseArea {
                                id: cancelButtonArea
                                anchors.fill: parent
                                onClicked: deleteConfirmDialog.visible = false
                            }
                        }

                        // Delete button
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: deleteButtonArea.pressed ? "#cc0000" : "#ff4444"
                            radius: 5
                            border.color: "#cc0000"
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Delete")
                                font.pixelSize: isSmallScreen ? 13 : 15
                                font.bold: true
                                color: "white"
                            }

                            MouseArea {
                                id: deleteButtonArea
                                anchors.fill: parent
                                onClicked: {
                                    console.log("[VideoFileList] Delete confirmed:", deleteConfirmDialog.filePathToDelete)
                                    deleteConfirmDialog.visible = false
                                    loadingOverlay.visible = true

                                    if (root.fileManager) {
                                        root.fileManager.deleteFile(deleteConfirmDialog.filePathToDelete)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Error toast
    Rectangle {
        id: errorToast
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50
        width: Math.min(parent.width - 40, 400)
        height: errorToastText.height + 20
        color: "#f44336"
        opacity: 0.9
        radius: 8
        visible: false

        Text {
            id: errorToastText
            anchors.centerIn: parent
            width: parent.width - 20
            text: ""
            color: "white"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        Timer {
            id: errorToastTimer
            interval: 3000
            onTriggered: errorToast.visible = false
        }

        function show(message) {
            errorToastText.text = message
            errorToast.visible = true
            errorToastTimer.restart()
        }
    }

    // Success toast
    Rectangle {
        id: successToast
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50
        width: Math.min(parent.width - 40, 300)
        height: successToastText.height + 20
        color: "#4caf50"
        opacity: 0.9
        radius: 8
        visible: false

        Text {
            id: successToastText
            anchors.centerIn: parent
            text: ""
            color: "white"
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
        }

        Timer {
            id: successToastTimer
            interval: 2000
            onTriggered: successToast.visible = false
        }

        function show(message) {
            successToastText.text = message
            successToast.visible = true
            successToastTimer.restart()
        }
    }

    // Connections to FileManager
    Connections {
        target: root.fileManager

        onFileListUpdated: {
            console.log("[VideoFileList] File list updated, count:", fileList.length)
            fileListModel.clear()
            for (var i = 0; i < fileList.length; i++) {
                fileListModel.append(fileList[i])
            }
            loadingOverlay.visible = false
        }

        onFileListError: {
            console.error("[VideoFileList] File list error:", errorMsg)
            loadingOverlay.visible = false
            errorToast.show(errorMsg)
        }

        onFileDeleted: {
            console.log("[VideoFileList][DEBUG] onFileDeleted triggered, success:", success, "filePath:", filePath)
            if (success) {
                console.log("[VideoFileList] File deleted:", filePath)
                successToast.show(qsTr("File deleted successfully"))
                console.log("[VideoFileList][DEBUG] Starting refreshTimer...")
                refreshTimer.start()
            } else {
                console.error("[VideoFileList] Failed to delete:", filePath)
                loadingOverlay.visible = false
                errorToast.show(qsTr("Failed to delete file"))
            }
        }
    }

    // Refresh timer (debounce after deletion)
    Timer {
        id: refreshTimer
        interval: 100
        repeat: false
        onTriggered: {
            console.log("[VideoFileList][DEBUG] refreshTimer triggered, calling triggerRefresh()")
            triggerRefresh()
        }
    }

    // Helper function to trigger refresh
    function triggerRefresh() {
        if (root.fileManager) {
            console.log("[VideoFileList] Triggering file list refresh")
            loadingOverlay.visible = true
            root.fileManager.requestFileList()
        }
    }

    // Trigger file list load when view becomes visible
    onVisibleChanged: {
        if (visible && root.fileManager) {
            console.log("[VideoFileList] View visible, requesting file list")
            triggerRefresh()
        }
    }
}
