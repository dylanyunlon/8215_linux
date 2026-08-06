import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

Rectangle {
    id: bluetoothCallRecordsPageView
    width: 585
    height: 402
    radius: 12
    border.color:"#00000000"
    color:"#00000000"

    property bool busyIndicatorState: bluetoothCallRecordsBooksPage.getCallRecordsUpdateState();
    property bool incomingState: bluetoothCallRecordsBooksPage.getCallRecordsIncomingButtonState();
    property bool outgoingState: bluetoothCallRecordsBooksPage.getCallRecordsOutgoingButtonState();
    property bool missingState:  bluetoothCallRecordsBooksPage.getCallRecordsMissingButtonState();
    property bool refreshFinishState:  false;
    property int  callRecordsSize: bluetoothCallRecordsBooksPage.getCallRecordsListSize();
/*
    NumberAnimation { target: bluetoothCallRecordsPageView; property: "x";
        duration: 400; from: 732; to: 0;
        easing.type: Easing.OutBounce
        easing.amplitude: 0.1
        running: true }
*/

    Component.onCompleted: {
        if (true == busyIndicatorState) {
            var newtext = qsTr("Downloading... ") + callRecordsSize;
            dialogs.text = newtext.toString();
        }
    }

    Timer{
        id:hidetipbox
        interval: 2000
        repeat:false
        onTriggered: {
            tipbox.visible = false
        }
    }

    Image {
        source:"/image/btphonerecords/bluetooth_rightbox.png"
    }

    Row {
        anchors.left: parent.left
        anchors.leftMargin: 61
        anchors.top:parent.top
        anchors.topMargin: 8
        y:16
        spacing:40

         Rectangle{
            id:dialinto
            width:100
            height:62

            Image {
                anchors.fill: parent
                source: incomingState ? "/image/btphonerecords/innone.png" : "/image/btphonerecords/inexist.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    incomingState = true;
                    outgoingState = false;
                    missingState = false;
                    refreshFinishState = false;

                    bluetoothCallRecordsBooksPage.callRecordsIncomingListRequest();
                }
            }
        }

        Rectangle{
            id:dialout
            width:100
            height:62

            Image {
                anchors.fill: parent
                source: outgoingState ? "/image/btphonerecords/outnone.png" : "/image/btphonerecords/outexist.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    incomingState = false;
                    outgoingState = true;
                    missingState = false;
                    refreshFinishState = false;

                    bluetoothCallRecordsBooksPage.callRecordsOutgoingListRequest();
                }
            }
        }

        Rectangle{
            id:dialnotanswer
            width:100
            height:62

            Image {
                anchors.fill: parent
                source: missingState ? "/image/btphonerecords/missednone.png" : "/image/btphonerecords/missedexist.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    incomingState = false;
                    outgoingState = false;
                    missingState = true;
                    refreshFinishState = false;

                    bluetoothCallRecordsBooksPage.callRecordsMissingListRequest();
                }
            }
        }
    }

    Rectangle{
        id: tipbox
        anchors.left: callRecordsBox.left
        anchors.leftMargin: 81
        anchors.top:callRecordsBox.top
        anchors.topMargin: 132
        height: 40
        width: 312
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

    Rectangle {
        id: callRecordsBox
        width: 473
        height: 304
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.top:parent.top
        anchors.topMargin: 80
        color:"#00000000"

        Image {
            width:473
            height: 304
            source: "/image/btphonerecords/callrecordsbox.png"
            opacity: 0.4
        }

        ScrollView {
            height: 304
            width:473
            flickableItem.interactive: true

            ListView {
                id: callRecordsListView
                anchors.fill: parent
                model: bluetoothCallRecordsModel
                delegate: bluetoothCallRecordsDelegate
                clip: true

                property real contentYOnFlickStarted: 0
                onFlickStarted: {
                    contentYOnFlickStarted = contentY;
                }
                onFlickEnded: {
                    if (false == refreshFinishState) {
                        console.log(" [BluetoothCallRecordsPage] callRecordsCallOutRequest");
                        bluetoothCallRecordsBooksPage.callRecordsListRefreshRequest();
                    } else {
                        console.log(" [BluetoothPhoneBooksPage] cannot refresh callrecords");
                    }
                }
            }

            style: ScrollViewStyle {
                transientScrollBars: true
                handle: Item {
                    implicitWidth: 16
                    implicitHeight: 22
                    Rectangle {
                        color: "#BFBFBF"
                        anchors.fill: parent
                        anchors.topMargin: 4
                        anchors.leftMargin: 3
                        anchors.rightMargin: 3
                        anchors.bottomMargin: 4
                     }
               }

               scrollBarBackground: Item {
                    implicitWidth: 16
                    implicitHeight: 22
               }
            }
        }

        BusyIndicator {
            id:busyIndicator
            width:80
            height:80
            anchors.left: parent.left
            anchors.leftMargin: 117
            anchors.top:parent.top
            anchors.topMargin: 112
            running:busyIndicatorState

            Connections {
                target: bluetoothApplication
                onLanguageChanged: {
                    console.log("[BusyIndicator] onLanguageChanged");
                    if (true == busyIndicatorState) {
                        var newtext = qsTr("Downloading... ") + callRecordsSize;
                        dialogs.text = newtext.toString();
                    }
                }
            }
        }

        Rectangle{
            id: dialogbox
            anchors.left: parent.left
            anchors.leftMargin: 205
            anchors.top:parent.top
            anchors.topMargin: 132
            height: 40
            width: 160
            color: "#BFBFBF"
            visible: busyIndicatorState
            opacity: 0.8
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
                id: dialogs
                color: "white"
                font.pixelSize: 16
                text: ""
                anchors.centerIn: parent
                objectName: "dialogs"
            }
        }
    }

    SysBtn {
        id: callrecords_call_btn
        width:64
        height:116
        anchors.left: callRecordsBox.right
        anchors.leftMargin: 16
        anchors.top: parent.top
        anchors.topMargin: 105
        picNormal: "/image/btphonerecords/callexist.png"
        picPressed: "/image/btphonerecords/callnone.png"

        onClicked: {
            console.log(" [BluetoothCallRecordsPage] callRecordsCallOutRequest");
            console.log(" [BluetoothCallRecordsPage] currentIndex = "+callRecordsListView.currentIndex);
            bluetoothCallRecordsBooksPage.callRecordsCallOutRequest(callRecordsListView.currentIndex);
            if (-1 == callRecordsListView.currentIndex) {
                tips.text = qsTr("Download callrecords first");
                tipbox.visible = true;
                hidetipbox.start();
            }
        }
    }

    SysBtn {
        id: callrecords_updatepause_btn
        width:64
        height:116
        anchors.left: callRecordsBox.right
        anchors.leftMargin: 16
        anchors.top: callrecords_call_btn.top
        anchors.topMargin:140
        opacity: busyIndicatorState ? 0.45 : 1
        picNormal: "/image/btphonerecords/downexist.png"
        picPressed: "/image/btphonerecords/downnone.png"

        onClicked: {
            console.log(" [BluetoothCallRecordsBooksPage] callRecordsUpdatePauseRequest");
            bluetoothCallRecordsBooksPage.callRecordsUpdatePauseRequest();
        }
    }

    Component {
        id: bluetoothCallRecordsDelegate
        Item {
            id: bluetoothCallRecordsItem
            width: 473
            height: 48

            Rectangle {
                id: bluetoothCallRecordsItemRectangle
                width: 473
                height: 48
                color:"#00000000"
                radius: 2

                Text {
                    id: record_index
                    color: "white"
                    width: 72
                    font.pixelSize: 22
                    text: bluetoothCallRecordsIndex
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    elide:Text.ElideRight
                }

                Text {
                    width: 136
                    id: record_name
                    color: "white"
                    font.pixelSize: 24
                    text: bluetoothCallRecordsName
                    anchors.left: record_index.right
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    elide:Text.ElideRight
                }

                Text {
                    width: 184
                    id:record_num
                    anchors.left: record_name.right
                    anchors.leftMargin: 8
                    color: "white"
                    font.pixelSize: 24
                    text: bluetoothCallRecordsNumber
                    anchors.verticalCenter: parent.verticalCenter
                    elide:Text.ElideRight
                }

                Image {
                    width:25
                    height: 18
                    id:record_type
                    anchors.left: record_num.right
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    source: {
                        if ("incoming" == bluetoothCallRecordsType) "/image/btphonerecords/callin.png"
                        else if ("outgoing" == bluetoothCallRecordsType) "/image/btphonerecords/callout.png"
                        else if ("missing" == bluetoothCallRecordsType) "/image/btphonerecords/callmiss.png"
                    }
                    asynchronous: true
                    cache: true
                }
            }

            states: State {
                name: "Current"
                when: bluetoothCallRecordsItem.ListView.isCurrentItem
                PropertyChanges { target: bluetoothCallRecordsItemRectangle; color: "#BFBFBF"; opacity: 0.6 }
            }

            MouseArea {
                id: mouse
                anchors.fill: parent
                onClicked: {
                    console.log(" [BluetoothCallRecordsBooksPage] CallRecords currentIndex = "+index);
                    bluetoothCallRecordsItem.ListView.view.currentIndex = index;
                }
            }
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onCallRecordsDownloadStart: {
            console.log("[bluetoothCallRecordsPage] onCallRecordsDownloadStart");
            incomingState = false;
            outgoingState = false;
            missingState = false;
            busyIndicatorState = true;
            dialogbox.visible = true;
            var newtext = qsTr("Downloading...");
            console.log("[bluetoothCallRecordsPage] newtext = "+newtext.toString());
            dialogs.text = newtext.toString();
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onCallRecordsListSizeChanged: {
            console.log("[bluetoothCallRecordsPage] onCallRecordsListSizeChanged");
            dialogbox.visible = true;
            busyIndicatorState = true;
            var newtext = qsTr("Downloading... ") + m_callRecordsListSize;
            dialogs.text = newtext.toString();

            refreshFinishState = false;
            console.log(" [bluetoothCallRecordsPage] refreshFinishState = "+refreshFinishState);
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onCallRecordsDownloadStop: {
            console.log("[bluetoothCallRecordsPage] onCallRecordsDownloadStop");
            busyIndicatorState = false;
            dialogbox.visible = false;
            tips.text = qsTr("CallRecords download stop");
            tipbox.visible = true;
            hidetipbox.start();
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onCallRecordsDownloadFinish: {
            console.log("[bluetoothCallRecordsPage] onCallRecordsDownloadFinish");
            busyIndicatorState = false;
            dialogbox.visible = false;
            tips.text = qsTr("CallRecords download finish");
            tipbox.visible = true;
            hidetipbox.start();
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onPhoneBookIsDownloading: {
            console.log(" [bluetoothCallRecordsPage] onPhoneBookIsDownloading");
            tips.text = qsTr("PhoneBook is downloading");
            tipbox.visible = true;
            hidetipbox.start();
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onCallRecordsRefreshFinish: {
            console.log(" [bluetoothCallRecordsPage] onCallRecordsRefreshFinish");
            tips.text = qsTr("CallRecords refresh finish");
            tipbox.visible = true;
            hidetipbox.start();
            refreshFinishState = true;
            console.log(" [bluetoothCallRecordsPage] refreshFinishState = "+refreshFinishState);
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onCallRecordsStateChanged: {
            console.log(" [bluetoothCallRecordsPage] onCallRecordsStateChanged");
            incomingState = m_callRecordsIncomingButtonState;
            outgoingState = m_callRecordsOutgoingButtonState;
            missingState = m_callRecordsMissingButtonState;
        }
    }

    Connections {
        target: bluetoothApplication
        onLanguageChanged: {
            console.log("[BluetoothTopbar] onLanguageChanged");
            if (true == busyIndicatorState) {
                var newtext = qsTr("Downloading... ") + callRecordsSize;
                dialogs.text = newtext.toString();
            }
        }
    }
}



