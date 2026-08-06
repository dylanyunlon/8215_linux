import QtQuick 2.6
import QtQuick.Window 2.2

//use for delivery coordinate value
Window {
    id: id_rootWindow
    visible: true
    width: managerservice.getScreenWidth()
    height: managerservice.getScreenHeight()
    color: "transparent"
    flags:Qt.FramelessWindowHint

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
        onPressed: {
            managerservice.onUITouched(true, Math.round(mouseX), Math.round(mouseY))
        }
        onReleased: {
            managerservice.onUITouched(false, Math.round(mouseX), Math.round(mouseY))
        }
        onPositionChanged: {
            managerservice.onUITouched(true, Math.round(mouseX), Math.round(mouseY))
        }
    }
}
