/*
 * MainWindow.qml - Main application window (Refactored)
 *
 * Phase 1: Basic UI structure without preview functionality
 * Includes: Toolbar, Button Container, Settings Panel, Toast
 */

import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

Item {
    id: root

    // Use Screen directly instead of relying on Item's width/height
    // This avoids timing issues with anchors.fill layout calculation
    readonly property bool isSmallScreen: Screen.width <= 800 || Screen.height <= 480
    readonly property int screenWidth: Screen.width
    readonly property int screenHeight: Screen.height

    // UI dimensions based on screen size
    readonly property int toolbarHeight: isSmallScreen ? 35 : 45
    readonly property int buttonContainerHeight: isSmallScreen ? 70 : 100
    readonly property int buttonContainerY: isSmallScreen ? 410 : 500

    Component.onCompleted: {
        console.log("[MainWindow] Created - Screen:", screenWidth, "x", screenHeight,
                    "Small:", isSmallScreen)
        console.log("[MainWindow] Toolbar height:", toolbarHeight)
        console.log("[MainWindow] Button container:", buttonContainerY, "x", buttonContainerHeight)
    }

    Component.onDestruction: {
        console.log("[MainWindow] Being destroyed")
    }

    // Connect to DVRBackend signals
    Connections {
        target: dvrBackend
        onCameraConnectionError: {
            console.log("[MainWindow] Camera connection error:", message)
            showToast(message, 3000, "error")
        }
        onErrorOccurred: {
            console.log("[MainWindow] Error occurred:", message)
            showToast(message, 3000, "error")
        }
        // Note: Recording state changes can be triggered by:
        // 1. User clicking record button - should show toast
        // 2. Camera switch (different cameras have different states) - should NOT show toast
        // We removed toast here because recording state feedback is already clear from button icon
        onRecordingChanged: {
            console.log("[MainWindow] Recording state changed:", recording)
        }
    }

    // Top toolbar
    Loader {
        id: toolbar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: toolbarHeight
        source: "components/ToolbarComponent.qml"
        visible: !videoPlayer.item || !videoPlayer.item.visible  // Hide when video player is visible

        // No background - fully transparent (matching original DVR app)

        onLoaded: {
            // Connect toolbar button signals to DVRBackend
            item.homeClicked.connect(function() {
                console.log("[MainWindow] Home button clicked, calling dvrBackend.goHome()")
                dvrBackend.goHome()
            })
            item.exitClicked.connect(function() {
                console.log("[MainWindow] Exit button clicked, calling dvrBackend.goExit()")
                dvrBackend.goExit()
            })
        }
    }

    // Bottom button container
    Loader {
        id: buttonContainer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: buttonContainerHeight
        source: "components/ButtonContainerComponent.qml"
        visible: !videoPlayer.item || !videoPlayer.item.visible  // Hide when video player is visible

        // No background - fully transparent to show video through
        // Buttons themselves have semi-transparent backgrounds for visibility

        onLoaded: {
            // Bind camera state to button component
            item.isFrontCamera = Qt.binding(function() { return dvrBackend.isFrontCamera })

            // Bind recording state to button component
            item.isRecording = Qt.binding(function() { return dvrBackend.isRecording })

            // Connect button signals
            item.recordClicked.connect(function() {
                console.log("[MainWindow] Record button clicked, current recording state:", dvrBackend.isRecording)
                dvrBackend.toggleRecording()
            })
            item.snapshotClicked.connect(function() {
                console.log("[MainWindow] Snapshot button clicked")
            })
            item.switchCameraClicked.connect(function() {
                console.log("[MainWindow] Switch camera button clicked")
                dvrBackend.switchCamera()
            })
            item.settingsClicked.connect(function() {
                console.log("[MainWindow] Settings button clicked")
                if (settingsPanel.item) {
                    settingsPanel.item.toggle()
                }
            })
            item.directoryClicked.connect(function() {
                console.log("[MainWindow] Directory button clicked")
                dvrBackend.showFileList()
            })
        }
    }

    // Settings panel (right side overlay)
    Loader {
        id: settingsPanel
        anchors.right: parent.right
        anchors.top: toolbar.bottom
        anchors.topMargin: 10
        source: "components/SettingsPanelComponent.qml"

        onLoaded: {
            if (item) {
                // Pass settingsManager from dvrBackend to the settings panel
                item.settingsManager = dvrBackend.settingsManager
                console.log("[MainWindow] SettingsManager (from dvrBackend) passed to SettingsPanelComponent")
            }
        }
    }

    // File list view (full-screen overlay, hidden by default)
    Loader {
        id: fileListView
        anchors.fill: parent
        z: 10  // Above all other components
        active: false
        source: "components/VideoFileListComponent.qml"

        onLoaded: {
            console.log("[MainWindow] VideoFileListComponent loaded")

            if (item) {
                // Bind FileManager property
                item.fileManager = dvrBackend.fileManager
                console.log("[MainWindow] FileManager bound to VideoFileListComponent")

                // Manually trigger initial file list load
                if (typeof item.triggerRefresh === "function") {
                    console.log("[MainWindow] Triggering initial file list load")
                    item.triggerRefresh()
                }

                // Connect back button to hide file list
                item.backClicked.connect(function() {
                    console.log("[MainWindow] VideoFileList back button clicked")
                    dvrBackend.hideFileList()
                })

                // Connect play file signal to show player
                item.videoFileSelected.connect(function(filePath) {
                    console.log("[MainWindow] Play file requested:", filePath)
                    // Use DVRBackend to handle exclusive resource management
                    // DVRBackend will: 1) Stop preview 2) Show player UI 3) Start playback
                    dvrBackend.showVideoPlayer(filePath)
                })
            }
        }
    }

    // Connect DVRBackend navigation signals
    Connections {
        target: dvrBackend
        onNavigateToFileList: {
            console.log("[MainWindow] Showing file list")
            // Close settings panel when switching to file list
            if (settingsPanel.item && settingsPanel.item.visible) {
                settingsPanel.item.toggle()
            }
            fileListView.active = true
        }
        onNavigateBackFromFileList: {
            console.log("[MainWindow] Hiding file list")
            fileListView.active = false
        }
        onVideoPlayerRequested: {
            console.log("[MainWindow] Video player requested for:", filePath)
            // Hide file list
            fileListView.active = false
            // Show video player
            if (videoPlayer.item) {
                videoPlayer.item.show()
            }
            // PlaybackManager.playVideo() is called by DVRBackend.showVideoPlayer()
        }
        onVideoPlayerClosed: {
            console.log("[MainWindow] Video player closed")
            // Hide video player
            if (videoPlayer.item) {
                videoPlayer.item.hide()
            }
            // Show file list again
            fileListView.active = true
        }
    }

    // Video player view (full-screen overlay, hidden by default)
    Loader {
        id: videoPlayer
        anchors.fill: parent
        z: 15  // Above file list view
        source: "components/VideoPlayerComponent.qml"

        onLoaded: {
            console.log("[MainWindow] VideoPlayer loaded")

            if (item) {
                // Inject PlaybackManager from DVRBackend
                item.playbackManager = dvrBackend.playbackManager
                console.log("[MainWindow] PlaybackManager bound to VideoPlayerComponent")
                
                // Connect back button to return to file list
                item.backButtonClicked.connect(function() {
                    console.log("[MainWindow] Player back button clicked")
                    // Use DVRBackend to handle exclusive resource management
                    // DVRBackend will: 1) Stop playback 2) Hide player UI 3) Resume preview
                    dvrBackend.hideVideoPlayer()
                })
                
                // Playback control is now handled directly in VideoPlayerComponent
                // via playbackManager property binding
            }
        }
    }

    // Toast message (bottom-right, above button container)
    Loader {
        id: toast
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.bottom: buttonContainer.top
        anchors.bottomMargin: 15
        source: "components/ToastComponent.qml"
    }

    // Expose toast function for external use
    // type: "info" (default) or "error"
    function showToast(message, duration, type) {
        if (toast.item) {
            toast.item.show(message, duration, type)
        }
    }
}
