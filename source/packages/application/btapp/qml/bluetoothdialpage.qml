import QtQuick 2.0
import QtQuick.Controls 1.3

Rectangle {
    id: bluetoothDialPageView
    width: 585
    height: 402
    radius: 12
    border.color:"#00000000"
    color:"#00000000"

    property string dialNumber: bluetoothDialPage.getDialNumber();
    property string redialNumber: bluetoothDialPage.getRedialNumber();

/*
    NumberAnimation { target: bluetoothDialPageView; property: "x";
        duration: 400; from: 732; to: 0;
        easing.type: Easing.OutBounce
        easing.amplitude: 0.1
        running: true }
*/

    Image {
        source:"/image/dailinterfaceView/bluetooth_rightbox.png"
    }

    Timer{
        id:hidetipbox
        interval: 2000
        repeat:false
        onTriggered: {
            tipbox.visible = false
        }
    }

    Rectangle{
        id: tipbox
        anchors.top: parent.top
        anchors.topMargin: 251
        anchors.left: parent.left
        anchors.leftMargin: 92
        height: 40
        width: 280
        color: "#BFBFBF"
        visible: false
        z:1
        radius: 10

        Rectangle{
            anchors.centerIn: parent
            height: parent.height
            width: parent.width
            color: "gray"
            visible: true
            opacity: 0.8
            z:0
            radius: 10
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

    Rectangle {
        id: phonebook_show
        height: 73
        width: 553
        anchors.top: parent.top
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.leftMargin: 16
        color:"#00000000"

        Image {
            height: 73
            width: 553
            source: "/image/dailinterfaceView/number.png"
            opacity: 0.4
            asynchronous: true
            cache: true
        }

        Text {
            id:dial_number
            width: 398
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: delete_num.left
            anchors.rightMargin: 28
            focus: true
            clip: true
            font.pointSize: 20
            font.bold: true
            horizontalAlignment: TextInput.AlignRight
            elide:Text.ElideLeft
            color: "white"
            text: dialNumber
        }

        Image {
            anchors.right: delete_num.left
            anchors.rightMargin: 6
            anchors.verticalCenter: parent.verticalCenter
            source: "/image/dailinterfaceView/number_divider.png"
            asynchronous: true
            cache: true
        }

        OpacityBtn {
            id:delete_num
            width:10684
            height:40
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter

            picNormal: "/image/dailinterfaceView/delete_n.png"
            picPressed: "/image/dailinterfaceView/delete_d30%.png"

            onClicked: {
                if (dialNumber != "") {
                    dialNumber = dialNumber.substring(0,dialNumber.length - 1);
                    bluetoothDialPage.setDialNumber(dialNumber);
                }
            }
            onPressAndHold: {
                dialNumber = "";
                bluetoothDialPage.setDialNumber(dialNumber);
            }
        }
    }

    SysBtn {
        id:bt_dialout_btn
        width:106
        height:132
        anchors.top: phonebook_show.top
        anchors.topMargin: 89
        anchors.left:dialpan.right
        anchors.leftMargin: 8
        picNormal: "/image/dailinterfaceView/dailconnect_n.png"
        picPressed: "/image/dailinterfaceView/dailconnect_d.png"
        onClicked: {
            console.log("[BluetoothDialPage] call_btn clicked");
            console.log("[BluetoothDialPage] dialNumber = "+dialNumber);
            if ("" != dialNumber) {
                redialNumber = dialNumber;
            }
            console.log("[BluetoothDialPage] bluetoothDialPage.btPhoneCall()");
            bluetoothDialPage.phoneCallRequest(dialNumber);
        }
    }

    SysBtn {
        id:bt_redial_btn
        width:106
        height:133
        anchors.top: bt_dialout_btn.top
        anchors.topMargin: 141
        anchors.left:dialpan.right
        anchors.leftMargin: 8
        picNormal: "/image/dailinterfaceView/recall.png"
        picPressed: "/image/dailinterfaceView/recall_d.png"
        onClicked: {
            console.log("[BluetoothDialPage] recall_btn clicked");
            console.log("[BluetoothDialPage] redial_number = "+redialNumber);
            console.log("[BluetoothDialPage] bluetoothDialPage.btPhoneRecall()");
            dialNumber = redialNumber;
            bluetoothDialPage.phoneRecallRequest(redialNumber);
        }
    }

    Grid {
        id:dialpan
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.top: phonebook_show.top
        anchors.topMargin: 89
        columns: 3
        spacing: 6
        columnSpacing: 6

        SysBtn {
            id:dial01
            width:141
            height:63
            picNormal: "/image/dailinterfaceView/1.png"
            picPressed: "/image/dailinterfaceView/1_d.png"
            onClicked: {
                if (dialNumber.length < 15) {
                    dialNumber += '1'
                    bluetoothDialPage.setDialNumber(dialNumber);
                } else {
                    notify_numlen();
                }
            }
        }

        SysBtn {
            id:dial02
            width:141
            height:63
            picNormal: "/image/dailinterfaceView/2.png"
            picPressed: "/image/dailinterfaceView/2_d.png"
            onClicked: {
                if (dialNumber.length < 15) {
                    dialNumber += '2'
                    bluetoothDialPage.setDialNumber(dialNumber);
                } else {
                    notify_numlen();
                }
            }
        }

        SysBtn {
            id:dial03
            width:141
            height:63
            picNormal: "/image/dailinterfaceView/3.png"
            picPressed: "/image/dailinterfaceView/3_d.png"
            onClicked: {
                if (dialNumber.length < 15) {
                    dialNumber += '3'
                    bluetoothDialPage.setDialNumber(dialNumber);
                } else {
                    notify_numlen();
                }
            }
        }

        SysBtn {
            id:dial04
            width:141
            height:63
            picNormal: "/image/dailinterfaceView/4.png"
            picPressed: "/image/dailinterfaceView/4_d.png"
            onClicked: {
                if (dialNumber.length < 15) {
                    dialNumber += '4'
                    bluetoothDialPage.setDialNumber(dialNumber);
                } else {
                    notify_numlen();
                }
            }
        }

        SysBtn {
            id:dial05
            width:141
            height:63
            picNormal: "/image/dailinterfaceView/5.png"
            picPressed: "/image/dailinterfaceView/5_d.png"
            onClicked: {
                if (dialNumber.length < 15) {
                    dialNumber += '5'
                    bluetoothDialPage.setDialNumber(dialNumber);
                } else {
                    notify_numlen();
                }
            }
        }

        SysBtn {
            id:dial06
            width:141
            height:63
            picNormal: "/image/dailinterfaceView/6.png"
            picPressed: "/image/dailinterfaceView/6_d.png"
            onClicked: {
                if (dialNumber.length < 15) {
                    dialNumber += '6'
                    bluetoothDialPage.setDialNumber(dialNumber);
                } else {
                    notify_numlen();
                }
            }
        }

        SysBtn {
            id:dial07
            width:141
            height:63
            picNormal: "/image/dailinterfaceView/7.png"
            picPressed: "/image/dailinterfaceView/7_d.png"
            onClicked: {
                if (dialNumber.length < 15) {
                    dialNumber += '7'
                    bluetoothDialPage.setDialNumber(dialNumber);
                } else {
                    notify_numlen();
                }
            }
        }

        SysBtn {
            id:dial08
            width:141
            height:63
            picNormal: "/image/dailinterfaceView/8.png"
            picPressed: "/image/dailinterfaceView/8_d.png"
            onClicked: {
                if (dialNumber.length < 15) {
                    dialNumber += '8'
                    bluetoothDialPage.setDialNumber(dialNumber);
                } else {
                    notify_numlen();
                }
            }
        }

        SysBtn {
            id:dial09
            width:141
            height:63
            picNormal: "/image/dailinterfaceView/9.png"
            picPressed: "/image/dailinterfaceView/9_d.png"
            onClicked: {
                if (dialNumber.length < 15) {
                    dialNumber += '9'
                    bluetoothDialPage.setDialNumber(dialNumber);
                } else {
                    notify_numlen();
                }
            }
        }

        SysBtn {
            id:dialxx
            width:141
            height:63
            picNormal: "/image/dailinterfaceView/xing_n.png"
            picPressed: "/image/dailinterfaceView/xing_d.png"
            onClicked: {
                if (dialNumber.length < 15) {
                    dialNumber += '*'
                    bluetoothDialPage.setDialNumber(dialNumber);
                } else {
                    notify_numlen();
                }
            }
        }

        SysBtn {
            id:dial00
            width:141
            height:63
            picNormal: "/image/dailinterfaceView/0.png"
            picPressed: "/image/dailinterfaceView/0_d.png"
            onClicked: {
                if (dialNumber.length < 15) {
                    dialNumber += '0'
                    bluetoothDialPage.setDialNumber(dialNumber);
                } else {
                    notify_numlen();
                }
            }
        }

        SysBtn {
            id:dialjj
            width:141
            height:63
            picNormal: "/image/dailinterfaceView/jing_n.png"
            picPressed: "/image/dailinterfaceView/jing_d.png"
            onClicked: {
                if (dialNumber.length < 15) {
                    dialNumber += '#'
                    bluetoothDialPage.setDialNumber(dialNumber);
                } else {
                    notify_numlen();
                }
            }
        }
    }

    function notify_numlen() {
        tips.text = qsTr("Number length > 15")
        tipbox.visible = true
        hidetipbox.start()
    }

    Connections {
        target: bluetoothDialPage
        onPhoneAudioDisconnect: {
            console.log("[BluetoothPairRecordsPage] onPhoneAudioDisconnect");
            tips.text = qsTr("Please connect the phone")
            tipbox.visible = true
            hidetipbox.start()
        }
    }

    Connections {
        target: bluetoothDialPage
        onDialNumberEmpty: {
            console.log("[BluetoothPairRecordsPage] onDialNumberEmpty");
            tips.text = qsTr("Dial number is empty")
            tipbox.visible = true
            hidetipbox.start()
        }
    }

    Connections {
        target: bluetoothDialPage
        onRedialNumberEmpty: {
            console.log("[BluetoothPairRecordsPage] onRedialNumberEmpty");
            tips.text = qsTr("Redial number is empty")
            tipbox.visible = true
            hidetipbox.start()
        }
    }
}


