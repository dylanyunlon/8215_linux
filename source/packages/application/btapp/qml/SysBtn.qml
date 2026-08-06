import QtQuick 2.0

Rectangle
{
    id:sysbtn

    property string picCurrent: ""
    property string picNormal: ""
    property string picPressed: ""

    signal clicked

    color:"#00000000"
    state:"normal"

    Image {
        anchors.centerIn: parent.Center
        source: picCurrent;
        anchors.fill: parent
        asynchronous: true
        cache: true
    }

    MouseArea {
        hoverEnabled: true
        anchors.fill: parent
        onPressed: sysbtn.state = "pressed"
        onReleased:
        {
            sysbtn.state = "normal"
            sysbtn.clicked()
        }
    }

    states:
    [
        State {
            name: "normal"
            PropertyChanges {
                target: sysbtn
                picCurrent:picNormal
            }
        },
        State {
            name: "pressed"
            PropertyChanges {
                target: sysbtn
                picCurrent:picPressed
            }
        }
    ]
}







