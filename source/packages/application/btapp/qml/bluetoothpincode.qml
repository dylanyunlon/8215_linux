import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

Item {
    id: passwordDialog
    anchors.fill: parent

    signal setPasswordClicked(string msg)
    property string btpassword: "0000";
    property bool keyboardstate: false;

    function doClickNumber(str) {
        btpassword = str;
        console.log("[BluetoothPinCode] btpassword = "+btpassword);
    }

    function doNumericKeyboardClose() {
        keyboardstate = false;
        dialogWindow.visible = true;
        dialogWindow2.visible = false;
        console.log("[BluetoothPinCode] doNumericKeyboardClose");
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
    PropertyAnimation { target: passwordDialog; property: "opacity";
        duration: 400; from: 0; to: 1;
        easing.type: Easing.InOutQuad ; running: true }

    Rectangle {
        anchors.fill: parent
        id: overlay
        color: "#000000"
        opacity: 0.6
        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("[BluetoothPinCode] PinCode's outside");
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
            id:dev_pincode
            width: 464
            height: 40
            anchors.top: dev_info.bottom
            anchors.topMargin: 8
            font.pointSize: 12
            anchors.left: parent.left
            anchors.leftMargin: 8
            color: "#BFBFBF"
            text:qsTr("PIN Code:")
        }

        TextField {
            id: btPasswordText
            anchors.top: dev_pincode.bottom
            anchors.topMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8
            focus: true
            font.pointSize: 20
            height: 80
            width: 464
            text:btpassword
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
                    dialogWindow.visible = false
                    dialogWindow2.visible = true
                    if (false == keyboardstate) {
                        console.log("[BluetoothPinCode] btn clicked");
                        var component = Qt.createComponent("/qml/numericKeyboard.qml");
                        console.log("[BluetoothSettingPage] numericKeyboard component:"+component.status+Component.Ready);
                        if (component.status == Component.Ready) {
                            console.log("[BluetoothSettingPage] creatObject:numericKeyboard before");
                            var keyboardDialog = component.createObject(passwordDialog,{click_number:btpassword});
                            console.log("[BluetoothSettingPage] creatObject:numericKeyboard after");
                            keyboardDialog.setClickNumber.connect(doClickNumber);
                            keyboardDialog.sigNumericKeyboardClose.connect(doNumericKeyboardClose);
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
            anchors.top: btPasswordText.bottom
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
                str = btPasswordText.text
                if ("" == str) {
                    tips.text = qsTr("please input PIN Code");
                    tipbox.visible = true;
                    hidetipbox.start();
                } else {
                    passwordDialog.setPasswordClicked(str);
                    passwordDialog.destroy();
                }
            }
        }

        Button {
            id:cancel_btn
            width:96
            height: 56
            anchors.top: btPasswordText.bottom
            anchors.topMargin: 16
            anchors.left: ok_btn.right
            anchors.leftMargin: 96

            Text {
                anchors.centerIn: parent
                text:qsTr("Cancel")
                font.pointSize: 12
            }

            onClicked: {
                passwordDialog.destroy();
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
            id: dev_info2
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
            id:dev_pincode2
            width: 464
            height: 40
            anchors.top: dev_info2.bottom
            anchors.topMargin: 8
            font.pointSize: 12
            anchors.left: parent.left
            anchors.leftMargin: 8
            color: "#BFBFBF"
            text:qsTr("PIN Code:")
        }

        TextField {
            id: btPasswordText2
            anchors.top: dev_pincode2.bottom
            anchors.topMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8
            focus: true
            font.pointSize: 20
            height: 80
            width:464
            text:btpassword
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
        }
    }


    Connections {
        target: bluetoothApplication
        onPhoneConnectStateChanged: {
            console.log("[keyboard] onPhoneConnectStateChanged, m_phoneConnectState = "+m_phoneConnectState);
            if (false == m_phoneConnectState) {
                console.log("passwordDialog.destroy()");
                passwordDialog.destroy();
            }
        }
    }

    Connections {
        target: bluetoothApplication
        onMediaConnectStateChanged: {
            console.log("[keyboard] onMediaConnectStateChanged, m_mediaConnectState = "+m_mediaConnectState);
            if (false == m_mediaConnectState) {
                console.log("passwordDialog.destroy()");
                passwordDialog.destroy();
            }
        }
    }


}



