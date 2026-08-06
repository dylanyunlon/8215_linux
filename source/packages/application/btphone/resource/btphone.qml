import QtQuick 2.2
import QtQml.Models 2.1
import QtQuick.Controls 1.2
import QtQuick.Window 2.2

Window {
    width: 320;
    height: 240;
    color: "transparent";
    Rectangle{
        visible: false;
        anchors.centerIn: parent;
        color: "red";
        width: 160;
        height: 120;
        Text {
            id: myText;
            visible: false;
            anchors.centerIn: parent;
            text: "I'm floating window";
        }
    }
}
