import QtQuick 2.0
import QtQuick.Controls 1.3

Rectangle {
    id:bluetoothSettingPageView
    width: 732
    height: 503
    radius: 12
    border.color:"#00000000"
    color:"#00000000"

    property string bluetoothName: bluetoothSettingPage.getBluetoothName();
    property string bluetoothPassword: bluetoothSettingPage.getBluetoothPassword();
    property bool   bluetoothAutoAnswerState: bluetoothSettingPage.getAutoAnswerState();
    property bool   bluetoothAutoConnectState: bluetoothSettingPage.getAutoConnectState();
    property int   bluetoothPowerState: bluetoothSettingPage.getPowerState();

/*
    NumberAnimation { target: bluetoothSettingPageView; property: "x";
        duration: 400; from: 732; to: 0;
        easing.type: Easing.OutBounce
        easing.amplitude: 0.1
        running: true }
*/
    Image {
        source:"/image/btsetting/bluetooth_rightbox.png"
    }

    Text {
        id: bt_name
        anchors.bottom: line01.top
        anchors.bottomMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 50
        text: qsTr("BT NAME")
        color: "white"
        font.pixelSize: 30
    }

    Text {
        id:bt_name_text
        width:250
        anchors.bottom: line01.top
        anchors.bottomMargin: 20
        anchors.left: bt_name.right
        anchors.leftMargin: 100
        text: bluetoothName
        color: "white"
        font.pixelSize: 20
        elide:Text.ElideRight
    }

    SysBtn
    {
        id:bt_name_btn
        width:96
        height:34
        anchors.bottom: line01.top
        anchors.bottomMargin: 20
        anchors.right:parent.right
        anchors.rightMargin: 50
        picNormal: "/image/btsetting/right_btn_n.png"
        picPressed: "/image/btsetting/right_btn_d.png"
        onClicked: {
            console.log("[BluetoothSettingPage] bt_name_btn clicked")
            bluetoothSettingPage.clickNameButton();
        }
    }

    function doShowNameDialog() {
        var component = Qt.createComponent("/qml/bluetoothdevicename.qml");
        console.log("[BluetoothSettingPage] name component:"+component.status+Component.Ready)
        if (component.status == Component.Ready) {
            console.log("[BluetoothSettingPage] creatObject:btname before")
            var nameDialog = component.createObject(bluetoothapplication,{btname:bluetoothName});
            console.log("[BluetoothSettingPage] creatObject:btname after")
            nameDialog.setNameClicked.connect(changeNameText);
        }
    }

    function changeNameText(msg_name) {
        console.log("[BluetoothSettingPage] msg_name:"+msg_name)
        bluetoothSettingPage.setBluetoothName(msg_name)
    }

    BorderImage {
        id:line01
        anchors.top: parent.top
        anchors.topMargin: 110
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: 0.35
        source: "/image/btsetting/right_divider35%.png"
        asynchronous: true
        cache: true
    }

    Text {
        id:pin_code
        anchors.bottom: line02.top
        anchors.bottomMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 50
        text: qsTr("PIN Code")
        color: "white"
        font.pixelSize: 30
    }

    Text {
        id:bt_passwd_text
        width:250
        anchors.bottom: line02.top
        anchors.bottomMargin: 20
        anchors.left: pin_code.right
        anchors.leftMargin: 100
        text: bluetoothPassword
        color: "white"
        font.pixelSize: 28
        elide:Text.ElideRight
    }

    SysBtn {
        id:pin_code_btn
        width:96
        height:34
        anchors.bottom: line02.top
        anchors.bottomMargin: 20
        anchors.right:parent.right
        anchors.rightMargin: 50
        picNormal: "/image/btsetting/right_btn_n.png"
        picPressed: "/image/btsetting/right_btn_d.png"
        onClicked: {
            console.log("[BluetoothSettingPage] pin_code_btn clicked");
            bluetoothSettingPage.clickPasswordButton();
        }
    }

    function doShowPasswdDialog() {
        var component = Qt.createComponent("/qml/bluetoothpincode.qml");
        console.log("[BluetoothSettingPage] passwd component:"+component.status+Component.Ready)
        if (component.status == Component.Ready) {
            console.log("[BluetoothSettingPage] creatObject:btpasswd");
            var passwdDialog = component.createObject(bluetoothapplication,{btpassword:bluetoothPassword});
            passwdDialog.setPasswordClicked.connect(changePasswdText);
        }
    }

    function changePasswdText(msg_passwd) {
        console.log("[BluetoothSettingPage] msg_passwd:"+msg_passwd+" ->btsetBtPasswd")
        bluetoothSettingPage.setBluetoothPassword(msg_passwd);
    }

    BorderImage {
        id:line02
        anchors.top: parent.top
        anchors.topMargin: 200
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: 0.35
        source: "/image/btsetting/right_divider35%.png"
        asynchronous: true
        cache: true
    }

    Text {
        id:auto_answer
        anchors.bottom: line03.top
        anchors.bottomMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 50
        text: qsTr("Auto Answer")
        color: "white"
        font.pixelSize: 30
    }

    Rectangle {
        id:auto_answer_btn
        width:96
        height:34
        anchors.bottom: line03.top
        anchors.bottomMargin: 20
        anchors.right:parent.right
        anchors.rightMargin: 50
        radius: 10

        Image {
            anchors.fill: parent
            source: bluetoothAutoAnswerState ? "/image/btsetting/on.png":"/image/btsetting/off.png"
            asynchronous: true
            cache: true
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("[BluetoothSettingPage] setAutoAnswer");
                bluetoothSettingPage.setAutoAnswer();
            }
        }

    }

    BorderImage {
        id:line03
        anchors.top: parent.top
        anchors.topMargin: 290
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: 0.35
        source: "/image/btsetting/right_divider35%.png"
        asynchronous: true
        cache: true
    }

    Text {
        id: auto_connect
        anchors.bottom: line04.top
        anchors.bottomMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 50
        text: qsTr("Auto Connect")
        color: "white"
        font.pixelSize: 30
    }

    Rectangle {
        id:auto_connect_btn
        width:96
        height:34
        anchors.bottom: line04.top
        anchors.bottomMargin: 20
        anchors.right:parent.right
        anchors.rightMargin: 50
        radius: 10

        Image {
            anchors.fill: parent
            source: bluetoothAutoConnectState ? "/image/btsetting/on.png" : "/image/btsetting/off.png"
            asynchronous: true
            cache: true
        }

        MouseArea {
            anchors.fill: parent
            onClicked:{
                console.log("[BluetoothSettingPage] setAutoConnect");
                bluetoothSettingPage.setAutoConnect();
            }
        }
    }

    BorderImage {
        id:line04
        anchors.top: parent.top
        anchors.topMargin: 380
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: 0.35
        source: "/image/btsetting/right_divider35%.png"
        asynchronous: true
        cache: true
    }

    Text {
        id: bt_power
        anchors.bottom: line05.top
        anchors.bottomMargin: 20
        anchors.left: parent.left
        anchors.leftMargin: 50
        text: qsTr("BT Power")
        color: "white"
        font.pixelSize: 30
    }

    Rectangle {
        id:bt_power_btn
        width:96
        height:34
        anchors.bottom: line05.top
        anchors.bottomMargin: 20
        anchors.right:parent.right
        anchors.rightMargin: 50
        radius: 10

        Image {
            anchors.fill: parent
            source: bluetoothPowerState == 1 ? "/image/btsetting/on.png" : (bluetoothPowerState == 0 ?
                                            "/image/btsetting/off.png" : "/image/btsetting/openingorclosing.png")
            asynchronous: true
            cache: true
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("[BluetoothSettingPage] setBluetoothPower");
                if (bluetoothSettingPage.isCarplayEnable() && bluetoothPowerState == 1) {
                    var component = Qt.createComponent("/qml/ConfirmCloseDialog.qml");
                    if (component.status === Component.Ready) {
                        var dialog = component.createObject(bluetoothapplication, {title:"Close bluetooth?", contentText:"Carplay is being used,turn off bluetooth may affect the use of carplay.Are you sure to close bluetooth?"});
                            dialog.sigConfirmClose.connect(function() {
                                bluetoothSettingPage.setBluetoothPower();
                        });
                    }
                } else {
                    bluetoothSettingPage.setBluetoothPower();
                }
            }
        }
    }

    BorderImage {
        id:line05
        anchors.top: parent.top
        anchors.topMargin: 470
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: 0.35
        source: "/image/btsetting/right_divider35%.png"
        asynchronous: true
        cache: true
    }

    Connections {
        target: bluetoothSettingPage
        onShowNameDialog: {
            console.log("[BluetoothSettingPage] onShowNameDialog");
            doShowNameDialog();
        }
    }

    Connections {
        target: bluetoothSettingPage
        onShowPasswdDialog: {
            console.log("[BluetoothSettingPage] onShowPasswdDialog");
            doShowPasswdDialog();
        }
    }

    Connections {
        target: bluetoothSettingPage
        onAutoAnswerStateChanged: {
            console.log("[BluetoothSettingPage] onAutoAnswerStateChanged, m_autoAnswer"+m_autoAnswer);
            bluetoothAutoAnswerState = m_autoAnswer;
        }
    }

    Connections {
        target: bluetoothSettingPage
        onAutoConnectStateChanged: {
            console.log("[BluetoothSettingPage] onAutoConnectStateChanged, m_autoConnect"+m_autoConnect);
            bluetoothAutoConnectState = m_autoConnect;
        }
    }

    Connections {
        target: bluetoothSettingPage
        onBluetoothPowerStateChanged: {
            console.log("[BluetoothSettingPage] onBluetoothPowerStateChanged, m_bluetoothPower"+m_bluetoothPower);
            bluetoothPowerState = m_bluetoothPower
        }
    }

    Connections {
        target: bluetoothSettingPage
        onBluetoothNameChanged: {
            console.log("[BluetoothSettingPage] onBluetoothNameChanged, m_bluetoothName = "+m_bluetoothName);
            bluetoothName = m_bluetoothName;
        }
    }

    Connections {
        target: bluetoothSettingPage
        onBluetoothPasswordChanged: {
            console.log("[BluetoothSettingPage] onBluetoothPasswordChanged, m_bluetoothPassword = "+m_bluetoothPassword);
            bluetoothPassword = m_bluetoothPassword
        }
    }

    Connections {
        target: bluetoothApplication
        onLanguageChanged: {
            console.log("[BluetoothSettingPage] onLanguageChanged");
            bt_name.text = qsTr("BT NAME")
            pin_code.text = qsTr("PIN Code")
            auto_answer.text = qsTr("Auto Answer")
            auto_connect.text = qsTr("Auto Connect")
            bt_power.text = qsTr("BT Power")
        }
    }
}

