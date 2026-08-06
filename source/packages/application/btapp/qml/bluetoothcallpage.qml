import QtQuick 2.0

Item {
    id: bluetoothCallPageView
    anchors.fill: parent;
    z:1;
    property string phoneName : qsTr("Unknown");
    property string phoneNumber : qsTr("");
    property bool   showKeyboard: false;
    property bool   hideCallPage: false;
    property bool   showPauseButton: false;
    property string dialNumber: "";
    property string dialDTMFCode: "";
    property bool   audioSourceInHF: bluetoothCallPage.getAudioSourceInHFState();
    property int    acceptCallState: notNeedAccept;
    property bool   holdCallState: false;
    property int    needAccept: 0;
    property int    notNeedAccept: 1;
    property int    needAcceptAndHold: 2;
    property int    terminateAndAcceptAction: 1;
    property int    switchCallAction: 2;
    property int    mergeCallAction: 3;
    property int    call_number: 0;
    property int    call_name: 1;
    property int    call_status: 3;
    property string accept_icon: "/image/dailinterfaceView/dialpopmenu/accept_phonecall.png";
    property string accept1_icon: "/image/dailinterfaceView/dialpopmenu/call_accept_terminate.png";
    property string accept2_icon: "/image/dailinterfaceView/answer_image.png";
    property string accept3_icon: "/image/dailinterfaceView/call_accept_terminate.png";
    property string hf_icon: "/image/dailinterfaceView/dialpopmenu/car_speaker.png";
    property string ag_icon: "/image/dailinterfaceView/dialpopmenu/phone_speaker.png";
    property string pause_icon: "/image/dailinterfaceView/dialpopmenu/call_held.png";
    property string hf_icon1: "/image/dailinterfaceView/car_speaker.png";
    property string ag_icon1: "/image/dailinterfaceView/phone_speaker.png";
    property string pause_icon1: "/image/dailinterfaceView/call_held.png";

    Rectangle {
        //anchors.fill: parent
        id: dialoverlay
        x: hideCallPage ? navigationCall.x :0
        width: hideCallPage ? navigationCall.width : 800
        height: hideCallPage ? navigationCall.height : 480
        color: "#000000"
        opacity: 0.3
        MouseArea {
            anchors.fill: parent
            onClicked: {
                hideCallPage = true;
            }
        }
    }

    Timer{
        id:autoAnswerTimer
        interval: 5000
        repeat:false
        onTriggered: {
            if (acceptCallState == needAccept) {
                console.log("[BluetoothCallPage] autoAnswerTimer.stop()");
                autoAnswerTimer.stop();
                bluetoothCallPage.autoAnswerTimerStop();
            }
        }
    }

    Rectangle {
        id: callmenu
        width: 323
        height: 373
        radius: 14
        x:200
        y:52
        z:1
        border.color: "black"
        visible: !hideCallPage

        MouseArea {
            anchors.fill: parent
            onClicked: {
                //hideCallPage = true;
            }
        }

        BorderImage  {
            anchors.fill: parent
            source:"/image/dailinterfaceView/dialpopmenu/popUp_base1.png"
        }

        BorderImage {
            id: picture_person
            width: 149
            height: 149
            source: "/image/dailinterfaceView/dialpopmenu/call_member.png"

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 12
        }

        Loader {
            id: callLoader
            anchors.top: picture_person.bottom
            anchors.topMargin: 8
            anchors.horizontalCenter: parent.horizontalCenter
            sourceComponent: bluetoothcallList.count == 1 ? callTextComponent : undefined
        }

        Rectangle {
            id: callList
            width: 304
            height: 105
            visible: bluetoothcallList.count > 1 ? true : false
            anchors.top: picture_person.bottom
            anchors.topMargin: 24
            anchors.horizontalCenter: parent.horizontalCenter
            color:"#00000000"
            ListView {
                id: bluetoothcallList
                anchors.fill: parent
                clip: true
                model:bluetoothCallListModel
                delegate: bluetoothCallDelegate
            }

        }

        Loader {
           anchors.top: callmenu.top
           anchors.left: callmenu.right
           sourceComponent: showKeyboard ? keyboardComponent : undefined
           MouseArea {
            anchors.fill: parent
            onClicked: {
            }
           }
        }

        Row {
            spacing: 8
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 12

            Rectangle {
                id: speaker
                width:93
                height:86
                radius: 10

                BorderImage {
                    anchors.fill: parent
                    source: acceptCallState == needAcceptAndHold ?  accept1_icon : (acceptCallState == needAccept ?  accept_icon : (showPauseButton ? pause_icon : (audioSourceInHF ?  hf_icon : ag_icon)))
                }

                Rectangle {
                    width:93
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 30
                    Text {
                        width:93
                        anchors.horizontalCenter: parent.horizontalCenter
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                        font.pixelSize: 20
                        text: acceptCallState == needAcceptAndHold ? qsTr("hangup_ans") : (acceptCallState == needAccept ? qsTr("answer") : (showPauseButton ? qsTr("hold") :(audioSourceInHF ? qsTr("carkit") : qsTr("phone"))))
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        switch (acceptCallState) {
                            case needAccept:
                            console.log("[BluetoothCallPage] bluetoothCallPage.acceptPhoneCall");
                            bluetoothCallPage.acceptPhoneCall();
                            break;

                            case notNeedAccept:
                            if (showPauseButton) {
                                bluetoothCallPage.holdPhoneCall(switchCallAction);
                                console.log("[BluetoothCallPage] bluetoothCallPage.switchHold");
                            }
                            else {
                                bluetoothCallPage.switchAudioSource(!audioSourceInHF);
                                console.log("[BluetoothCallPage] bluetoothCallPage.switchAudioSource");
                            }
                            break;

                            case needAcceptAndHold:
                            console.log("[BluetoothCallPage] termiante and accept");
                            bluetoothCallPage.holdPhoneCall(terminateAndAcceptAction);
                            break;
                        }

                    }
                }
            }

            OpacityBtn {

                id:hangup
                width:93
                height:86

                picNormal: "/image/dailinterfaceView/dialpopmenu/hangUp.png"
                picPressed: "/image/dailinterfaceView/dialpopmenu/btn_d.png"
                onClicked: {
                    console.log("[BluetoothCallPage] bluetoothCallPage.terminatePhoneCall");
                    bluetoothCallPage.terminatePhoneCall();
                }

                Rectangle {
                    width:93
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 30
                    Text {
                        width:93
                        anchors.horizontalCenter: parent.horizontalCenter
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                        font.pixelSize: 20
                        text: qsTr("hang up")
                    }
                }

            }

            Loader {
                id: swapLoader
                sourceComponent: (holdCallState && !showKeyboard) ? swapComponet : undefined
            }

             Loader {
                id: mergeLoader
                sourceComponent: (holdCallState && !showKeyboard)  ? mergeComponet : undefined
            }

            OpacityBtn {
                id:dailpan
                width:93
                height:86
                isStretch: true
                picNormal: acceptCallState == needAcceptAndHold ? "/image/dailinterfaceView/dialpopmenu/call_accept_hold.png" : "/image/dailinterfaceView/dialpopmenu/dial_pan.png"
                picPressed: acceptCallState == needAcceptAndHold ? "/image/dailinterfaceView/dialpopmenu/call_accept_hold_d.png" : "/image/dailinterfaceView/dialpopmenu/btn_d.png"
                onClicked: {
                    if (acceptCallState == needAcceptAndHold) {
                        bluetoothCallPage.holdPhoneCall(switchCallAction);

                    } else {
                        showKeyboard = !showKeyboard;
                        if (showKeyboard) {
                            callmenu.x = 123
                            showHoldButton(false);
                        }
                        else {
                            callmenu.x = 200
                            if (holdCallState)
                                showHoldButton(true);
                        }

                    }
                }

                Rectangle {
                    width:93
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 30
                    Text {
                        width:93
                        anchors.horizontalCenter: parent.horizontalCenter
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                        font.pixelSize: 20
                        text: acceptCallState == needAcceptAndHold ? qsTr("hold_ans") : qsTr("keyboard")
                    }
                }

            }
        }



    }

    Rectangle {
        id: navigationCall
        x:249
        width:320
        height: bluetoothcallList2.count > 3 ? 160 : 70 + bluetoothcallList2.count * 30
        color: "#808080"
        opacity: 1
        visible: hideCallPage

        ListView {
            id: bluetoothcallList2
            anchors.top: parent.top
            anchors.topMargin: 4
            width: 320
            height: bluetoothcallList2.count > 3 ? 72 : bluetoothcallList2.count * 24
            clip: true
            model:bluetoothCallListModel
            delegate: bluetoothCallDelegate
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                hideCallPage = false;
            }
        }



        Image {
            id: answerorterminate_image
            anchors.top: bluetoothcallList2.bottom
            x: 4
            width: 64
            height: 48
            BorderImage {
                    anchors.fill: parent
                    source: acceptCallState == needAcceptAndHold ?  accept3_icon : (acceptCallState == needAccept ?  accept2_icon : (showPauseButton ? pause_icon1 : (audioSourceInHF ?  hf_icon1 : ag_icon1)))
                }

           MouseArea {
            anchors.fill: parent
            onClicked: {
                switch (acceptCallState) {
                    case needAccept:
                    console.log("[BluetoothCallPage] bluetoothCallPage.acceptPhoneCall");
                    bluetoothCallPage.acceptPhoneCall();
                    break;

                    case notNeedAccept:
                    if (showPauseButton) {
                        bluetoothCallPage.holdPhoneCall(switchCallAction);
                        console.log("[BluetoothCallPage] bluetoothCallPage.switchHold");
                    }
                    else {
                        bluetoothCallPage.switchAudioSource(!audioSourceInHF);
                        console.log("[BluetoothCallPage] bluetoothCallPage.switchAudioSource");
                    }
                    break;

                    case needAcceptAndHold:
                    console.log("[BluetoothCallPage] termiante and accept");
                    bluetoothCallPage.holdPhoneCall(terminateAndAcceptAction);
                    break;
                }
                }
            }
        }

        Rectangle {
            id: terminateCall
            width: 64
            height: 48
            anchors.left: answerorterminate_image.right
            anchors.leftMargin: 12
            anchors.verticalCenter: answerorterminate_image.verticalCenter
            color: "#00000000"

            Image {
                id: terminate_image
                anchors.fill: parent
                anchors.verticalCenter: parent.verticalCenter
                source: "/image/dailinterfaceView/hangup_image.png";

            }

            MouseArea{
                anchors.fill: parent
                onPressed: {
                console.log(" [BluetoothCallNavigation] terminatePhoneCall");
                bluetoothCallPage.terminatePhoneCall();
                }
            }
        }

         Rectangle {
            id: holdOrswitchBtn
            width: 64
            height: 48
            anchors.left: terminateCall.right
            anchors.leftMargin: 12
            anchors.verticalCenter: answerorterminate_image.verticalCenter
            color: "#00000000"
            visible: (acceptCallState == needAcceptAndHold) || holdCallState

            Image {
                anchors.fill: parent
                anchors.verticalCenter: parent.verticalCenter
                source: acceptCallState == needAcceptAndHold  ? "/image/dailinterfaceView/call_accept_hold.png" : "/image/dailinterfaceView/call_swap.png";

            }

            MouseArea{
                anchors.fill: parent
                onPressed: {
                console.log(" [BluetoothCallNavigation] hold and accpect");
                bluetoothCallPage.holdPhoneCall(switchCallAction);
                }
            }
        }

         Rectangle {
            width: 64
            height: 48
            anchors.left: holdOrswitchBtn.right
            anchors.leftMargin: 12
            anchors.verticalCenter: answerorterminate_image.verticalCenter
            color: "#00000000"
            visible: holdCallState

            Image {
                anchors.fill: parent
                anchors.verticalCenter: parent.verticalCenter
                source: "/image/dailinterfaceView/call_merge.png"

            }

            MouseArea{
                anchors.fill: parent
                onPressed: {
                console.log(" [BluetoothCallNavigation] merger");
                bluetoothCallPage.holdPhoneCall(mergeCallAction);
                }
            }
         }
    }



    Component {
        id: callTextComponent
        Rectangle {
            Text {
                id:books_number
                width: 304
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                text: phoneNumber
                color: "white"
                font.pixelSize: 28
                elide:Text.ElideRight
            }

            Text {
                id: books_name
                width: 304
                anchors.top: books_number.bottom
                anchors.topMargin: 8
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                text: phoneName
                color: "white"
                font.pixelSize: 28
                elide:Text.ElideRight
            }
        }
    }

    Component {
         id: mergeComponet
         OpacityBtn {
            width:93
            height:86
            isStretch: true
            picNormal: "/image/dailinterfaceView/dialpopmenu/call_merge.png"
            picPressed: "/image/dailinterfaceView/dialpopmenu/call_merge_d.png"
            onClicked: {
                console.log("[BluetoothCallPage] merge");
                bluetoothCallPage.holdPhoneCall(mergeCallAction);
            }

            Rectangle {
                width:93
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 30
                Text {
                    width:93
                    anchors.horizontalCenter: parent.horizontalCenter
                    horizontalAlignment: Text.AlignHCenter
                    color: "white"
                    font.pixelSize: 20
                    text: qsTr("merge")
                }
            }

        }
    }

    Component {
        id: swapComponet
        OpacityBtn {
            width:93
            height:86
            isStretch: true
            picNormal: "/image/dailinterfaceView/dialpopmenu/call_swap.png"
            picPressed: "/image/dailinterfaceView/dialpopmenu/call_swap_d.png"
            onClicked: {
                console.log("[BluetoothCallPage] switch");
                bluetoothCallPage.holdPhoneCall(switchCallAction);
            }

            Rectangle {
                width:93
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 30
                Text {
                    width:93
                    anchors.horizontalCenter: parent.horizontalCenter
                    horizontalAlignment: Text.AlignHCenter
                    color: "white"
                    font.pixelSize: 20
                    text: qsTr("switch")
                }
            }
        }

    }

    Component {
        id: bluetoothCallDelegate

        Item {
            id: callItem
            width:304
            height: hideCallPage ? 24 : 32

            states: State {
                name: "Current"
                when: callItem.ListView.isCurrentItem
                //PropertyChanges { target: background; color: "#BFBFBF"; opacity: 0.4 }
            }

            Rectangle {
                id: bluetoothCallItem
                width:304
                height: 24
                color:"#00000000"

                Rectangle {
                    id: background
                    width: 304
                    height: 24
                    radius: 1
                    color:"#00000000"

                    Text {
                        id: callText
                        color: "white"
                        font.pixelSize: hideCallPage ? 20 : 24
                        text: bluetoothCallText
                        elide: Text.ElideRight
                        anchors.left: background.left
                        anchors.leftMargin: 16
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        id: callStatus
                        color: "white"
                        font.pixelSize: 17
                        text: bluetoothCallStatus
                        anchors.left: background.left
                        anchors.leftMargin: 216
                        anchors.verticalCenter: parent.verticalCenter
                        opacity: 0.5
                    }
                    /*
                    Text {
                        id:callAddress
                        color: "white"
                        font.pixelSize: 14
                        text: bluetoothCallAddress
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        anchors.top:callText.bottom
                        anchors.topMargin: 1
                    }*/
                }
            }
        }
     }



 Component {
    id: keyboardComponent
    Rectangle {
        width: 324
        height: 373
        radius: 14
        border.color: "black"
        visible: true

        Image {
            source:"/image/dailinterfaceView/dialpopmenu/popUp_base1.png"

        }

        Image {
            width:265
            height: 51
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 12
            source: "/image/dailinterfaceView/dialpopmenu/dial_show50%.png"
            opacity: 0.5

            Text {
                id:dail_number
                width:256
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 4
                horizontalAlignment: Text.AlignRight
                font.pixelSize: 20
                font.bold: true
                text: dialNumber
                color: "white"
                clip: true
                elide:Text.ElideLeft
            }
        }

        Grid {
            id:popdialpan
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 80
            columns: 3
            spacing: 4
            columnSpacing: 4

            OpacityBtn {
                id:pan1
                width:84
                height:63
                picNormal: "/image/dailinterfaceView/dialpopmenu/1.png"
                picPressed: "/image/dailinterfaceView/dialpopmenu/dial_pan_d.png"
                onClicked: {
                    dialNumber += '1';
                    dialDTMFCode = '1';
                    bluetoothCallPage.inputDTMFCode(dialDTMFCode);
                }
            }

            OpacityBtn {
                id:pan2
                width:84
                height:63
                picNormal: "/image/dailinterfaceView/dialpopmenu/2.png"
                picPressed: "/image/dailinterfaceView/dialpopmenu/dial_pan_d.png"
                onClicked: {
                    dialNumber += '2';
                    dialDTMFCode = '2';
                    bluetoothCallPage.inputDTMFCode(dialDTMFCode);
                }
            }

            OpacityBtn {
                id:pan3
                width:84
                height:63
                picNormal: "/image/dailinterfaceView/dialpopmenu/3.png"
                picPressed: "/image/dailinterfaceView/dialpopmenu/dial_pan_d.png"
                onClicked: {
                    dialNumber += '3';
                    dialDTMFCode = '3';
                    bluetoothCallPage.inputDTMFCode(dialDTMFCode);
                }
            }

            OpacityBtn {
                id:pan4
                width:84
                height:63
                picNormal: "/image/dailinterfaceView/dialpopmenu/4.png"
                picPressed: "/image/dailinterfaceView/dialpopmenu/dial_pan_d.png"
                onClicked: {
                    dialNumber += '4';
                    dialDTMFCode = '4';
                    bluetoothCallPage.inputDTMFCode(dialDTMFCode);
                }
            }

            OpacityBtn {
                id:pan5
                width:84
                height:63
                picNormal: "/image/dailinterfaceView/dialpopmenu/5.png"
                picPressed: "/image/dailinterfaceView/dialpopmenu/dial_pan_d.png"
                onClicked: {
                    dialNumber += '5';
                    dialDTMFCode = '5';
                    bluetoothCallPage.inputDTMFCode(dialDTMFCode);
                }
            }

            OpacityBtn {
                id:pan6
                width:84
                height:63
                picNormal: "/image/dailinterfaceView/dialpopmenu/6.png"
                picPressed: "/image/dailinterfaceView/dialpopmenu/dial_pan_d.png"
                onClicked: {
                    dialNumber += '6';
                    dialDTMFCode = '6';
                    bluetoothCallPage.inputDTMFCode(dialDTMFCode);

                }
            }

            OpacityBtn {
                id:pan7
                width:84
                height:63
                picNormal: "/image/dailinterfaceView/dialpopmenu/7.png"
                picPressed: "/image/dailinterfaceView/dialpopmenu/dial_pan_d.png"
                onClicked: {
                    dialNumber += '7';
                    dialDTMFCode = '7';
                    bluetoothCallPage.inputDTMFCode(dialDTMFCode);
                }
            }

            OpacityBtn {
                id:pan8
                width:84
                height:63
                picNormal: "/image/dailinterfaceView/dialpopmenu/8.png"
                picPressed: "/image/dailinterfaceView/dialpopmenu/dial_pan_d.png"
                onClicked: {
                    dialNumber += '8';
                    dialDTMFCode = '8';
                    bluetoothCallPage.inputDTMFCode(dialDTMFCode);
                }
            }

            OpacityBtn {
                id:pan9
                width:84
                height:63
                picNormal: "/image/dailinterfaceView/dialpopmenu/9.png"
                picPressed: "/image/dailinterfaceView/dialpopmenu/dial_pan_d.png"
                onClicked: {
                    dialNumber += '9';
                    dialDTMFCode = '9';
                    bluetoothCallPage.inputDTMFCode(dialDTMFCode);
                }
            }

            OpacityBtn {
                id:panx
                width:84
                height:63
                picNormal: "/image/dailinterfaceView/dialpopmenu/xing.png"
                picPressed: "/image/dailinterfaceView/dialpopmenu/dial_pan_d.png"
                onClicked: {
                    dialNumber += '*';
                    dialDTMFCode = '*';
                    bluetoothCallPage.inputDTMFCode(dialDTMFCode);
                }
            }

            OpacityBtn {
                id:pan0
                width:84
                height:63
                picNormal: "/image/dailinterfaceView/dialpopmenu/0.png"
                picPressed: "/image/dailinterfaceView/dialpopmenu/dial_pan_d.png"
                onClicked: {
                    dialNumber += '0';
                    dialDTMFCode = '0';
                    bluetoothCallPage.inputDTMFCode(dialDTMFCode);
                }
            }

            OpacityBtn {
                id:panj
                width:84
                height:63
                picNormal: "/image/dailinterfaceView/dialpopmenu/jing.png"
                picPressed: "/image/dailinterfaceView/dialpopmenu/dial_pan_d.png"
                onClicked: {
                    dialNumber += '#';
                    dialDTMFCode = '#';
                    bluetoothCallPage.inputDTMFCode(dialDTMFCode);
                }
            }
        }
    }
    }

    Connections {
        target: bluetoothCallPage
        onBluetoothCallPageChanged: {
            console.log(" [BluetoothCallPage] onBluetoothCallPageChanged");
            if (false == m_callPageState) {
                bluetoothCallPageView.destroy();
            }
        }
    }

    function showHoldButton(flag) {
        console.log(" [BluetoothCallPage] showHoldButton " + flag);
        if (flag) {
             swapLoader.width = 93;
             mergeLoader.width = 93;
             callmenu.width = 323 + 101*2
        } else {
             swapLoader.width = 0;
             mergeLoader.width = 0;
             callmenu.width = 265
        }
    }

    Connections {
        target: bluetoothCallPage
        onCallListChanged: {
            console.log("call list changed");
            var tempHoldCallState = false;
            var tempAcceptCallState = notNeedAccept;
            if (bluetoothCallListModel.rowCount() == 1) {
                phoneNumber = bluetoothCallListModel.get(0, call_number);
                phoneName = bluetoothCallListModel.get(0, call_name);
                tempAcceptCallState = (bluetoothCallListModel.hasIncomingCall()) ? needAccept : notNeedAccept;
            }

            if (bluetoothCallListModel.rowCount() > 1) {
                if (bluetoothCallListModel.hasIncomingCall()) {
                    tempAcceptCallState = needAccept;
                } else if (bluetoothCallListModel.hasWattingCall()) {
                    tempAcceptCallState = needAcceptAndHold;
                } else if (tempAcceptCallState != needAcceptAndHold && bluetoothCallListModel.hasHoldCall()) {
                    tempHoldCallState = true;
                }
            }

            showPauseButton = bluetoothCallListModel.isAllHoldStatus() ? true : false;
            holdCallState = tempHoldCallState;
            acceptCallState = tempAcceptCallState;
            if (!showKeyboard)
                showHoldButton(holdCallState);
        }
    }



    Connections {
        target: bluetoothCallPage
        onScoStateChanged : {
            console.log(" [BluetoothCallPage] onScoStateChanged ");
            audioSourceInHF = m_audioSourceInHF;
            console.log(" [BluetoothCallPage] audioSourceInHF = "+audioSourceInHF);
        }
    }

    Connections {
        target: bluetoothCallPage
        onTriggerAutoAnswerTimer : {
            console.log(" [BluetoothCallPage] onTriggerAutoAnswerTimer ");
            autoAnswerTimer.start();
        }
    }

}

