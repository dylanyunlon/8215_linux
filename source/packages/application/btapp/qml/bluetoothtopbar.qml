import QtQuick 2.5

Item {
    id: bluetoothBar
    width: parent.width
    height: 44
    opacity: 0.7

    property bool bleutoothClock24IsActive: bluetoothApplication.getClockState();
    property string bleutoothAM : qsTr(" AM");
    property string bleutoothPM : qsTr(" PM");

    // Properties for keyboard navigation highlight
    property int highlightedButton: -1 // -1 = none, 0 = home, 1 = back

    // Functions to handle highlight from parent
    function setHighlight(buttonIndex) {
        highlightedButton = buttonIndex;
    }

    function clearHighlight() {
        highlightedButton = -1;
    }

    Image {
        id: topBar
        source: "/image/btmainView/topbase5%.png"
        asynchronous: true
        cache: true
        anchors.centerIn: parent
        opacity: 0.1
    }

    Timer{
        id:curtimetimer
        interval: 1000
        repeat: true
        running: true
        onTriggered: {
            var d = new Date();
            if (true == bleutoothClock24IsActive) {
                var curTime = add_zero(d.getHours())+":"+add_zero(d.getMinutes())+":"+add_zero(d.getSeconds());
                time.text = curTime.toString();
            } else {
                var hour = d.getHours();
                if (hour >= 12) {
                    if (hour > 12) {
                        hour -= 12;
                    }
                    var curTime = add_zero(hour)+":"+add_zero(d.getMinutes())+":"+add_zero(d.getSeconds())+bleutoothPM;
                    time.text = curTime.toString();
                } else {
                    if (hour == 0) {
                        hour = 12;
                    }
                    var curTime = add_zero(hour)+":"+add_zero(d.getMinutes())+":"+add_zero(d.getSeconds())+bleutoothAM;
                    time.text = curTime.toString();
                }
            }
        }
    }

    Rectangle {
        id: bt_home
        height: 40
        width: 84
        color: highlightedButton === 0 ? "#BFBFBF" : "#00000000"
        opacity: highlightedButton === 0 ? 0.6 : 1
        anchors.top: parent.top
        anchors.topMargin: 3
        radius: 5

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            source: "/image/btmainView/backhome.png"
            asynchronous: true
            cache: true
        }

        Image {
            id: back_d
            source: "/image/btmainView/back_d.png"
            opacity: 0
            asynchronous: true
            cache: true
        }

        MouseArea {
            anchors.fill: parent
            onPressed: back_d.opacity = 0.5
            onReleased: {
                console.log("[BluetoothTopbar] onReleased~gohome");
                back_d.opacity = 0
                bluetoothApplication.goHome();
            }
        }
    }

    Image {
        id: line
        source: "/image/btmainView/depart_line.png"
        asynchronous: true
        cache: true
        anchors.left: bt_home.right
        width: 1
        height: 29
        anchors.verticalCenter: parent.verticalCenter
    }

    Rectangle {
        height: 40
        width: 84
        color:"#00000000"
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.left: line.right
        anchors.leftMargin: 24

        Text {
            id: title
            color: "white"
            font.pixelSize: 20
            opacity: 1
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Bluetooth")
        }
    }

    Rectangle {
        height: 40
        width: 84
        color:"#00000000"
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.horizontalCenter: parent.horizontalCenter

        Text {
            id: time
            color: "white"
            font.pixelSize: 20
            text: ""
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            objectName: "time"
        }
    }

    Rectangle {
        id: bt_recent
        height: 40
        width: 84
        color: highlightedButton === 1 ? "#BFBFBF" : "#00000000"
        opacity: highlightedButton === 1 ? 0.6 : 1
        anchors.top: parent.top
        anchors.topMargin: 4
        anchors.right: parent.right
        radius: 5

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            source: "/image/btmainView/back.png"
            asynchronous: true
            cache: true
        }

        Image {
            id: back_h
            source: "/image/btmainView/back_d.png"
            opacity: 0
            asynchronous: true
            cache: true
        }

        MouseArea {
            anchors.fill: parent
            onPressed: back_h.opacity = 0.5
            onReleased: {
                console.log("[BluetoothTopbar] onReleased~goexit");
                back_h.opacity = 0
                bluetoothApplication.goExit();
            }
        }
    }

    Connections {
        target: bluetoothApplication
        onClockStateChanged: {
            console.log("[BluetoothTopbar] onClockStateChanged, m_clock24IsActive = "+m_clock24IsActive);
            bleutoothClock24IsActive = m_clock24IsActive;
        }
    }

    Connections {
        target: bluetoothApplication
        onLanguageChanged: {
            console.log("[BluetoothTopbar] onLanguageChanged");
            title.text = qsTr("Bluetooth");
            bleutoothAM = qsTr(" AM");
            bleutoothPM = qsTr(" PM");
        }
    }
}