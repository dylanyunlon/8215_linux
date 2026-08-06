import QtQuick 2.0

Rectangle {
    id: opacitybtn

    property string picNormal: ""
    property string picPressed: ""

    signal clicked()
    signal pressAndHold()
    color:"#00000000"
    BorderImage {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        source: picNormal
        asynchronous: true
        cache: true
    }

    Image {
        id: backgroud
        source: picPressed
        opacity: 0
        asynchronous: true
        cache: true
    }

    MouseArea {
        anchors.fill: parent
        onPressed: opacitybtn.state = "pressed"
        onReleased: opacitybtn.state = "normal"
        onClicked: {
            opacitybtn.clicked()
        }
        onPressAndHold: {
            opacitybtn.pressAndHold()
        }
    }
    states:
    [
        State {
            name: "normal"
            PropertyChanges {
                target: backgroud
                opacity: 0
            }
        },
        State {
            name: "pressed"
            PropertyChanges {
                target: backgroud
                opacity: 0.35
            }
        }
    ]
}


