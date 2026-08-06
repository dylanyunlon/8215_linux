
import QtQuick 2.2
import QtQuick.Window 2.2

Item {
    id: volumewindow
    objectName: "volumewindow"
    width: parent.width
    height: parent.height
    
    property int volume_type_media:0
    property int volume_type_gps:1
    property int volume_type_bt:2
    property int volume_type_mute:3

    property var volume_mute:       0
    
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        id: mutePanel
        visible: false
        
        Image {
            id: mute_image
            //x: 0
            //y: 0
            anchors.verticalCenter: mutePanel.verticalCenter
            anchors.horizontalCenter: mutePanel.horizontalCenter
            source: "mute.png"
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        id: volumePanel
        visible: false
        VolumePanel {
            id: mediaVolume
            width: parent.width
            height: parent.height
            visible: false
            maximumValue:40
            text: qsTr("Media")
        }

        VolumePanel {
            id: gpsVolume
            width: parent.width
            height: parent.height
            visible: false
            maximumValue: 40
            text: qsTr("GIS")
        }

        VolumePanel {
            id: btCallVolume
            width: parent.width
            height: parent.height
            visible: false
            maximumValue: 40
            text: qsTr("Bt Call")
        }
    }

    Component.onCompleted: {
        console.log("[VolumeDemo] volumewindow.qml Component.onCompleted is done\n");
    }


    Connections {
        target: volumeOverlay
        
        onBroadcastVolumeValue: {
            switch (type) {
                case volume_type_media:
                    mediaVolume.value = value;
                    break;

                case volume_type_gps:
                    gpsVolume.value = value;
                    break;

                case volume_type_bt:
                    btCallVolume.value = value;
                    break;

                case volume_type_mute:
                {
                    volume_mute = ((value == "1")? true : false);
                    if (volume_mute) {
                        volumePanel.visible = false;
                        mutePanel.visible = true;
                    } else {
                        mutePanel.visible = false;
                        volumePanel.visible = true;
                    }
                }
                    break;
            }
        }
    }

    Connections {
        target: volumeOverlay
        
        onShowVolume: {
            var notShowMute = 0;
            switch (type) {
                case volume_type_media:
                    mediaVolume.value = value;
                    gpsVolume.visible = false;
                    btCallVolume.visible = false;
                    mediaVolume.visible = true;
                    break;

                case volume_type_gps:
                    gpsVolume.value = value;
                    mediaVolume.visible = false;
                    btCallVolume.visible = false;
                    gpsVolume.visible = true;
                    break;

                case volume_type_bt:
                    btCallVolume.value = value;
                    mediaVolume.visible = false;
                    gpsVolume.visible = false;
                    btCallVolume.visible = true;
                    notShowMute = 1;
                    break;
            }

            console.log("onShowVolume, mute:"  + volume_mute + ", notShowMute: " + notShowMute);
            if (volume_mute && (notShowMute == 0)) {
                volumePanel.visible = false;
                mutePanel.visible = true;
            } else {
                mutePanel.visible = false;
                volumePanel.visible = true;
            }
        }
    }
}
