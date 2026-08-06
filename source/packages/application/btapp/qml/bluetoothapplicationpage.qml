import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQml.Models 2.2

Rectangle {
    id:bluetoothapplicationpage
    anchors.top: parent.top
    width: parent.width
    height: parent.height
    color:"#00000000"

    // Add focus support for keyboard events
    focus: true

    Component.onCompleted: {
        forceActiveFocus()
    }

    // Keyboard press event handling
    Keys.onPressed: {
        console.log("[BluetoothApplication] Key pressed: " + event.key);
        switch(event.key) {
            case 49: // OK/Enter
                console.log("[BluetoothApplication] OK key pressed");
                handleOkKey();
                event.accepted = true;
                break;
            case 50: // Up
                console.log("[BluetoothApplication] Up key pressed");
                handleUpKey();
                event.accepted = true;
                break;
            case 51: // Back
                console.log("[BluetoothApplication] Back key pressed");
                bluetoothApplication.goExit();
                event.accepted = true;
                break;
            case 52: // Left
                console.log("[BluetoothApplication] Left key pressed");
                handleLeftKey();
                event.accepted = true;
                break;
            case 53: // Down
                console.log("[BluetoothApplication] Down key pressed");
                handleDownKey();
                event.accepted = true;
                break;
            case 54: // Right
                console.log("[BluetoothApplication] Right key pressed");
                handleRightKey();
                event.accepted = true;
                break;
        }
    }

    // Keyboard release event handling
    Keys.onReleased: {
        console.log("[BluetoothApplication] Key released: " + event.key);
        event.accepted = true;
    }

    // Handle OK/Enter key
    function handleOkKey() {
        if (focusArea === 1) {
            // Execute topbar button action
            if (topbarButtonIndex === 0) {
                // Home button
                console.log("[BluetoothApplication] Home button activated");
                bluetoothApplication.goHome();
            } else {
                // Back button
                console.log("[BluetoothApplication] Back button activated");
                bluetoothApplication.goExit();
            }
        } else {
            // Trigger click event for currently selected item
            var currentIndex = btListView.currentIndex;
            if (currentIndex >= 0 && currentIndex < btModel.count) {
                // Simulate click on current selected menu item
                simulateMenuClick(currentIndex);
            }
        }
    }

    // Handle Up key
    function handleUpKey() {
        if (focusArea === 0) {
            // Currently in menu list
            if (btListView.currentIndex > 0) {
                btListView.currentIndex--;
            } else {
                // At first item, move focus to topbar
                focusArea = 1;
                topbarButtonIndex = 0;
                highlightTopbarButton();
            }
        } else {
            // Already in topbar, do nothing or wrap to menu
            focusArea = 0;
            btListView.currentIndex = btModel.count - 1;
            clearTopbarHighlight();
        }
    }

    // Handle Down key
    function handleDownKey() {
        if (focusArea === 0) {
            // Currently in menu list
            if (btListView.currentIndex < btModel.count - 1) {
                btListView.currentIndex++;
            } else {
                btListView.currentIndex = 0; // Wrap to first item
            }
        } else {
            // In topbar, move focus back to menu list
            focusArea = 0;
            clearTopbarHighlight();
        }
    }

    // Handle Left key
    function handleLeftKey() {
        if (focusArea === 1) {
            // In topbar, switch to home button
            topbarButtonIndex = 0;
            highlightTopbarButton();
        } else {
            // Navigate back to previous page
            if (rootStackView.depth > 1) {
                rootStackView.pop();
            }
        }
    }

    // Handle Right key
    function handleRightKey() {
        if (focusArea === 1) {
            // In topbar, switch to back button
            topbarButtonIndex = 1;
            highlightTopbarButton();
        } else {
            // Right key acts as OK/Enter
            handleOkKey();
        }
    }

    // Handle Back key
    function handleBackKey() {
        // Back key: pop page if there is history
        if (rootStackView.depth > 1) {
            rootStackView.pop();
        }
    }

    // Simulate menu item click
    function simulateMenuClick(index) {
        console.log("[BluetoothApplication] simulateMenuClick index = " + index);
        if ((dialPageIndex == index || phoneBookPageIndex == index || callRecordPageIndex == index)
            && true == phoneConnectState) {
            console.log("[BluetoothApplication] phoneConnectState is true");
            if (previousIndex != index) {
                rootStackView.clear();
                btListView.currentIndex = index;
                rootStackView.push(Qt.resolvedUrl(btModel.get(index).page));
                previousIndex = index;
                bluetoothApplication.setPageIndex(index);
            }
        } else if ((dialPageIndex == index || phoneBookPageIndex == index || callRecordPageIndex == index)
            && false == phoneConnectState) {
            console.log("[BluetoothApplication] phoneConnectState is false");
            notify_to_connect_phone();
        } else if (musicPageIndex == index && true == mediaConnectState) {
            console.log("[BluetoothApplication] mediaConnectState is true");
            if (previousIndex != index) {
                rootStackView.clear();
                btListView.currentIndex = index;
                rootStackView.push(Qt.resolvedUrl(btModel.get(index).page));
                previousIndex = index;
                bluetoothApplication.setPageIndex(index);
            }
        } else if (musicPageIndex == index && false == mediaConnectState) {
            console.log("[BluetoothApplication] mediaConnectState is false");
            notify_to_connect_music();
        } else if (pairRecordPageIndex == index || settingPageIndex == index){
            console.log("[BluetoothApplication] default");
            if (previousIndex != index) {
                rootStackView.clear();
                btListView.currentIndex = index;
                rootStackView.push(Qt.resolvedUrl(btModel.get(index).page));
                previousIndex = index;
                bluetoothApplication.setPageIndex(index);
            }
        }
    }

    property int  dialPageIndex: 0;
    property int  phoneBookPageIndex: 1;
    property int  callRecordPageIndex: 2;
    property int  musicPageIndex: 3;
    property int  pairRecordPageIndex: 4;
    property int  settingPageIndex: 5;

    property bool phoneConnectState: bluetoothApplication.getPhoneConnectState();
    property bool mediaConnectState: bluetoothApplication.getMediaConnectState();
    property int  previousIndex: pairRecordPageIndex;

    // Focus area: 0 = menu list, 1 = topbar (home/back buttons)
    property int focusArea: 0
    // Topbar button index: 0 = home, 1 = back
    property int topbarButtonIndex: 0

    Image {
        id: leftlist
        width:202
        height: 504
        source: "/image/btmainView/bluetooth_leftlist.png"
        anchors.top: parent.top
        anchors.topMargin: 13.5
        anchors.left:parent.left
        anchors.leftMargin:40
        asynchronous: true
        cache: true
        opacity: 1
    }

    Timer{
        id:hidetipbox
        interval: 2000
        repeat:false
        onTriggered: {
            tipbox.visible = false
            tipsboxonbtm.visible = false
        }
    }

    Rectangle{
        id: tipbox
        anchors.top: parent.top
        anchors.topMargin: 358
        anchors.left: parent.left
        anchors.leftMargin: 356
        height: 50
        width: 400
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
            font.pixelSize: 28
            text: ""
            anchors.centerIn: parent
            objectName: "tips"
        }
    }

    Rectangle{
        id: tipsboxonbtm
        anchors.left: parent.left
        anchors.leftMargin: 356
        anchors.top:parent.top
        anchors.topMargin: 418
        height: 50
        width: 400
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
            id: tipsonbtm
            color: "white"
            font.pixelSize: 28
            text: ""
            anchors.centerIn: parent
            objectName: "tips"
        }
    }


    StackView {
        id:rootStackView
        width: 732
        height: 503
        anchors.left: leftlist.right
        anchors.top: parent.top
        anchors.leftMargin: 10
        anchors.topMargin: 14
        initialItem: Qt.resolvedUrl("/qml/bluetoothpairrecordspage.qml");
        /*
        delegate: StackViewDelegate {
            function transitionFinished(properties) {
                properties.exitItem.opacity = 1;
            }
            pushTransition: StackViewTransition {
                PropertyAnimation {
                    target: enterItem
                    property: "opacity"
                    from: 0
                    to: 1
                }
                PropertyAnimation {
                    target: exitItem
                    property: "opacity"
                    from: 1
                    to: 0
                }
            }
        }
        */
    }

    Rectangle {
        id:group_item
        width:202
        height: 504
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 40
        anchors.topMargin: 13.5
        radius: 10
        color:"#00000000"

        Component {
            id: btdelegate
            Item {
                id: itemlist
                width: 202
                height: 84

                Rectangle {
                    id: rectlist
                    width: 202
                    height: 84
                    radius: 10
                    color:"#00000000"
                }

                states: State {
                    name: "Current"
                    when: itemlist.ListView.isCurrentItem
                    PropertyChanges {
                        target: rectlist;
                        color: "#BFBFBF"
                        opacity: 0.6
                    }
                }

                BorderImage {
                    id: btimage
                    source: image
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 15
                    asynchronous: true
                    cache: true
                }

                Text {
                    id: textitem
                    color: "white"
                    font.pixelSize: 25
                    text: name
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: btimage.right
                    anchors.leftMargin: 5
                    wrapMode: Text.Wrap
                }

                MouseArea {
                    id: mouse
                    anchors.fill: parent
                    onClicked: {
                        console.log("[BluetoothApplication] index = "+index)
                        if ((dialPageIndex == index | phoneBookPageIndex == index | callRecordPageIndex == index)
                            && true == phoneConnectState) {
                            console.log("[BluetoothApplication] phoneConnectState is true");
                            if (previousIndex != index) {
                                rootStackView.clear();
                                itemlist.ListView.view.currentIndex = index;
                                rootStackView.push(Qt.resolvedUrl(page));
                                previousIndex = index;
                                bluetoothApplication.setPageIndex(index);
                            }
                        } else if ((dialPageIndex == index | phoneBookPageIndex == index | callRecordPageIndex == index)
                            && false == phoneConnectState) {
                            console.log("[BluetoothApplication] phoneConnectState is false");
                            notify_to_connect_phone();
                        } else if (musicPageIndex == index && true == mediaConnectState) {
                            console.log("[BluetoothApplication] mediaConnectState is true");
                            if (previousIndex != index) {
                                rootStackView.clear();
                                itemlist.ListView.view.currentIndex = index;
                                rootStackView.push(Qt.resolvedUrl(page));
                                previousIndex = index;
                                bluetoothApplication.setPageIndex(index);
                            }
                        } else if (musicPageIndex == index && false == mediaConnectState) {
                            console.log("[BluetoothApplication] mediaConnectState is false");
                            notify_to_connect_music();
                        } else if (pairRecordPageIndex == index | settingPageIndex == index){
                            console.log("[BluetoothApplication] default");
                            if (previousIndex != index) {
                                rootStackView.clear();
                                itemlist.ListView.view.currentIndex = index;
                                rootStackView.push(Qt.resolvedUrl(page));
                                previousIndex = index;
                                bluetoothApplication.setPageIndex(index);
                            }
                        }
                    }
                }
            }
        }

        ListView {
            id: btListView
            focus: true
            anchors.fill: parent
            model:btModel
            delegate: btdelegate
            currentIndex: pairRecordPageIndex
        }

        ListModel {
            id: btModel
            ListElement {
                name: qsTr("Dial")
                image: "/image/btmainView/dail.png"
                page: "/qml/bluetoothdialpage.qml"
            }
            ListElement {
                name: qsTr("PhoneBook")
                image: "/image/btmainView/phonebook.png"
                page:  "/qml/bluetoothphonebookpage.qml"
            }
            ListElement {
                name: qsTr("CallRecord")
                image: "/image/btmainView/dialrecords.png"
                page:  "/qml/bluetoothcallrecordspage.qml"
            }
            ListElement {
                name: qsTr("Music")
                image: "/image/btmainView/bt_music.png"
                page:  "/qml/bluetoothmusicpage.qml"
            }
            ListElement {
                name: qsTr("PairRecord")
                image: "/image/btmainView/pairedconnect.png"
                page:  "/qml/bluetoothpairrecordspage.qml"
            }
            ListElement {
                name: qsTr("Setting")
                image: "/image/btmainView/bt_setting.png"
                page:  "/qml/bluetoothsettingpage.qml"
            }
        }
    }

    function notify_to_connect_phone() {
        tips.text = qsTr("Please connect the phone");
        tipbox.visible = true;
        hidetipbox.start();
    }

    function notify_to_connect_music() {
        tips.text = qsTr("Please connect the media");
        tipbox.visible = true;
        hidetipbox.start();
    }

    function notify_to_poweron_bt() {
        tips.text = qsTr("Please power on BT");
        tipbox.visible = true;
        hidetipbox.start();
    }

    Connections {
        target: bluetoothApplication
        onPopDialupInterface: {
            console.log("[BluetoothApplication] onPopDialupInterface");
            var component_dail = Qt.createComponent("/qml/bluetoothcallpage.qml");
            console.log("[BluetoothApplication] component:"+component_dail.status+Component.Ready);
            if (component_dail.status == Component.Ready) {
                console.log("[BluetoothApplication] creatObject");
                var phoneNumber = component_dail.createObject(bluetoothapplication,
                    {phone_number: "unkown", acceptCallState: m_acceptCallState});
            }
        }
    }

    Connections {
        target: bluetoothApplication
        onPhoneConnectStateChanged: {
            console.log("[BluetoothApplication] onPhoneConnectStateChanged, phoneConnectState = ", m_phoneConnectState);
            phoneConnectState = m_phoneConnectState;
            if ((true == phoneConnectState)) {
                tips.text = qsTr("PhoneAudio Connected");
                tipbox.visible = true;
                hidetipbox.start();
            } else if ((false == phoneConnectState) &&
                (musicPageIndex != btListView.currentIndex) &&
                (pairRecordPageIndex != btListView.currentIndex) &&
                (settingPageIndex != btListView.currentIndex)) {
                rootStackView.clear();
                btListView.currentIndex = pairRecordPageIndex;
                rootStackView.push(Qt.resolvedUrl("/qml/bluetoothpairrecordspage.qml"));
                previousIndex = pairRecordPageIndex;
                bluetoothApplication.setPageIndex(pairRecordPageIndex);
            }
        }
    }

    Connections {
        target: bluetoothApplication
        onMediaConnectStateChanged: {
            console.log("[BluetoothApplication] onMediaConnectStateChanged, mediaConnectState = ", m_mediaConnectState);
            mediaConnectState = m_mediaConnectState;
            if ((true == mediaConnectState)) {
                tips.text = qsTr("MediaAudio Connected");
                tipbox.visible = true;
                hidetipbox.start();
            } else if ((false == mediaConnectState) &&
                (musicPageIndex == btListView.currentIndex)) {
                rootStackView.clear();
                btListView.currentIndex = pairRecordPageIndex;
                rootStackView.push(Qt.resolvedUrl("/qml/bluetoothpairrecordspage.qml"));
                previousIndex = pairRecordPageIndex;
                bluetoothApplication.setPageIndex(pairRecordPageIndex);
            }
        }
    }

    Connections {
        target: bluetoothApplication
        onLanguageChanged: {
            console.log("[BluetoothApplication] onLanguageChanged");
            btModel.get(dialPageIndex).name = qsTr("Dial");
            btModel.get(phoneBookPageIndex).name = qsTr("PhoneBook");
            btModel.get(callRecordPageIndex).name = qsTr("CallRecord");
            btModel.get(musicPageIndex).name = qsTr("Music");
            btModel.get(pairRecordPageIndex).name = qsTr("PairRecord");
            btModel.get(settingPageIndex).name = qsTr("Setting");
        }
    }

    Connections {
        target: bluetoothApplication
        onEnterPage: {
            console.log("[BluetoothApplication] onEnterPage, index = " + index);
            rootStackView.clear();
            btListView.currentIndex = index;
            rootStackView.push(Qt.resolvedUrl(btModel.get(index).page));
            previousIndex = index;
            bluetoothApplication.setPageIndex(index);
        }
    }

    Connections {
        target: bluetoothApplication
        onNotifyToConnectPhone: {
            console.log("[BluetoothApplication] onNotifyToConnectPhone");
            notify_to_connect_phone();
        }
    }

    Connections {
        target: bluetoothSettingPage
        onNotifyToPowerOn: {
            console.log("[BluetoothApplication] onNotifyToPowerOn");
            notify_to_poweron_bt();
        }
    }

    Connections {
        target: bluetoothApplication
        onSigShowPhoneNoAnswer: {
            console.log(" [BluetoothApplication] onSigShowPhoneNoAnswer");
            tipsonbtm.text = qsTr("The phone didn't respond!")
            tipsboxonbtm.visible = true
            hidetipbox.start()
        }
    }

    Connections {
        target: bluetoothApplication
        onSigShowLinkLost: {
            console.log(" [BluetoothApplication] onSigShowLinkLost");
            tipsonbtm.text = qsTr("The link lost!")
            tipsboxonbtm.visible = true
            hidetipbox.start()
        }
    }

    // Highlight topbar button (send signal to topbar)
    function highlightTopbarButton() {
        // Use signals to communicate with topbar
        sigHighlightTopbarButton(topbarButtonIndex);
    }

    // Clear topbar highlight
    function clearTopbarHighlight() {
        sigClearTopbarHighlight();
    }

    // Signals for topbar communication
    signal sigHighlightTopbarButton(int buttonIndex)
    signal sigClearTopbarHighlight()

}
