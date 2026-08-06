/*
 * VideoPlayerComponent.qml - Video playback interface
 * Full-screen video player with playback controls
 */

import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1

Rectangle {
    id: root
    anchors.fill: parent
    color: "transparent"  // Allow video to show through from /dev/fb0
    visible: false

    // Properties
    property var playbackManager: null  // Injected from MainWindow
    property bool controlsVisible: true  // Control bar visibility state
    
    // Computed properties from PlaybackManager
    readonly property int currentPosition: playbackManager ? Math.floor(playbackManager.position / 1000) : 0  // Convert ms to seconds
    readonly property int totalDuration: playbackManager ? Math.floor(playbackManager.duration / 1000) : 0
    readonly property bool isPlaying: playbackManager ? (playbackManager.state === 1) : false  // PlayingState = 1
    readonly property string currentFileName: playbackManager ? playbackManager.currentFilePath.split('/').pop() : ""

    readonly property bool isSmallScreen: parent.width <= 800
    readonly property int toolbarHeight: isSmallScreen ? 35 : 45
    readonly property int controlBarHeight: isSmallScreen ? 100 : 120

    // Signals
    signal backButtonClicked()
    signal playPauseClicked()
    signal seekBackward()
    signal seekForward()
    signal skipToStart()
    signal skipToEnd()
    signal seekTo(int position)

    // Connections to PlaybackManager
    Connections {
        target: playbackManager
        
        onPositionChanged: {
            // Update progress slider when position changes
            if (!progressSlider.pressed) {
                progressSlider.value = currentPosition
            }
        }
        
        onPlaybackFinished: {
            console.log("[VideoPlayer] Playback completed naturally")
            // UI automatically updates via property bindings
            // - playbackManager.state -> "stopped" -> play button shows "play" icon
            // - playbackManager.position -> duration -> progress bar at 100%
        }
        
        onPlaybackError: {
            console.log("[VideoPlayer] Playback error:", errorMsg)
            // TODO: Show error dialog
        }
    }
    
    // Timer for smooth progress bar updates during playback
    Timer {
        id: progressUpdateTimer
        interval: 500  // Update every 500ms for smooth animation
        running: isPlaying && visible
        repeat: true
        onTriggered: {
            // Update progress slider if not being dragged by user
            if (!progressSlider.pressed && playbackManager) {
                progressSlider.value = currentPosition
            }
        }
    }
    
    // Public functions
    function show() {
        visible = true
        console.log("[VideoPlayer] Shown")
    }

    function hide() {
        visible = false
        console.log("[VideoPlayer] Hidden")
    }

    // Format time as MM:SS
    function formatTime(seconds) {
        var mins = Math.floor(seconds / 60)
        var secs = seconds % 60
        return (mins < 10 ? "0" : "") + mins + ":" + (secs < 10 ? "0" : "") + secs
    }

    // Full-screen click area to toggle controls visibility
    // Placed behind the ColumnLayout to catch clicks on video area
    MouseArea {
        id: videoToggleArea
        anchors.fill: parent
        z: 1  // Below ColumnLayout (z: implicit, usually 0 or higher for items)
        
        onClicked: {
            // Only toggle if controls are visible OR if clicking outside control bar area
            // When controls are hidden, any click shows them
            // When controls are visible, clicking video area (not buttons) hides them
            console.log("[VideoPlayer] Background clicked, toggle controls visibility")
            root.controlsVisible = !root.controlsVisible
        }
    }

    // Main layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        z: 10  // Above the background MouseArea

        // Top toolbar with back button
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: toolbarHeight
            color: "transparent"  // Fully transparent like preview UI
            z: 10  // Above video area
            visible: root.controlsVisible  // Bind visibility to controls state
            
            // Intercept clicks to prevent toggling controls when clicking toolbar
            MouseArea {
                anchors.fill: parent
                onPressed: mouse.accepted = true
                onClicked: mouse.accepted = true
            }
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: isSmallScreen ? 10 : 15
                anchors.rightMargin: isSmallScreen ? 10 : 15
                spacing: 10
                
                // Filename display (left side)
                Text {
                    Layout.fillWidth: true
                    text: currentFileName
                    font.pixelSize: isSmallScreen ? 14 : 16  // Match file list toolbar
                    font.bold: true                          // Match file list toolbar
                    color: "white"
                    elide: Text.ElideMiddle
                    verticalAlignment: Text.AlignVCenter
                }
                
                // Back button (right side)
                Rectangle {
                    Layout.preferredWidth: isSmallScreen ? 50 : 60
                    Layout.preferredHeight: toolbarHeight
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
                        onClicked: {
                            console.log("[VideoPlayer] Back button clicked")
                            root.backButtonClicked()
                        }
                    }
                }
            }
        }

        // Video display area (transparent - hardware decoder renders to /dev/fb0)
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            // Hardware VDEC renders directly to framebuffer (/dev/fb0)
            // This QML layer is transparent overlay (Z=2) above video (Z=1)
            // Click events will fall through to videoToggleArea in the background
        }

        // Bottom control bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: controlBarHeight
            color: "transparent"  // Fully transparent like preview UI
            z: 10  // Above video area
            visible: root.controlsVisible  // Bind visibility to controls state
            
            // Intercept clicks to prevent toggling controls when clicking control bar
            MouseArea {
                anchors.fill: parent
                onPressed: mouse.accepted = true
                onClicked: mouse.accepted = true
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.leftMargin: isSmallScreen ? 15 : 20
                anchors.rightMargin: isSmallScreen ? 15 : 20
                anchors.topMargin: isSmallScreen ? 10 : 15
                anchors.bottomMargin: isSmallScreen ? 15 : 20
                spacing: isSmallScreen ? 8 : 10

                // Progress bar with time labels
                RowLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: isSmallScreen ? 15 : 20
                    Layout.rightMargin: isSmallScreen ? 15 : 20
                    spacing: 10

                    // Current time
                    Text {
                        text: formatTime(currentPosition)
                        color: "white"
                        font.pixelSize: isSmallScreen ? 14 : 16
                        font.bold: true
                        Layout.preferredWidth: 60
                    }

                    // Progress slider
                    Slider {
                        id: progressSlider
                        Layout.fillWidth: true
                        minimumValue: 0
                        maximumValue: totalDuration
                        value: currentPosition

                        style: SliderStyle {
                            groove: Rectangle {
                                implicitHeight: 8
                                color: "#444444"
                                radius: 4

                                Rectangle {
                                    width: (progressSlider.value / progressSlider.maximumValue) * parent.width
                                    height: parent.height
                                    color: "#00aaff"
                                    radius: 4
                                }
                            }

                            handle: Rectangle {
                                width: 24
                                height: 24
                                radius: 12
                                color: control.pressed ? "#00ccff" : "white"
                                border.color: "#00aaff"
                                border.width: 3
                            }
                        }

                        onPressedChanged: {
                            if (!pressed && playbackManager) {
                                var positionMs = Math.floor(value * 1000)  // Convert seconds to milliseconds
                                console.log("[VideoPlayer] Seek to:", positionMs, "ms")
                                playbackManager.seek(positionMs)
                            }
                        }
                    }

                    // Total duration
                    Text {
                        text: formatTime(totalDuration)
                        color: "white"
                        font.pixelSize: isSmallScreen ? 14 : 16
                        font.bold: true
                        Layout.preferredWidth: 60
                    }
                }

                // Control buttons
                RowLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignLeft
                    Layout.leftMargin: isSmallScreen ? 10 : 20
                    spacing: isSmallScreen ? 20 : 30

                    // Play/Pause button
                    Rectangle {
                        width: isSmallScreen ? 60 : 75
                        height: isSmallScreen ? 60 : 75
                        color: playPauseArea.pressed ? "#555555" : "transparent"
                        radius: 5

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            source: isPlaying ? "qrc:/images/player_pause.png" : "qrc:/images/player_play.png"
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            id: playPauseArea
                            anchors.fill: parent
                            onClicked: {
                                if (playbackManager) {
                                    if (isPlaying) {
                                        console.log("[VideoPlayer] Pausing playback")
                                        playbackManager.pause()
                                    } else {
                                        console.log("[VideoPlayer] Resuming/Replaying playback")
                                        playbackManager.play()  // Handles both resume and replay
                                    }
                                }
                            }
                        }
                    }

                    // Backward 10s button
                    Rectangle {
                        width: isSmallScreen ? 60 : 75
                        height: isSmallScreen ? 60 : 75
                        color: backwardArea.pressed ? "#555555" : "transparent"
                        radius: 5

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            source: "qrc:/images/player_backward.png"
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            id: backwardArea
                            anchors.fill: parent
                            onClicked: {
                                if (playbackManager) {
                                    console.log("[VideoPlayer] Seek backward 10s")
                                    playbackManager.seekBackward(10000)  // 10000ms = 10s
                                }
                            }
                        }
                    }

                    // Forward 10s button
                    Rectangle {
                        width: isSmallScreen ? 60 : 75
                        height: isSmallScreen ? 60 : 75
                        color: forwardArea.pressed ? "#555555" : "transparent"
                        radius: 5

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            source: "qrc:/images/player_forward.png"
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            id: forwardArea
                            anchors.fill: parent
                            onClicked: {
                                if (playbackManager) {
                                    console.log("[VideoPlayer] Seek forward 10s")
                                    playbackManager.seekForward(10000)  // 10000ms = 10s
                                }
                            }
                        }
                    }

                    // Skip to start button
                    Rectangle {
                        width: isSmallScreen ? 60 : 75
                        height: isSmallScreen ? 60 : 75
                        color: skipToStartArea.pressed ? "#555555" : "transparent"
                        radius: 5

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            source: "qrc:/images/player_skip_to_start.png"
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            id: skipToStartArea
                            anchors.fill: parent
                            onClicked: {
                                if (playbackManager) {
                                    console.log("[VideoPlayer] Skip to start")
                                    playbackManager.skipToStart()
                                }
                            }
                        }
                    }

                    // Skip to end button
                    Rectangle {
                        width: isSmallScreen ? 60 : 75
                        height: isSmallScreen ? 60 : 75
                        color: skipToEndArea.pressed ? "#555555" : "transparent"
                        radius: 5

                        Image {
                            anchors.centerIn: parent
                            width: parent.width
                            height: parent.height
                            source: "qrc:/images/player_skip_to_end.png"
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            id: skipToEndArea
                            anchors.fill: parent
                            onClicked: {
                                if (playbackManager) {
                                    console.log("[VideoPlayer] Skip to end")
                                    playbackManager.skipToEnd()
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
