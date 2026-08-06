import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2

Item {
    id: topPage
    objectName: "topPageName"
    x:0
    y:0
    width: 1080
    height: 56
    property int device_id_sdcard: 1
    property int device_id_udisk:  2
    property int device_id_bt:     3
    property int device_id_wifi:   4
    property int device_id_ipod:   5
    
    Rectangle{
        anchors.fill: parent
        color:"black"
        opacity: 0.1
    }
    
    Connections {
        target: csubwndhome
        onSendMsgToQml: {
            console.log("TopPage: msg = "+ msg+ ", wParam = " + wParam + ", from topPage.qml");            
            switch (msg) {
                case device_id_sdcard:
                    setState("imageSdcardState", wParam);
                    break;
                case device_id_udisk:
                    setState("imageUdiskState", wParam);
                    break;
                case device_id_bt:
                    setState("imageBtState", wParam);
                    break;
                case device_id_wifi:
                    setState("imageWifiState", wParam);
                    break;
                case device_id_ipod:
                    setState("imageIpodState", wParam);
                    break;
                default:
                    break;
            }
        }
    }
    
    function setState(item, state)
    {
        console.log("Top page: item:",item,"state",state);
        if(item == "imageBtState")
        {
            if(state == "1"){
                console.log("bt:1");
                imageBtState.state = "BT_OPEN";
            }
            else{
                console.log("bt:0");
                imageBtState.state = "BT_CLOSE";
            }
        }
        else if(item == "imageSdcardState")
        {
            if(state == "1"){
                console.log("SD:1");
                imageSdcardState.state = "SDCARD_EXISIT"
            }
            else{
                console.log("SD:0");
                imageSdcardState.state = "SDCARD_NONE"
            }
        }
        else if(item == "imageUdiskState")
        {
            if(state == "1"){
                console.log("USB:1");
                imageUdiskState.state = "UDISK_EXISIT";
            }
            else{
                console.log("USB:0");
                imageUdiskState.state = "UDISK_NONE";
            }
        }
        else if(item == "imageWifiState")
        {
            if(state == "4"){
                console.log("wifi:4");
                imageWifiState.state = "WIFI_LEVEL_4";
            }
            else if(state == "3"){
                console.log("wifi:3");
                imageWifiState.state = "WIFI_LEVEL_3";
            }
            else if(state == "2"){
                console.log("wifi:2");
                imageWifiState.state = "WIFI_LEVEL_2";
            }
            else if(state == "1"){
                console.log("wifi:1");
                imageWifiState.state = "WIFI_LEVEL_1";
            }
            else{
                console.log("wifi:0");
                imageWifiState.state = "WIFI_LEVEL_0"
            }
        }else if(item == "imageIpodState")
        {
            if(state == "1"){
                console.log("IPOD:1");
                imageIpodState.state = "IPOD_EXISIT";
            }
            else{
                console.log("IPOD:0");
                imageIpodState.state = "IPOD_NONE";
            }
        }

    }
    
    Image {
        objectName: "imageIpodStateName"
        id: imageIpodState
        x:934
        width: 28
        height: 44
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        source: "ipodNone.png"
        states:[
            State{
                name:"IPOD_EXISIT"
                PropertyChanges {target: imageIpodState; source:"ipodExist.png"}
            },
            State{
                name:"IPOD_NONE"
                PropertyChanges {target: imageIpodState; source:"ipodNone.png"}
            }
        ]
    }

    Image {
        id: imageWifiState
        anchors.verticalCenter: parent.verticalCenter
        objectName: "imageWifiStateName"
        anchors.right: imageIpodState.left
        width: 38
        height: 40
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        source: "wifiLevel0.png"
        states:[
            State{
                name:"WIFI_LEVEL_0"
                PropertyChanges {target: imageWifiState; source:"wifiLevel0.png"}
            },
            State{
                name:"WIFI_LEVEL_1"
                PropertyChanges {target: imageWifiState; source:"wifiLevel1.png"}
            },
            State{
                name:"WIFI_LEVEL_2"
                PropertyChanges {target: imageWifiState; source:"wifiLevel2.png"}
            },
            State{
                name:"WIFI_LEVEL_3"
                PropertyChanges {target: imageWifiState; source:"wifiLevel3.png"}
            },
            State{
                name:"WIFI_LEVEL_4"
                PropertyChanges {target: imageWifiState; source:"wifiLevel4.png"}
            }
        ]
    }

    Image {
        id: imageUdiskState
        anchors.verticalCenter: parent.verticalCenter
        objectName: "imageUdiskStateName"
        anchors.right: imageWifiState.left
        width: 28
        height: 44
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        source: "udiskNone.png"

        states:[
            State{
                name:"UDISK_EXISIT"
                PropertyChanges {target: imageUdiskState; source:"udiskExist.png"}
            },
            State{
                name:"UDISK_NONE"
                PropertyChanges {target: imageUdiskState; source:"udiskNone.png"}
            }
        ]
    }

    Image {
        id: imageSdcardState
        anchors.verticalCenter: parent.verticalCenter
        objectName: "imageSdcardStateName"
        anchors.right: imageUdiskState.left
        width: 28
        height: 44
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        source: "sdcardNone.png"

        states:[
            State{
                name:"SDCARD_EXISIT"
                PropertyChanges {target: imageSdcardState; source:"sdcardExist.png"}
            },
            State{
                name:"SDCARD_NONE"
                PropertyChanges {target: imageSdcardState; source:"sdcardNone.png"}
            }
        ]
    }

    Image {
        id: imageBtState
        anchors.verticalCenter: parent.verticalCenter
        objectName: "imageBtStateName"
        anchors.right: imageSdcardState.left
        width: 28
        height: 44
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        source: "btClose.png"
        states:[
            State{
                name:"BT_OPEN"
                PropertyChanges {target: imageBtState; source:"btOpen.png"}
            },
            State{
                name:"BT_CLOSE"
                PropertyChanges {target: imageBtState; source:"btClose.png"}
            }
        ]
    }

    Text {
        id: textTopTime
        width: 80
        height: 40
        text: qsTr("Text")
        visible: false
        anchors.horizontalCenter: parent.horizontalCenter
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 20
    }
}
