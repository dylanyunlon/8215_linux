
import QtQuick 2.2
import QtQuick.Window 2.2

Window {
    id: volume_overlay
    objectName: "volume_overlay"
    flags: Qt.Window | Qt.FramelessWindowHint
    visible: true
    x: (Screen.width - width)/2
    y: (Screen.height - height)/2
    width: 600
    height: 120
    color: "transparent"

    Rectangle {
	    anchors.fill: parent
        color: "black"
        visible: true
        radius: 12
        opacity: 0.4
        z: -0.8
    }

   //the following is using async loader
   Loader {
        id: volumeWindowLoader
        //asynchronous: true
        asynchronous: false
        width: parent.width
        height: parent.height
        onStatusChanged:if(volumeWindowLoader.status == Loader.Ready)
        {
            console.log("volumeWindowLoader loaded!");
        }
    }

    Component.onCompleted: {
        console.log("[VolumeDemo] volume_overlay.qml Component.onCompleted is called\n");
        volumeWindowLoader.source = "volumewindow.qml"
        console.log("[VolumeDemo] volume_overlay.qml Component.onCompleted is called done\n");
    }
}
