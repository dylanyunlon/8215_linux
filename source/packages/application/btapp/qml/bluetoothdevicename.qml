import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

Item {
    id: dialogComponent
    anchors.fill: parent

    signal setNameClicked(string msg)
    property string btname: "";
    property bool keyboardstate: false;

    function doClickString(str) {
        btname = str;
        console.log("[BluetoothDeviceName] btname = "+btname);
    }

    function doKeyboardClose() {
        keyboardstate = false;
        dialogWindow.visible = true;
        dialogWindow2.visible = false;
        console.log("[BluetoothDeviceName] doKeyboardClose");
    }

    Timer{
        id:hidetipbox
        interval: 2000
        repeat:false
        onTriggered: {
            tipbox.visible = false;
        }
    }

    // Add a simple animation to fade in the popup
    // let the opacity go from 0 to 1 in 400ms
    PropertyAnimation { target: dialogComponent; property: "opacity";
        duration: 400; from: 0; to: 1;
        easing.type: Easing.InOutQuad ; running: true }

    // This rectange is the a overlay to partially show the parent through it
    // and clicking outside of the 'dialog' popup will do 'nothing'
    Rectangle {
        anchors.fill: parent
        id: overlay
        color: "#000000"
        opacity: 0.6
        // add a mouse area so that clicks outside
        // the dialog window will not do anything
        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("[BluetoothDeviceName] DeviceName's outside");
            }
        }
    }

    Rectangle{
        id: tipbox
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top:dialogWindow.bottom
        anchors.topMargin: 16
        height: 40
        width: 280
        color: "#BFBFBF"
        opacity: 0.8
        visible: false
        z:1

        Rectangle{
            anchors.centerIn: parent
            height: parent.height
            width: parent.width
            color: "gray"
            visible: true
            opacity: 0.8
            z:0
        }

        Text {
            id: tips
            color: "white"
            font.pixelSize: 22
            text: ""
            anchors.centerIn: parent
            objectName: "tips"
        }
    }

    // This rectangle is the actual popup
    Rectangle {
        id: dialogWindow
        width: 480
        height: 264
        radius: 5
        color:"#404040"
        anchors.centerIn: parent

        Text {
            id: dev_info
            width: 460
            height: 32
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8
            font.pointSize: 9
            color: "#BFBFBF"
            text: qsTr("Device Information")
        }

        Text {
            id:dev_name
            width: 460
            height: 40
            anchors.top: dev_info.bottom
            anchors.topMargin: 8
            font.pointSize: 12
            anchors.left: parent.left
            anchors.leftMargin: 8
            color: "#BFBFBF"
            text:qsTr("Bluetooth Name:")
        }

        TextField {
            id: btName
            anchors.top: dev_name.bottom
            anchors.topMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8
            focus: true
            font.pointSize: 20
            height: 80
            width: 464
            text:btname
            textColor:"black"

            style: TextFieldStyle {
                background: Rectangle {
                            radius: 10
                            border.color: "#FF9900"
                            border.width: 2
                            color:"white"
                            opacity: 1
                        }
            }

            MouseArea {
                anchors.fill: parent
                visible: !keyboardstate
                opacity: 0.1
                onClicked: {
                    dialogWindow.visible = false;
                    dialogWindow2.visible = true;
                    if (false == keyboardstate) {
                        console.log("[BluetoothDeviceName] btn clicked");
                        var component = Qt.createComponent("/qml/keyboard.qml");
                        console.log("[BluetoothSettingPage] keyboard component:"+component.status+Component.Ready);
                        if (component.status == Component.Ready) {
                            console.log("[BluetoothSettingPage] creatObject:keyboard before");
                            var keyboardDialog = component.createObject(dialogComponent,{click_string:btname});
                            console.log("[BluetoothSettingPage] creatObject:keyboard after");
                            keyboardDialog.sigClickString.connect(doClickString);
                            keyboardDialog.sigKeyboardClose.connect(doKeyboardClose);
                        }

                        keyboardstate = true;
                    }
                }
            }

        }


        Button {
            id:ok_btn
            width:96
            height: 56
            anchors.top: btName.bottom
            anchors.topMargin: 16
            anchors.left: parent.left
            anchors.leftMargin: 96

            Text {
                anchors.centerIn: parent
                text:qsTr("OK")
                font.pointSize: 12
            }

            onClicked: {
                var str
                str = btName.text
                if ("" == str) {
                    tips.text = qsTr("please input BT name");
                    tipbox.visible = true;
                    hidetipbox.start();
                } else {
                    dialogComponent.setNameClicked(str);
                    dialogComponent.destroy();
                }
            }
        }

        Button {
            id:cancel_btn
            width:96
            height: 56
            anchors.top: btName.bottom
            anchors.topMargin: 16
            anchors.left: ok_btn.right
            anchors.leftMargin: 96

            Text {
                anchors.centerIn: parent
                text:qsTr("Cancel")
                font.pointSize: 12
            }

            onClicked: {
                dialogComponent.destroy();
            }
        }
    }

    Rectangle {
        id: dialogWindow2
        width: 480
        height: 264
        radius: 5
        color:"#404040"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 72
        visible: false

        Text {
            id:dev_info2
            width: 464
            height: 32
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8
            font.pointSize: 9
            color: "#BFBFBF"
            text: qsTr("Device Information")
        }

        Text {
            id:dev_name2
            width: 464
            height: 40
            anchors.top: dev_info2.bottom
            anchors.topMargin: 8
            font.pointSize: 12
            anchors.left: parent.left
            anchors.leftMargin: 8
            color: "#BFBFBF"
            text:qsTr("Bluetooth Name:")
        }

        TextField {
            id: btName2
            anchors.top: dev_name2.bottom
            anchors.topMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8
            focus: true
            font.pointSize: 20
            height: 80
            width:464
            text:btname
            textColor:"black"

            style: TextFieldStyle {
                background: Rectangle {
                            radius: 10
                            border.color: "#FF9900"
                            border.width: 2
                            color:"white"
                            opacity: 0.9
                        }
            }
        }

    }


    Connections {
        target: bluetoothApplication
        onPhoneConnectStateChanged: {
            console.log("[keyboard] onPhoneConnectStateChanged, m_phoneConnectState = "+m_phoneConnectState);
            if (false == m_phoneConnectState) {
                console.log("dialogComponent.destroy()");
                dialogComponent.destroy();
            }
        }
    }

    Connections {
        target: bluetoothApplication
        onMediaConnectStateChanged: {
            console.log("[keyboard] onMediaConnectStateChanged, m_mediaConnectState = "+m_mediaConnectState);
            if (false == m_mediaConnectState) {
                console.log("dialogComponent.destroy()");
                dialogComponent.destroy();
            }
        }
    }

}


