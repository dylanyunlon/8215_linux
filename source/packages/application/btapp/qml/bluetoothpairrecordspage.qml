import QtQuick 2.0
import QtQuick.Controls 1.2

Rectangle {
    id:bluetoothPairRecordsPageView
    width: 732
    height: 503
    radius: 12
    border.color:"#00000000"
    color:"#00000000"

    property string conenctshow: "No Pair"
    property bool   pairItemState: true
    property bool   availableItemState: false
    property bool   busyIndicatorState: bluetoothPairRecordsPage.getScanState();
/*
    NumberAnimation { target: bluetoothPairRecordsPageView; property: "x";
        duration: 400; from: 732; to: 0;
        easing.type: Easing.OutBounce
        easing.amplitude: 0.1
        running: true }
*/
    Image {
        source:"/image/pairedConnect/bluetooth_rightbox.png"
    }

    Timer{
        id:hidetipbox
        interval: 2000
        repeat:false
        onTriggered: {
            tipbox.visible = false
        }
    }

    Rectangle {
        id:pairedDevice
        width:587
        height: 34
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top:parent.top
        anchors.topMargin: 25

        BorderImage {
            opacity: 0.45
            width:587
            height: 34
            source:"/image/pairedConnect/paired_divider45%.png"
            asynchronous: true
            cache: true
        }

        Text {
            id: pairedDeviceText
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Paired Device")
            color: "white"
            font.pixelSize: 20
            font.bold:true
        }
    }

    Rectangle{
        id: tipbox
        anchors.left: parent.left
        anchors.leftMargin: 130
        anchors.top:parent.top
        anchors.topMargin: 140
        height: 50
        width: 330
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
        }

        Text {
            id: tips
            color: "white"
            font.pixelSize: 28
            text: ""
            anchors.centerIn: parent
            objectName: "tips"
        }
    }

    Rectangle {
        id:pairedList
        width:587
        height: 200
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top:pairedDevice.top
        anchors.topMargin: 34
        color: "#00000000"

        Image {
            width:587
            height: 200
            source:"/image/pairedConnect/paired_box.png"
            opacity: 0.4
            asynchronous: true
            cache: true
        }

        ListView {
            id: bluetoothPairedList
            anchors.fill: parent
            clip:true
            model:bluetoothPairedDeviceModel
            delegate: bluetoothPairedDeviceDelegate
        }
    }

    Rectangle {
        id:availableDevice
        width:587
        height: 34
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top:pairedList.top
        anchors.topMargin: 200

        BorderImage {
            opacity: 0.45
            width:587
            height: 34
            source:"/image/pairedConnect/paired_divider45%.png"
            asynchronous: true
            cache: true
        }

        Text {
            id: availableDeviceText
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Available Device")
            color: "white"
            font.pixelSize: 20
            font.bold:true
        }
    }

    Rectangle {
        id:availableList
        width:587
        height: 180
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top:availableDevice.top
        anchors.topMargin: 34
        color:"#00000000"

        Image {
            width:587
            height: 180
            source:"/image/pairedConnect/available_box.png"
            opacity: 0.4
            asynchronous: true
            cache: true
        }

        ListView {
            id: bluetoothAvailableList
            anchors.fill: parent
            clip: true
            model:bluetoothAvailableDeviceModel
            delegate: bluetoothAvailableDeviceDelegate
        }

        BusyIndicator {
            id:busyIndicator
            width:100
            height:100
            anchors.centerIn:parent
            running:busyIndicatorState
        }
    }

    Component {
        id: bluetoothPairedDeviceDelegate
        Item {
            id: pairItem
            width:587
            height: 60

            states: State {
                name: "Current"
                when: (pairItemState == true) && (pairItem.ListView.isCurrentItem)
                PropertyChanges { target: background; color: "#BFBFBF"; opacity: 0.4 }
                PropertyChanges { target: slecteconnect; color: "#BFBFBF"; opacity: 0.4 }
            }

            Rectangle {
                id: bluetoothAvailableDeviceItem
                width:587
                height: 60
                color:"#00000000"

                Rectangle {
                    id: background
                    width: 539
                    height: 60
                    radius: 1
                    color:"#00000000"

                    Text {
                        id: textitem
                        color: "white"
                        font.pixelSize: 22
                        text: bluetoothPairedDeviceName
                        width: 315
                        elide: Text.ElideRight
                        anchors.left: background.left
                        anchors.leftMargin: 5
                        anchors.top:background.top
                        anchors.topMargin: 4
                    }

                    Text {
                        id: textaddress
                        color: "white"
                        font.pixelSize: 22
                        text: bluetoothPairedDeviceAddress
                        anchors.left: background.left
                        anchors.leftMargin: 325
                        anchors.top:background.top
                        anchors.topMargin: 4
                        opacity: 0.5
                    }

                    Text {
                        id: text_paired
                        color: "white"
                        font.pixelSize: 18
                        text: bluetoothPairedDeviceState
                        anchors.left: parent.left
                        anchors.leftMargin: 5
                        anchors.top:textitem.bottom
                        anchors.topMargin: 1
                    }

                    MouseArea {
                        id: mouse
                        anchors.fill: parent
                        onClicked: {
                            console.log("[BluetoothPairRecordsPage] pairItem currentIndex :"+index)
                            pairItem.ListView.view.currentIndex = index
                            pairItemState = true;
                            availableItemState = false;
                        }
                    }
                }

                Rectangle {
                    id: slecteconnect
                    width: 48
                    height: 60
                    anchors.left: background.right
                    color:"#00000000"

                    Image {
                        anchors.verticalCenter: parent.verticalCenter
                        source: "/image/pairedConnect/navigation_next_item.png"
                        asynchronous: true
                        cache: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            console.log("[BluetoothPairRecordsPage] pairItem currentIndex :"+index)
                            pairItem.ListView.view.currentIndex = index;
                            pairItemState = true;
                            availableItemState = false;

                            var component = Qt.createComponent("/qml/SelectConnect.qml");
                            console.log("[BluetoothPairRecordsPage] createComponent:"+component.status+Component.Ready)
                            if (component.status == Component.Ready) {
                                console.log("[BluetoothPairRecordsPage] creatObject")
                                bluetoothPairRecordsPage.openPairedItemSelectConnectBox(index);
                                var nameDialog = component.createObject(bluetoothPairRecordsPageView,
                                    {phonevalue:bluetoothPairedDevicePhoneAudioState,
                                     musicvalue:bluetoothPairedDeviceMediaAudioState});
                                nameDialog.sendConnectSelect.connect(pairedItemSelectConnect);
                            }
                        }
                    }
                }
            }
        }
    }

    function pairedItemSelectConnect(phone, music) {
        console.log("[BluetoothPairRecordsPage] pairedItemSelectConnect");
        if (true == phone && true == music) {
            console.log("[BluetoothPairRecordsPage] connect phone && media!");
        } else if (true == phone && false == music) {
            console.log("[BluetoothPairRecordsPage] only connect phone!");
        } else if (false == phone && true == music) {
            console.log("[BluetoothPairRecordsPage] only connect media!");
        } else {
            console.log("[BluetoothPairRecordsPage] do not connect!");
        }

        console.log("[BluetoothPairRecordsPage] phoneConnectState = "+phone);
        console.log("[BluetoothPairRecordsPage] mediaConnectState = "+music);

        bluetoothPairRecordsPage.setPairedItemSelectConnectState(bluetoothPairedList.currentIndex, phone, music);
    }

    Component {
        id: bluetoothAvailableDeviceDelegate
        Item {
            id: availableItem
            width:587
            height: 60

            states: State {
                name: "Current"
                when: (availableItemState == true) && (availableItem.ListView.isCurrentItem)
                PropertyChanges { target: background; color: "#BFBFBF"; opacity: 0.4 }
                PropertyChanges { target: slecteconnect; color: "#BFBFBF"; opacity: 0.4 }
            }

            Rectangle {
                id: bluetoothAvailableDeviceItem
                width:587
                height: 60
                color:"#00000000"

                Rectangle {
                    id: background
                    width: 539
                    height: 60
                    radius: 1
                    color:"#00000000"

                    Text {
                        id: textitem
                        color: "white"
                        font.pixelSize: 22
                        text: bluetoothAvailableDeviceName
                        width:315
                        elide: Text.ElideRight
                        anchors.left: background.left
                        anchors.leftMargin: 5
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        id: textaddress
                        color: "white"
                        font.pixelSize: 22
                        text: bluetoothAvailableDeviceAddress
                        anchors.left: background.left
                        anchors.leftMargin: 325
                        anchors.verticalCenter: parent.verticalCenter
                        opacity: 0.5
                    }
/*
                    Text {
                        id:text_paired
                        color: "white"
                        font.pixelSize: 18
                        text: bluetoothAvailableDeviceState
                        anchors.left: parent.left
                        anchors.leftMargin: 5
                        anchors.top:textitem.bottom
                        anchors.topMargin: 2
                    }
*/
                    MouseArea {
                        id: mouse
                        anchors.fill: parent
                        onClicked: {
                            console.log("[BluetoothPairRecordsPage] availableItem currentIndex :"+index)
                            availableItem.ListView.view.currentIndex = index;
                            pairItemState = false;
                            availableItemState = true;
                        }
                    }
                }

                Rectangle {
                    id: slecteconnect
                    width: 48
                    height: 60
                    anchors.left: background.right
                    color:"#00000000"

                    Image {
                        anchors.verticalCenter: parent.verticalCenter
                        source: "/image/pairedConnect/navigation_next_item.png"
                        asynchronous: true
                        cache: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            console.log("[BluetoothPairRecordsPage] availableItem currentIndex :"+index)
                            availableItem.ListView.view.currentIndex = index;
                            pairItemState = false;
                            availableItemState = true;

                            var component = Qt.createComponent("/qml/SelectConnect.qml");
                            console.log("[BluetoothPairRecordsPage] createComponent:"+component.status+Component.Ready)
                            if (component.status == Component.Ready) {
                                console.log("[BluetoothPairRecordsPage] creatObject")
                                var nameDialog = component.createObject(bluetoothPairRecordsPageView,
                                    {phonevalue: bluetoothAvailableDevicePhoneAudioState,
                                     musicvalue: bluetoothAvailableDeviceMediaAudioState});
                                nameDialog.sendConnectSelect.connect(availableItemSelectConnect);
                            }
                        }
                    }
                }
            }
        }
    }

    function availableItemSelectConnect(phone, music) {
        console.log("[BluetoothPairRecordsPage] availableItemSelectConnect");
        if (true == phone && true == music) {
            console.log("[BluetoothPairRecordsPage] connect phone && media!");
        } else if (true == phone && false == music) {
            console.log("[BluetoothPairRecordsPage] only connect phone!");
        } else if (false == phone && true == music) {
            console.log("[BluetoothPairRecordsPage] only connect media!");
        } else {
            console.log("[BluetoothPairRecordsPage] do not connect!");
        }

        console.log("[BluetoothPairRecordsPage] phoneConnectState = "+phone);
        console.log("[BluetoothPairRecordsPage] mediaConnectState = "+music);

        bluetoothPairRecordsPage.setAvailableItemSelectConnectState(bluetoothAvailableList.currentIndex, phone, music);
    }

    Column {
        anchors.left: pairedDevice.left
        anchors.leftMargin: 607
        anchors.top: parent.top
        anchors.topMargin: 50
        spacing: 20

        SysBtn {
            id:bt_search_btn
            width:85
            height:85
            picNormal: "/image/pairedConnect/SearchExist.png"
            picPressed: "/image/pairedConnect/SearchNone.png"

            onClicked: {
                console.log("[BluetoothPairRecordsPage] scanningRequest");
                bluetoothPairRecordsPage.scanningRequest();
            }
        }

        SysBtn {
            id:bt_connect_btn
            width:85
            height:85
            picNormal: "/image/pairedConnect/ConnectExist.png"
            picPressed: "/image/pairedConnect/ConnectNone.png"

            onClicked: {
                if ((0 != bluetoothPairedDeviceModel.count) && (true == pairItemState)) {
                    console.log("[BluetoothPairRecordsPage] connectRequest");
                    bluetoothPairRecordsPage.connectRequest(bluetoothPairedList.currentIndex);
                }
                if ((0 != bluetoothAvailableDeviceModel.count) && (true == availableItemState)) {
                    console.log("[BluetoothPairRecordsPage] pairingRequest");
                    bluetoothPairRecordsPage.pairingRequest(bluetoothAvailableList.currentIndex);
                }
            }
        }

        SysBtn {
            id:bt_disconnect_btn
            width:85
            height:85
            picNormal: "/image/pairedConnect/DisconnectExist.png"
            picPressed: "/image/pairedConnect/DisconnectNone.png"

            onClicked: {
                if (0 != bluetoothPairedDeviceModel.count && (true == pairItemState)) {
                    console.log("[BluetoothPairRecordsPage] disconnectRequest");
                    bluetoothPairRecordsPage.disconnectRequest(bluetoothPairedList.currentIndex);
                }
            }
        }

        SysBtn {
            id:bt_delete_btn
            width:85
            height:85
            picNormal: "/image/pairedConnect/DeleteExist.png"
            picPressed: "/image/pairedConnect/DeleteNone.png"
            onClicked: {
                if (0 != bluetoothPairedDeviceModel.count && (true == pairItemState)) {
                    console.log("[BluetoothPairRecordsPage] dispairRequest");
                    bluetoothPairRecordsPage.dispairRequest(bluetoothPairedList.currentIndex);
                }
            }
        }
    }

    Connections {
        target: bluetoothPairRecordsPage
        onWaitToScan: {
            console.log(" [BluetoothPairRecordsPage] onWaitToScan");
            tips.text = qsTr("Please wait a moment!")
            tipbox.visible = true
            hidetipbox.start()
        }
    }

    Connections {
        target: bluetoothPairRecordsPage
        onWaitToPower: {
            console.log(" [BluetoothPairRecordsPage] onWaitToPower");
            tips.text = qsTr("Wait bluetooth poweron!")
            tipbox.visible = true
            hidetipbox.start()
        }
    }

    Connections {
        target: bluetoothPairRecordsPage
        onRemindPairFail: {
            console.log(" [BluetoothPairRecordsPage] onRemindPairFail");
            tips.text = qsTr("Bluetooth Pairing Fail!")
            tipbox.visible = true
            hidetipbox.start()
        }
    }

     Connections {
        target: bluetoothPairRecordsPage
        onScanStateChanged: {
            console.log(" [BluetoothPairRecordsPage] onScanStateChanged");
            busyIndicatorState = m_scanState;
        }
    }

    Connections {
        target: bluetoothApplication
        onLanguageChanged: {
            console.log("[BluetoothPairRecordsPage] onLanguageChanged");
            pairedDeviceText.text = qsTr("Paired Device");
            availableDeviceText.text = qsTr("Available Device");
        }
    }

}


