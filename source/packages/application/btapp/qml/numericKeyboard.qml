import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

Rectangle {
    id:numerickeyboard
    width: 800
    height: 217
    anchors.top: parent.top
    anchors.topMargin: 262
    anchors.right:parent.right
    color: "#00000000"
    visible:true

    property string click_number: ""
    signal setClickNumber(string str)
    signal sigNumericKeyboardClose()

    Image {
        width: 800
        height: 217
        source:"/image/numeric_keyboard/keyboard_bottom.png"
        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("[numerickeyboard] numerickeyboard's outside")
            }
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

    Rectangle{
        id: tipbox
        anchors.centerIn: parent
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

    Rectangle{
        anchors.fill: parent
        color: "#00000000"
        visible:true

        Rectangle {
            id:key_1
            width:177
            height:48
            anchors.top: parent.top
            anchors.topMargin: 6
            anchors.left:parent.left
            anchors.leftMargin: 72
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/1.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += '1'
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_2
            width:177
            height:48
            anchors.top: parent.top
            anchors.topMargin: 6
            anchors.left:key_1.right
            anchors.leftMargin: 6
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/2.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += '2'
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_3
            width:177
            height:48
            anchors.top: parent.top
            anchors.topMargin: 6
            anchors.left:key_2.right
            anchors.leftMargin: 6
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/3.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += '3'
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_minus
            width:124
            height:48
            anchors.top: parent.top
            anchors.topMargin: 6
            anchors.left:key_3.right
            anchors.leftMargin: 6
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/-.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += '-'
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_4
            width:177
            height:48
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:parent.left
            anchors.leftMargin: 72
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/4.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += '4'
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_5
            width:177
            height:48
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_4.right
            anchors.leftMargin: 6
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/5.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += '5'
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_6
            width:177
            height:48
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_5.right
            anchors.leftMargin: 6
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/6.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += '6'
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_comma
            width:124
            height:48
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_6.right
            anchors.leftMargin: 6
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/comma.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += ','
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_7
            width:177
            height:48
            anchors.top: parent.top
            anchors.topMargin: 115
            anchors.left:parent.left
            anchors.leftMargin: 72
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/7.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += '7'
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_8
            width:177
            height:48
            anchors.top: parent.top
            anchors.topMargin: 115
            anchors.left:key_7.right
            anchors.leftMargin: 6
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/8.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += '8'
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_9
            width:177
            height:48
            anchors.top: parent.top
            anchors.topMargin: 115
            anchors.left:key_8.right
            anchors.leftMargin: 6
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/9.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += '9'
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_delete
            width:124
            height:48
            anchors.top: parent.top
            anchors.topMargin: 115
            anchors.left:key_9.right
            anchors.leftMargin: 6
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/delete.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number != "") {
                        click_number = click_number.substring(0,click_number.length - 1)
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    }
                }
                onPressAndHold :{
                    click_number = "";
                    console.log("click_number = "+click_number)
                    numerickeyboard.setClickNumber(click_number)
                }
            }
        }

        Rectangle {
            id:key_black
            width:177
            height:48
            anchors.top: parent.top
            anchors.topMargin: 169
            anchors.left:parent.left
            anchors.leftMargin: 72
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/English.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += ' '
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_0
            width:177
            height:48
            anchors.top: parent.top
            anchors.topMargin: 169
            anchors.left:key_black.right
            anchors.leftMargin: 6
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/0.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += '0'
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_end
            width:177
            height:48
            anchors.top: parent.top
            anchors.topMargin: 169
            anchors.left:key_0.right
            anchors.leftMargin: 6
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/end.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_number.length < 16) {
                        click_number += '.'
                        console.log("click_number = "+click_number)
                        numerickeyboard.setClickNumber(click_number)
                    } else {
                        notify_numberlen();
                    }
                }
            }
        }

        Rectangle {
            id:key_done
            width:124
            height:48
            anchors.top: parent.top
            anchors.topMargin: 169
            anchors.left:key_end.right
            anchors.leftMargin: 6
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/numeric_keyboard/Done.png"
                asynchronous:true
                cache:true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    numerickeyboard.visible = false
                    console.log("[numerickeyboard] sigNumericKeyboardClose before")
                    numerickeyboard.sigNumericKeyboardClose()
                    console.log("[numerickeyboard] sigNumericKeyboardClose after")
                    numerickeyboard.destroy()
                    console.log("[numerickeyboard] destroy after")
                }
            }
        }
    }

    function notify_numberlen() {
        tips.text = qsTr("Number length > 16");
        tipbox.visible = true;
        hidetipbox.start();
    }

    Connections {
        target: bluetoothApplication
        onPhoneConnectStateChanged: {
            console.log("[keyboard] onPhoneConnectStateChanged, m_phoneConnectState = "+m_phoneConnectState);
            if (false == m_phoneConnectState) {
                console.log("numerickeyboard.destroy()")
                numerickeyboard.visible = false;
                numerickeyboard.destroy();
            }
        }
    }

    Connections {
        target: bluetoothApplication
        onMediaConnectStateChanged: {
            console.log("[keyboard] onMediaConnectStateChanged, m_mediaConnectState = "+m_mediaConnectState);
            if (false == m_mediaConnectState) {
                console.log("numerickeyboard.destroy()")
                numerickeyboard.visible = false;
                numerickeyboard.destroy();
            }
        }
    }
}
