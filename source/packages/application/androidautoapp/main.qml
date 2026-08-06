import QtQuick 2.5
import QtQuick.Window 2.2

//use for delivery coordinate value
Window {
    id: id_rootWindow
    visible: false
    width: 1024
    height: 600
    color: "transparent"
    flags:Qt.FramelessWindowHint

    onVisibleChanged: {
        if (visible) {
            rootItem.forceActiveFocus()
        }
    }

    Item {
        id: rootItem
        anchors.fill: parent
        focus: true

        Component.onCompleted: {
            forceActiveFocus()
        }

        Keys.onPressed: {
            managerservice.onKeyEvent(true, event.key)
            event.accepted = true
        }

        Keys.onReleased: {
            managerservice.onKeyEvent(false, event.key)
            event.accepted = true
        }
    }

    MouseArea {
        anchors.fill: parent

        property int actionDown: 0;
        property int actionUp: 1;
        property int actionMove: 2;

        onPressed: {
            console.log("onUITouched");
            managerservice.onUITouched(actionDown, Math.round(mouseX), Math.round(mouseY))
        }
        onReleased: {
            managerservice.onUITouched(actionUp, Math.round(mouseX), Math.round(mouseY))
        }
        onPositionChanged: {
            managerservice.onUITouched(actionMove, Math.round(mouseX), Math.round(mouseY))
        }
    }
}
