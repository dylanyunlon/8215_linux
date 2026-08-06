import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

Rectangle {
    id:keyboard
    width: 800
    height: 217
    anchors.top: parent.top
    anchors.topMargin: 262
    anchors.right:parent.right
    color: "#00000000"
    visible:true

    property string click_string: ""
    property int max_bluetoothName_len: 249 // The bluetooth device name cannot exceed 248 characters.
    signal sigClickString(string str)
    signal sigKeyboardClose()

    Image {
        width: 800
        height: 217
        source:"/image/keyboard/lowercase/keyboard_bottom.png"
        asynchronous: true
        cache: true
        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("[keyboard] keyboard's outside")
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
        id:lowercasekeyboard
        anchors.fill: parent
        color: "#00000000"
        visible:true

        Rectangle {
            id:key_q
            width:74
            height:44
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:parent.left
            anchors.leftMargin: 8
            color: "#00000000"
            radius: 50

            Image {
                source:"/image/keyboard/lowercase/q.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'q'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_w
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_q.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/lowercase/w.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'w'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_e
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_w.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/lowercase/e.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'e'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_r
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_e.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/lowercase/r.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'r'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_t
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_r.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/lowercase/t.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 't'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_y
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_t.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/lowercase/y.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'y'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_u
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_y.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/lowercase/u.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'u'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_i
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_u.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/lowercase/i.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'i'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_o
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_i.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/lowercase/o.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'o'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_p
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_o.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/lowercase/p.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'p'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_a
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:parent.left
            anchors.leftMargin: 48
            radius: 5

            Image {
                source:"/image/keyboard/lowercase/a.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'a'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_s
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_a.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/lowercase/s.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 's'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_d
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_s.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/lowercase/d.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'd'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_f
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_d.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/lowercase/f.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'f'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_g
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_f.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/lowercase/g.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'g'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_h
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_g.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/lowercase/h.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'h'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_j
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_h.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/lowercase/j.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'j'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_k
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_j.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/lowercase/k.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'k'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_l
            width:108
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_k.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/lowercase/l.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'l'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_lowercase
            width:108
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:parent.left
            anchors.leftMargin: 8
            radius:5

            Image {
                source:"/image/keyboard/lowercase/lowercase.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    lowercasekeyboard.visible = false;
                    uppercasekeyboard.visible = true;
                    characterkeyboard.visible = false;
                    characterkeyboard2.visible = false;
                }
            }
        }

        Rectangle {
            id:key_z
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_lowercase.right
            anchors.leftMargin: 13
            radius:5

            Image {
                source:"/image/keyboard/lowercase/z.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'z'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_x
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_z.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/lowercase/x.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'x'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_c
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_x.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/lowercase/c.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'c'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_v
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_c.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/lowercase/v.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'v'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_b
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_v.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/lowercase/b.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'b'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_n
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_b.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/lowercase/n.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'n'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_m
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_n.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/lowercase/m.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'm'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_Delete
            width:108
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_m.right
            anchors.leftMargin: 13
            radius:5

            Image {
                source:"/image/keyboard/lowercase/Delete.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string != "") {
                        click_string = click_string.substring(0,click_string.length - 1)
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    }
                }
                onPressAndHold :{
                /*
                    click_string = "";
                    console.log("click_string = "+click_string)
                    keyboard.sigClickString(click_string)
                */
                    if (click_string != "") {
                        click_string = click_string.substring(0,click_string.length - 1)
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    }
                }
            }
        }

        Rectangle {
            id:key_number
            width:114
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:parent.left
            anchors.leftMargin: 8
            radius:5

            Image {
                source:"/image/keyboard/lowercase/number.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    lowercasekeyboard.visible = false;
                    uppercasekeyboard.visible = false;
                    characterkeyboard.visible = true;
                    characterkeyboard2.visible = false;
                }
            }
        }

        Rectangle {
            id:key_comma
            width:73
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_number.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/lowercase/comma.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += ','
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_blank
            width:401
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_comma.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/lowercase/blank.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += ' '
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_end
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_blank.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/lowercase/end.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '.'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_Done
            width:115
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_end.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/lowercase/Done.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    lowercasekeyboard.visible = false;
                    uppercasekeyboard.visible = false;
                    characterkeyboard.visible = false;
                    characterkeyboard2.visible = false;
                    keyboard.visible = false;
                    console.log("[keyboard:key_Done] sigKeyboardClose before");
                    keyboard.sigKeyboardClose();
                    console.log("[keyboard:key_Done] sigKeyboardClose after");
                    keyboard.destroy();
                    console.log("[keyboard:key_Done] destroy after");
                }
            }
        }
    }

    Rectangle{
        id:uppercasekeyboard
        anchors.fill: parent
        color: "#00000000"
        visible:false

        Rectangle {
            id:key_Q
            width:74
            height:44
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:parent.left
            anchors.leftMargin: 8
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/keyboard/uppercase/Q.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'Q'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_W
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_Q.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/uppercase/W.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'W'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_E
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_W.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/uppercase/E.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'E'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_R
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_E.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/uppercase/R.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'R'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_T
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_R.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/uppercase/T.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'T'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_Y
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_T.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/uppercase/Y.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'Y'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_U
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_Y.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/uppercase/U.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'U'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_I
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_U.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/uppercase/I.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'I'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_O
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_I.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/uppercase/O.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'O'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_P
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_O.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/uppercase/P.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'P'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_A
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:parent.left
            anchors.leftMargin: 48
            radius: 5

            Image {
                source:"/image/keyboard/uppercase/A.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'A'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_S
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_A.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/uppercase/S.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'S'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_D
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_S.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/uppercase/D.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'D'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_F
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_D.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/uppercase/F.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'F'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_G
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_F.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/uppercase/G.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'G'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_H
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_G.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/uppercase/H.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'H'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_J
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_H.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/uppercase/J.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'J'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_K
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_J.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/uppercase/K.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'K'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_L
            width:108
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_K.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/uppercase/L.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'L'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_uppercase
            width:108
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:parent.left
            anchors.leftMargin: 8
            radius:5

            Image {
                source:"/image/keyboard/uppercase/uppercase.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    uppercasekeyboard.visible = false;
                    lowercasekeyboard.visible = true;
                    characterkeyboard.visible = false;
                    characterkeyboard2.visible = false;
                }
            }
        }

        Rectangle {
            id:key_Z
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_uppercase.right
            anchors.leftMargin: 13
            radius:5

            Image {
                source:"/image/keyboard/uppercase/Z.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'Z'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_X
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_Z.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/uppercase/X.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'X'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_C
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_X.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/uppercase/C.png"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'C'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_V
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_C.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/uppercase/V.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'V'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_B
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_V.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/uppercase/B.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'B'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_N
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_B.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/uppercase/N.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'N'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_M
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_N.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/uppercase/M.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += 'M'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_DELETE
            width:108
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_M.right
            anchors.leftMargin: 13
            radius:5

            Image {
                source:"/image/keyboard/uppercase/Delete.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string != "") {
                        click_string = click_string.substring(0,click_string.length - 1)
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    }
                }
                onPressAndHold :{
                /*
                    click_string = "";
                    console.log("click_string = "+click_string)
                    keyboard.sigClickString(click_string)
                */
                    if (click_string != "") {
                        click_string = click_string.substring(0,click_string.length - 1)
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    }
                }
            }
        }

        Rectangle {
            id:key_Number
            width:114
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:parent.left
            anchors.leftMargin: 8
            radius:5

            Image {
                source:"/image/keyboard/uppercase/number.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    lowercasekeyboard.visible = false;
                    uppercasekeyboard.visible = false;
                    characterkeyboard.visible = true;
                    characterkeyboard2.visible = false;
                }
            }
        }

        Rectangle {
            id:key_Comma
            width:73
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_Number.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/uppercase/comma.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += ','
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_Blank
            width:401
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_Comma.right
            anchors.leftMargin: 8
            radius:5

            Image {
                source:"/image/keyboard/uppercase/blank.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += ' '
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_End
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_Blank.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/uppercase/end.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '.'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_DONE
            width:115
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_End.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/uppercase/Done.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    lowercasekeyboard.visible = false;
                    uppercasekeyboard.visible = false;
                    characterkeyboard.visible = false;
                    characterkeyboard2.visible = false;
                    keyboard.visible = false;
                    console.log("[keyboard:key_DONE] sigKeyboardClose before");
                    keyboard.sigKeyboardClose();
                    keyboard.destroy();
                }
            }
        }
    }

    Rectangle{
        id:characterkeyboard
        anchors.fill: parent
        color: "#00000000"
        visible:false

        Rectangle {
            id:key_1
            width:74
            height:44
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:parent.left
            anchors.leftMargin: 8
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/keyboard/character/1.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '1'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_2
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_1.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/2.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '2'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_3
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_2.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/3.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '3'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_4
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_3.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/4.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '4'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_5
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_4.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/5.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '5'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_6
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_5.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/6.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '6'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_7
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_6.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/7.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '7'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_8
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_7.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/8.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '8'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_9
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_8.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/9.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '9'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_0
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_9.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/0.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '0'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_at
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:parent.left
            anchors.leftMargin: 48
            radius: 5

            Image {
                source:"/image/keyboard/character/@.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                if (click_string.length <= max_bluetoothName_len) {
                    click_string += '@'
                    console.log("click_string = "+click_string)
                    keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_hashtag
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_at.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/hashtag.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '#'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_dollarsign
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_hashtag.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/dollarsign.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '$'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_percentsign
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_dollarsign.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/%.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '%'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_caret
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_percentsign.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/caret.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '^'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_andsign
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_caret.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/andsign.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '&'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_startsign
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_andsign.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/asterisk.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '*'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_minussign
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_startsign.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/-.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '-'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_plussign
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 60
            anchors.left:key_minussign.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/+.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '+'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_mathematical_symbols
            width:108
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:parent.left
            anchors.leftMargin: 8
            radius:5

            Image {
                source:"/image/keyboard/character/mathematical_symbols.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    lowercasekeyboard.visible = false;
                    uppercasekeyboard.visible = false;
                    characterkeyboard.visible = false;
                    characterkeyboard2.visible = true;
                }
            }
        }

        Rectangle {
            id:key_exclamationmark
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_mathematical_symbols.right
            anchors.leftMargin: 13
            radius:5

            Image {
                source:"/image/keyboard/character/!.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '!'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_double_quotation_mark
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_exclamationmark.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/double_quotation_mark.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '"'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_single_quotation_mark
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_double_quotation_mark.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/single_quotation_mark.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += "'"
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_colon
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_single_quotation_mark.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/colon.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += ':'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_semicolon
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_colon.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/semicolon.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += ';'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_separator
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_semicolon.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/separator.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '/'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_question_mark
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_separator.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/question_mark.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '?'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_DElete
            width:108
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_question_mark.right
            anchors.leftMargin: 13
            radius:5

            Image {
                source:"/image/keyboard/character/Delete.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string != "") {
                        click_string = click_string.substring(0,click_string.length - 1)
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    }
                }
                onPressAndHold :{
                /*
                    click_string = "";
                    console.log("click_string = "+click_string)
                    keyboard.sigClickString(click_string)
                */
                    if (click_string != "") {
                        click_string = click_string.substring(0,click_string.length - 1)
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    }
                }
            }
        }

        Rectangle {
            id:key_character
            width:114
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:parent.left
            anchors.leftMargin: 8
            radius:5

            Image {
                source:"/image/keyboard/character/character.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    lowercasekeyboard.visible = true;
                    uppercasekeyboard.visible = false;
                    characterkeyboard.visible = false;
                    characterkeyboard2.visible = false;
                }
            }
        }

        Rectangle {
            id:key_COmma
            width:92
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_character.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/comma.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += ','
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_BLank
            width:401
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_COmma.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/blank.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += ' '
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_ENd
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_BLank.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/end.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '.'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_DOne
            width:92
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_ENd.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/Done.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    lowercasekeyboard.visible = false;
                    uppercasekeyboard.visible = false;
                    characterkeyboard.visible = false;
                    characterkeyboard2.visible = false;
                    keyboard.visible = false;
                    console.log("[keyboard:key_DOne] sigKeyboardClose before");
                    keyboard.sigKeyboardClose();
                    keyboard.destroy();
                }
            }
        }
    }

    Rectangle{
        id:characterkeyboard2
        anchors.fill: parent
        color: "#00000000"
        visible:false

        Rectangle {
            id:key_12
            width:74
            height:44
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:parent.left
            anchors.leftMargin: 8
            color: "#00000000"
            radius: 10

            Image {
                source:"/image/keyboard/character/1.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '1'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_22
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_12.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/2.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '2'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_32
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_22.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/3.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '3'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_42
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_32.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/4.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '4'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_52
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_42.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/5.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '5'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_62
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_52.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/6.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '6'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_72
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_62.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/7.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '7'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_82
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_72.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/8.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '8'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_92
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_82.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/9.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '9'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_02
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left:key_92.right
            anchors.leftMargin: 6
            radius: 10

            Image {
                source:"/image/keyboard/character/0.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '0'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_tab
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 48
            anchors.left:parent.left
            anchors.leftMargin: 48
            radius: 5

            Image {
                source:"/image/keyboard/character/Tab.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += "    "
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_ellipsis
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 48
            anchors.left:key_tab.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/ellipsis.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += "..."
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_tilde
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 48
            anchors.left:key_ellipsis.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/~.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '~'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_point
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 48
            anchors.left:key_tilde.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/point.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += "`"
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_verticalbar
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 48
            anchors.left:key_point.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/verticalbar.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '|'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_underline
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 48
            anchors.left:key_verticalbar.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/underline.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '_'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_equalsign
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 48
            anchors.left:key_underline.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/=.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '='
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_curlybrace_left
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 48
            anchors.left:key_equalsign.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/curlybrace_left.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += "{"
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_curlybrace_right
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 48
            anchors.left:key_curlybrace_left.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/curlybrace_right.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += "}"
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_mathematical_symbols2
            width:108
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:parent.left
            anchors.leftMargin: 8
            radius:5

            Image {
                source:"/image/keyboard/character/mathematical_symbols.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    lowercasekeyboard.visible = false;
                    uppercasekeyboard.visible = false;
                    characterkeyboard.visible = true;
                    characterkeyboard2.visible = false;
                }
            }
        }

        Rectangle {
            id:key_leftparenthesis
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_mathematical_symbols2.right
            anchors.leftMargin: 13
            radius: 5

            Image {
                source:"/image/keyboard/character/(.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '('
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_rightparenthesis
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_leftparenthesis.right
            anchors.leftMargin: 6
            radius: 5

            Image {
                source:"/image/keyboard/character/).png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += ')'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_slash
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_rightparenthesis.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/slash.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += "\\"
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_lessthan
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_slash.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/lessthan.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += "<"
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_morethan
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_lessthan.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/morethan.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += ">"
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_bracket_left
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_morethan.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/bracket_left.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '['
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_bracket_right
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_bracket_left.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/bracket_right.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += ']'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_DELete
            width:108
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 112
            anchors.left:key_bracket_right.right
            anchors.leftMargin: 13
            radius:5

            Image {
                source:"/image/keyboard/character/Delete.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string != "") {
                        click_string = click_string.substring(0,click_string.length - 1)
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    }
                }
                onPressAndHold :{
                /*
                    click_string = "";
                    console.log("click_string = "+click_string)
                    keyboard.sigClickString(click_string)
                */
                    if (click_string != "") {
                        click_string = click_string.substring(0,click_string.length - 1)
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    }
                }
            }
        }

        Rectangle {
            id:key_character2
            width:114
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:parent.left
            anchors.leftMargin: 8
            radius:5

            Image {
                source:"/image/keyboard/character/character.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    lowercasekeyboard.visible = true;
                    uppercasekeyboard.visible = false;
                    characterkeyboard.visible = false;
                    characterkeyboard2.visible = false;
                }
            }
        }

        Rectangle {
            id:key_COMma
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_character2.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/comma.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += ','
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_BLAnk
            width:401
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_COMma.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/blank.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += ' '
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_END2
            width:74
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_BLAnk.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/end.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (click_string.length <= max_bluetoothName_len) {
                        click_string += '.'
                        console.log("click_string = "+click_string)
                        keyboard.sigClickString(click_string)
                    } else {
                        notify_bluetoothName_len();
                    }
                }
            }
        }

        Rectangle {
            id:key_DONe
            width:115
            height:44
            color: "#00000000"
            anchors.top: parent.top
            anchors.topMargin: 164
            anchors.left:key_END2.right
            anchors.leftMargin: 6
            radius:5

            Image {
                source:"/image/keyboard/character/Done.png"
                asynchronous: true
                cache: true
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    lowercasekeyboard.visible = false;
                    uppercasekeyboard.visible = false;
                    characterkeyboard.visible = false;
                    characterkeyboard2.visible = false;
                    keyboard.visible = false;
                    console.log("[keyboard:key_DONe] sigKeyboardClose before");
                    keyboard.sigKeyboardClose();
                    keyboard.destroy();
                }
            }
        }

    }

    function notify_bluetoothName_len() {
        tips.text = qsTr("Number length > 248")
        tipbox.visible = true
        hidetipbox.start()
    }

    Connections {
        target: bluetoothApplication
        onPhoneConnectStateChanged: {
            console.log("[keyboard] onPhoneConnectStateChanged, m_phoneConnectState = "+m_phoneConnectState);
            if (false == m_phoneConnectState) {
                console.log("[keyboard] hide keyboard -> onPhoneConnectStateChanged");
                keyboard.visible = false;
                console.log("keyboard.destroy()")
                keyboard.destroy();
            }
        }
    }

    Connections {
        target: bluetoothApplication
        onMediaConnectStateChanged: {
            console.log("[keyboard] onMediaConnectStateChanged, m_mediaConnectState = "+m_mediaConnectState);
            if (false == m_mediaConnectState) {
                console.log("[keyboard] hide keyboard -> onMediaConnectStateChanged");
                keyboard.visible = false;
                console.log("keyboard.destroy()")
                keyboard.destroy();
            }
        }
    }

}