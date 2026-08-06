import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

Item {
    id: selectconnect
    anchors.fill: parent

    property bool phonevalue: true
    property bool musicvalue: true
    signal sendConnectSelect(bool phone, bool music)

    PropertyAnimation { target: selectconnect; property: "opacity";
        duration: 400; from: 0; to: 1;
        easing.type: Easing.InOutQuad ; running: true }

    Rectangle {
        anchors.fill: parent
        id: overlay
        color: "#000000"
        opacity: 0.6
        // add a mouse area so that clicks outside
        // the dialog window will not do anything
        MouseArea {
            anchors.fill: parent
        }
    }

    Rectangle {
        width: 320
        height: 200
        radius: 5
        color:"grey"
        anchors.centerIn: parent

        CheckBox {
            id: phone
            anchors.top: parent.top
            anchors.topMargin: 16
            anchors.left: parent.left
            anchors.leftMargin: 56
            checked: phonevalue

            style: CheckBoxStyle {
                indicator: Rectangle {
                        implicitWidth: 40
                        implicitHeight: 40
                        radius: 3
                        border.color: control.activeFocus ? "darkblue" : "gray"
                        border.width: 1
                        Rectangle {
                            visible: control.checked
                            color: "red"
                            border.color: "green"
                            radius: 10
                            anchors.margins: 3
                            anchors.fill: parent
                        }
                }
            }
        }

        Text {
            id: phone_audio
            anchors.left: phone.right
            anchors.leftMargin: 8
            anchors.verticalCenter: phone.verticalCenter
            text: qsTr("Phone Audio")
            font.pointSize: 12
        }

        CheckBox {
            id: music
            anchors.top: phone.bottom
            anchors.topMargin: 16
            anchors.left: parent.left
            anchors.leftMargin: 56
            checked: musicvalue

            style: CheckBoxStyle {
                indicator: Rectangle {
                    implicitWidth: 40
                    implicitHeight: 40
                    radius: 3
                    border.color: control.activeFocus ? "darkblue" : "gray"
                    border.width: 1
                    Rectangle {
                        visible: control.checked
                        color: "red"
                        border.color: "green"
                        radius: 10
                        anchors.margins: 3
                        anchors.fill: parent
                    }
                }
            }
        }

        Text {
            id: media_audio
            anchors.left: music.right
            anchors.leftMargin: 8
            anchors.verticalCenter: music.verticalCenter
            text: qsTr("Media Audio")
            font.pointSize: 12
        }

        Button {
            id:ok_btn
            width:96
            height: 56
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                id: ok
                anchors.centerIn: parent
                text:qsTr("OK")
                font.pointSize: 12
            }

            onClicked: {
                var bphone = phone.checked
                var bmusic = music.checked
                console.log("[SelectConnect] phone = "+bphone+", media = "+bmusic)
                selectconnect.sendConnectSelect(bphone, bmusic)
                selectconnect.destroy()

            }
        }
    }

    Connections {
        target: bluetoothApplication
        onLanguageChanged: {
            console.log("[SelectConnect] onLanguageChanged");
            phone_audio.text = qsTr("Phone Audio")
            media_audio.text = qsTr("Media Audio")
            ok.text = qsTr("OK")
        }
    }

    Connections {
        target: bluetoothPairRecordsPage
        onUpdateCheckBoxPhoneConnectState: {
            console.log("[SelectConnect] onUpdateCheckBoxPhoneConnectState, isConnected = " + isConnected);
            phonevalue = isConnected;
        }
    }

    Connections {
        target: bluetoothPairRecordsPage
        onUpdateCheckBoxMediaConnectState: {
            console.log("[SelectConnect] onUpdateCheckBoxMediaConnectState, isConnected = " + isConnected);
            musicvalue = isConnected;
        }
    }
}


