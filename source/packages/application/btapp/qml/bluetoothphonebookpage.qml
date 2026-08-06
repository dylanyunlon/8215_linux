import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

Rectangle {
    id: bluetoothPhoneBookPageView
    width: 585
    height: 402
    radius: 12
    border.color:"#00000000"
    color:"#00000000"

    property string phonebookSearchString: bluetoothCallRecordsBooksPage.getPhoneBookSearchString();
    property bool busyIndicatorState: bluetoothCallRecordsBooksPage.getPhoneBooksUpdateState();
    property int  phonebookSize: bluetoothCallRecordsBooksPage.getPhoneBookListSize();
    property bool keyboardState: false;
    property bool refreshFinishState:  false;


    Component.onCompleted: {
        if ("" != phonebookSearchString) {
            phonebookText.text = phonebookSearchString;
            if (0 != phonebookSize) {
                var newtext = qsTr("A total of ") + phonebookSize + qsTr(" contacts");
                console.log("[bluetoothPhoneBooksPage] newtext = "+newtext.toString());
                phonebookText.placeholderText = newtext.toString();
            }
        } else if (0 == phonebookSize && false == busyIndicatorState) {
            phonebookText.placeholderText = qsTr("Search");
        } else {
            var newtext = qsTr("A total of ") + phonebookSize + qsTr(" contacts");
            console.log("[bluetoothPhoneBooksPage] newtext = "+newtext.toString());
            phonebookText.placeholderText = newtext.toString();

            var newtext2 = qsTr("Downloading... ") + phonebookSize;
            dialogs.text = newtext2.toString();
        }
    }

/*
    NumberAnimation { target: bluetoothPhoneBookPageView; property: "x";
        duration: 400; from: 732; to: 0;
        easing.type: Easing.OutBounce
        easing.amplitude: 0.1
        running: true }
*/

    Timer{
        id:hidetipbox
        interval: 2000
        repeat:false
        onTriggered: {
            tipbox.visible = false
        }
    }

    Image {
        source:"/image/btphonebooks/bluetooth_rightbox.png"
    }

    TextField {
        id: phonebookText
        focus: true
        font.pointSize: 12
        anchors.top: parent.top
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.leftMargin: 16
        height: 53
        width:425
        placeholderText: qsTr("Search")
        textColor:"white"

        Image {
            height: 53
            width:473
            source: "/image/btphonebooks/searchfocus.png"
            opacity: 0.4
            asynchronous: true
            cache: true
        }

        style: TextFieldStyle {
            background: Rectangle {
                radius: 10
                implicitHeight: 53
                implicitWidth:425
                color:"#00000000"
            }
            placeholderTextColor:"white"
        }

        MouseArea {
            height: 53
            width: 385
            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 40
            visible: !keyboardState
            opacity: 0.1
            onClicked: {
                if (false == keyboardState) {
                    console.log("[bluetoothPhoneBooksPage] btn clicked");
                    phonebookOverlay.visible = true;
                    console.log("[bluetoothPhoneBooksPage] phonebookOverlay.visible:"+phonebookOverlay.visible);
                    var component = Qt.createComponent("/qml/keyboard.qml");
                    console.log("[bluetoothPhoneBooksPage] keyboard component:"+component.status+Component.Ready);
                    if (component.status == Component.Ready) {
                        console.log("[bluetoothPhoneBooksPage] creatObject:keyboard before");
                        var keyboardDialog = component.createObject(phonebookOverlay,{click_string:phonebookText.text,"anchors.topMargin":272});
                        console.log("[bluetoothPhoneBooksPage] creatObject:keyboard after");
                        keyboardDialog.sigClickString.connect(doPhonebookText);
                        keyboardDialog.sigKeyboardClose.connect(doCloseKeyboard);
                    }
                    keyboardState = true;
                }
            }
        }
    }

    function doPhonebookText(str) {
        phonebookText.text = str;
        //bluetoothCallRecordsBooksPage.phoneBookSearchString(str);
    }

    function doCloseKeyboard() {
        keyboardState = false;
        phonebookOverlay.visible = false;
        refreshFinishState = false;
        console.log(" [BluetoothPhoneBooksPage] refreshFinishState = "+refreshFinishState);
        if ("" == phonebookText.text) {
            console.log(" [BluetoothPhoneBooksPage] phoneBookListShow");
            bluetoothCallRecordsBooksPage.phoneBookListShow();
        } else {
            if (-1 == phoneListView.currentIndex && 0 == bluetoothCallRecordsBooksPage.getPhoneBookSize()) {
                tips.text = qsTr("Download phonebook first");
                tipbox.visible = true;
                hidetipbox.start();
            } else {
                console.log(" [BluetoothPhoneBooksPage] phoneBookSearchByString");
                bluetoothCallRecordsBooksPage.phoneBookSearchByString(phonebookText.text);
            }
        }
    }

    SysBtn {
        id: phonebook_search_btn
        width:28
        height:28
        anchors.left:phonebookText.right
        anchors.leftMargin:8
        anchors.verticalCenter:phonebookText.verticalCenter

        picNormal: "/image/btphonebooks/searchicon.png"
        picPressed: "/image/btphonebooks/searchicon2.png"

        onClicked:  {
            if (-1 == phoneListView.currentIndex && 0 == bluetoothCallRecordsBooksPage.getPhoneBookSize()) {
                tips.text = qsTr("Download phonebook first")
                tipbox.visible = true
                hidetipbox.start()
            } else if ("" == phonebookText.text) {
                tips.text = qsTr("Please input character")
                tipbox.visible = true
                hidetipbox.start()
            } else {
                refreshFinishState = false
                console.log(" [BluetoothPhoneBooksPage] refreshFinishState = "+refreshFinishState);
                console.log(" [BluetoothPhoneBooksPage] phoneBookSearchByString");
                bluetoothCallRecordsBooksPage.phoneBookSearchByString(phonebookText.text);
            }
        }


    }

    Rectangle{
        id: tipbox
        anchors.left: phonebookBox.left
        anchors.leftMargin: 81
        anchors.top:phonebookBox.top
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
        id: phonebookBox
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.top:parent.top
        anchors.topMargin: 80
        height: 304
        width: 473
        color:"#00000000"

        Image {
            height: 304
            width: 473
            source: "/image/btphonebooks/phonebookbox.png"
            opacity: 0.4
            asynchronous: true
            cache: true
        }

        ScrollView {
            height: 304
            width: 473
            flickableItem.interactive: true

            ListView {
                id: phoneListView
                anchors.fill: parent
                clip:true
                model:bluetoothPhoneBookModel
                delegate: bluetoothPhoneBookDelegate

                property real contentYOnFlickStarted: 0
                onFlickStarted: {
                    contentYOnFlickStarted = contentY;
                }
                onFlickEnded: {
                    if (false == refreshFinishState) {
                        console.log(" [BluetoothPhoneBooksPage] phoneBookListRefreshRequest");
                        bluetoothCallRecordsBooksPage.phoneBookListRefreshRequest();
                    } else {
                        console.log(" [BluetoothPhoneBooksPage] cannot refresh phonebook");
                    }
                }
            }

            style: ScrollViewStyle {
                transientScrollBars: true
                handle: Item {
                    implicitWidth: 16
                    implicitHeight: 20
                    Rectangle {
                        color: "#BFBFBF"
                        anchors.fill: parent
                        anchors.topMargin: 6
                        anchors.leftMargin: 4
                        anchors.rightMargin: 4
                        anchors.bottomMargin: 6
                     }
               }

               scrollBarBackground: Item {
                    implicitWidth: 16
                    implicitHeight: 20
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
        id: phonebook_call_btn
        width:64
        height:116
        anchors.top: parent.top
        anchors.topMargin: 105
        anchors.left: phonebookBox.right
        anchors.leftMargin: 16
        picNormal: "/image/btphonebooks/callexist.png"
        picPressed: "/image/btphonebooks/callnone.png"

        onClicked:  {
            console.log(" [BluetoothPhoneBooksPage] phoneBookCallOutRequest");
            console.log(" [BluetoothPhoneBooksPage] currentIndex = "+ phoneListView.currentIndex);
            bluetoothCallRecordsBooksPage.phoneBookCallOutRequest(phoneListView.currentIndex);
            if (-1 == phoneListView.currentIndex) {
                tips.text = qsTr("Download phonebook first");
                tipbox.visible = true;
                hidetipbox.start();
            }
        }
    }

    SysBtn {
        id: phonebook_updatepause_btn
        width:64
        height:116
        anchors.top: phonebook_call_btn.top
        anchors.topMargin: 140
        anchors.left: phonebookBox.right
        anchors.leftMargin: 16
        picNormal: "/image/btphonebooks/downexist.png"
        picPressed: "/image/btphonebooks/downnone.png"
        opacity: busyIndicatorState ? 0.45 : 1

        onClicked: {
            refreshFinishState = false;
            console.log(" [BluetoothPhoneBooksPage] phoneBooksUpdatePauseRequest");
            bluetoothCallRecordsBooksPage.phoneBooksUpdatePauseRequest();
        }
    }

    Component {
        id: bluetoothPhoneBookDelegate
        //Row {
        Item {
            id: bluetoothPhoneBookItem
            //spacing: 1
            width: 473
            height: 48

            Rectangle {
                id: bluetoothPhoneBookItemRectangle
                width: 473
                height: 48
                radius: 1
                color:"#00000000"

                Text {
                    id: book_index
                    color: "white"
                    width: 73
                    font.pixelSize: 24
                    text: bluetoothPhoneBookIndex
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    elide:Text.ElideRight
                }

                Text {
                    id: book_name
                    color: "white"
                    width: 136
                    font.pixelSize: 24
                    text: bluetoothPhoneBookName
                    anchors.left: book_index.right
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    elide:Text.ElideRight
                }

                Text {
                    id:book_number
                    color: "white"
                    font.pixelSize: 24
                    width: 184
                    elide:Text.ElideRight
                    text: bluetoothPhoneBookNumber
                    anchors.left: book_name.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 8
                }

                Image {
                    width: 22
                    height: 29
                    id:record_type
                    anchors.left: book_number.right
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    source: {
                        if ("phone" == bluetoothPhoneBookType) "/image/btphonebooks/phone.png"
                        else if ("SIM" == bluetoothPhoneBookType) "/image/btphonebooks/SIM.png"
                    }
                    asynchronous: true
                    cache: true
                }
            }

            states: State {
                name: "Current"
                when: bluetoothPhoneBookItem.ListView.isCurrentItem
                PropertyChanges { target: bluetoothPhoneBookItemRectangle; color: "#BFBFBF"; opacity: 0.6 }
            }

            MouseArea {
                id: mouse
                anchors.fill: parent
                onClicked: {
                    console.log(" [BluetoothPhoneBooksPage] PhoneBooks currentIndex = "+index);
                    bluetoothPhoneBookItem.ListView.view.currentIndex = index;
                }
            }
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onPhoneBookDownloadStart: {
            console.log("[bluetoothPhoneBooksPage] onPhoneBookDownloadStart");
            busyIndicatorState = true;
            dialogbox.visible = true;
            phonebookText.text = "";

            var newtext = qsTr("A total of ") + 0 + qsTr(" contacts");
            phonebookText.placeholderText = newtext.toString();

            var newtext2 = qsTr("Downloading...");
            dialogs.text = newtext2.toString();
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onPhoneBookListSizeChanged: {
            console.log("[bluetoothPhoneBooksPage] onPhoneBookListSizeChanged");
            busyIndicatorState = true;
            dialogbox.visible = true;

            var newtext = qsTr("A total of ") + m_phoneBookListSize + qsTr(" contacts");
            phonebookText.placeholderText = newtext.toString();

            var newtext2 = qsTr("Downloading... ") + m_phoneBookListSize;
            dialogs.text = newtext2.toString();

            refreshFinishState = false;
            console.log(" [bluetoothPhoneBooksPage] refreshFinishState = "+refreshFinishState);
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onPhoneBookDownloadStop: {
            console.log("[bluetoothPhoneBooksPage] onPhoneBookDownloadStop");
            busyIndicatorState = false;
            dialogbox.visible = false;
            tips.text = qsTr("PhoneBook download stop");
            tipbox.visible = true;
            hidetipbox.start();
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onPhoneBookDownloadFinish: {
            console.log("[bluetoothPhoneBooksPage] onPhoneBookDownloadFinish");
            busyIndicatorState = false;
            dialogbox.visible = false;
            tips.text = qsTr("PhoneBook download finish");
            tipbox.visible = true;
            hidetipbox.start();
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onCallRecordsIsDownloading: {
            console.log(" [bluetoothPhoneBooksPage] onCallRecordsIsDownloading");
            tips.text = qsTr("CallRecords is downloading");
            tipbox.visible = true;
            hidetipbox.start();
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onPhoneBookRefreshFinish: {
            console.log(" [bluetoothPhoneBooksPage] onPhoneBookRefreshFinish");
            tips.text = qsTr("PhoneBook refresh finish");
            tipbox.visible = true;
            hidetipbox.start();
            refreshFinishState = true;
            console.log(" [bluetoothPhoneBooksPage] refreshFinishState = "+refreshFinishState);
        }
    }

    Connections {
        target: bluetoothCallRecordsBooksPage
        onPhoneBookSearchResultEmpty: {
            console.log(" [bluetoothPhoneBooksPage] onPhoneBookSearchResultEmpty");
            tips.text = qsTr("Search result empty");
            tipbox.visible = true;
            hidetipbox.start();
        }
    }

    Connections {
        target: bluetoothApplication
        onPhoneConnectStateChanged: {
            console.log("[bluetoothPhoneBooksPage] onPhoneConnectStateChanged, m_phoneConnectState = "+m_phoneConnectState);
            if (false == m_phoneConnectState) {
                phonebookOverlay.visible = false;
            }
        }
    }

    Connections {
        target: bluetoothApplication
        onMediaConnectStateChanged: {
            console.log("[bluetoothPhoneBooksPage] onMediaConnectStateChanged, m_mediaConnectState = "+m_mediaConnectState);
            if (false == m_mediaConnectState) {
                phonebookOverlay.visible = false;
                keyboardState = false;
            }
        }
    }
    Connections {
        target: bluetoothApplication
        onLanguageChanged: {
            console.log("[bluetoothPhoneBooksPage] onLanguageChanged");
            if ("" != phonebookSearchString) {
                phonebookText.text = phonebookSearchString;
                if (0 != phonebookSize) {
                    var newtext = qsTr("A total of ") + phonebookSize + qsTr(" contacts");
                    console.log("[bluetoothPhoneBooksPage] newtext = "+newtext.toString());
                    phonebookText.placeholderText = newtext.toString();
                }
            } else if (0 == phonebookSize && false == busyIndicatorState) {
                phonebookText.placeholderText = qsTr("Search");
            } else {
                var newtext = qsTr("A total of ") + phonebookSize + qsTr(" contacts");
                console.log("[bluetoothPhoneBooksPage] newtext = "+newtext.toString());
                phonebookText.placeholderText = newtext.toString();

                var newtext2 = qsTr("Downloading... ") + phonebookSize;
                dialogs.text = newtext2.toString();
            }
        }
    }
}


