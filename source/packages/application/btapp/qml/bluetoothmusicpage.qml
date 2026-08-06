import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.1

Rectangle {
    id: bluetoothMusicPageView
    width: 585
    height: 402
    radius: 12
    border.color:"#00000000"
    color:"#00000000"

    property string title:  bluetoothMusicPage.getTitle();
    property string artist: bluetoothMusicPage.getArtist();
    property string album:  bluetoothMusicPage.getAlbum();
    property bool a2dpState:  bluetoothMusicPage.getA2dpState();
    property bool avrcpState: bluetoothMusicPage.getAvrcpState();
    property bool musicState: bluetoothMusicPage.getMusicState();
    property int currentTime: bluetoothMusicPage.getCurrentTime();
    property int totalTime: bluetoothMusicPage.getTotalTime();

    Component.onCompleted: {
        bluetoothMusicPage.checkMediaAudioConnectState();
        bluetoothMusicPage.updatePlayStatusRequest();
        checkId3InfoLength();
        totaltime.text = (add_zero(((totalTime/1000)/3600))+ ":" + add_zero(((totalTime/1000)%3600/60)) + ":" + add_zero(((totalTime/1000)%60))).toString()
        currenttime.text = (add_zero(((currentTime/1000)/3600))+ ":" + add_zero(((currentTime/1000)%3600/60)) + ":" + add_zero(((currentTime/1000)%60))).toString()
    }
/*
    NumberAnimation { target: bluetoothMusicPageView; property: "x";
        duration: 400; from: 732; to: 0;
        easing.type: Easing.OutBounce
        easing.amplitude: 0.1
        running: true }
*/
    Image {
        source:"/image/btmusic/bluetooth_rightbox.png"
    }

    BorderImage {
        id: btMusicview
        anchors.top: parent.top
        anchors.topMargin: 60
        anchors.left: parent.left
        anchors.leftMargin: 24
        source: a2dpState? "/image/btmusic/music-connect.png" : "/image/btmusic/muic_noneconnect.png"
        asynchronous: true
        cache: true
    }

    BorderImage {
        id: id3_music_view
        width:32
        height:32
        anchors.top: parent.top
        anchors.topMargin: 52
        anchors.left: btMusicview.right
        anchors.leftMargin: 24
        source: "/image/btmusic/ID3music.png"
        asynchronous: true
        cache: true
    }

    Rectangle {
        id: id3_music_rect
        width:160
        height: 28
        anchors.left: id3_music_view.right
        anchors.leftMargin: 16
        anchors.verticalCenter: id3_music_view.verticalCenter
        color: "#00000000"
        clip: true

        Text {
            id: id3_music_text
            width:160
            text: title
            color: "white"
            font.pixelSize: 20

            NumberAnimation {
                id: id3_music_animation
                target: id3_music_text
                property: "x"
                duration: 10000
                from: 0
                to: id3_music_text.text.length - 1.8*id3_music_rect.width
                running : false
                loops: Animation.Infinite
            }
        }
    }

    BorderImage {
        id: id3_singer_view
        width:32
        height:32
        anchors.top: id3_music_view.bottom
        anchors.topMargin: 20
        anchors.left: btMusicview.right
        anchors.leftMargin: 24
        source: "/image/btmusic/ID3singer.png"
        asynchronous: true
        cache: true
    }

    Rectangle {
        id: id3_singer_rect
        width: 160
        height: 28
        anchors.left: id3_singer_view.right
        anchors.leftMargin: 16
        anchors.verticalCenter: id3_singer_view.verticalCenter
        color: "#00000000"
        clip: true

        Text {
            id: id3_singer_text
            width:160
            text: artist
            color: "white"
            font.pixelSize: 20

            NumberAnimation {
                id: id3_singer_animation
                target: id3_singer_text
                property: "x"
                duration: 10000
                from: 0
                to: id3_singer_text.text.length - 1.8*id3_singer_rect.width
                running : false
                loops: Animation.Infinite
            }
        }
    }

    BorderImage {
        id: id3_album_view
        width:32
        height:32
        anchors.top: id3_singer_view.bottom
        anchors.topMargin: 20
        anchors.left: btMusicview.right
        anchors.leftMargin: 24
        source: "/image/btmusic/ID3album.png"
        asynchronous: true
        cache: true
    }

    Rectangle {
        id: id3_album_rect
        width:160
        height: 28
        anchors.left: id3_album_view.right
        anchors.leftMargin: 16
        anchors.verticalCenter: id3_album_view.verticalCenter
        color: "#00000000"
        clip: true

        Text {
            id: id3_album_text
            width:160
            text: album
            color: "white"
            font.pixelSize: 20

            NumberAnimation {
                id: id3_album_animation
                target: id3_album_text
                property: "x"
                duration: 10000
                from: 0
                to: id3_album_text.text.length - 1.8*id3_album_rect.width
                running : false
                loops: Animation.Infinite
            }
        }
    }

    Text {
        id: a2dp_msg
        anchors.top: id3_album_view.bottom
        anchors.topMargin: 20
        anchors.left: btMusicview.right
        anchors.leftMargin: 24
        font.pixelSize: 20
        color: "white"
        text: a2dpState? qsTr("A2DP Connected") : qsTr("A2DP No Connect")
    }

    Text {
        id: avrcp_msg
        anchors.top: a2dp_msg.bottom
        anchors.topMargin: 20
        anchors.left: btMusicview.right
        anchors.leftMargin: 24
        font.pixelSize: 20
        color: "white"
        text: avrcpState? qsTr("AVRCP Connected") : qsTr("AVRCP No Connect")
    }

    Row {
        id: sysbtngroup
        spacing: 16
        anchors.horizontalCenter: parent.horizontalCenter
        y:340

        OpacityBtn {
            id:pre
            width:80
            height:37
            picNormal: "/image/btmusic/pre_btn.png"
            picPressed: "/image/btmusic/btn_d.png"
            onClicked: {
                console.log("[BluetoothMusicPage] pre_btn clicked")
                bluetoothMusicPage.musicPreviousRequest();
            }
        }

        OpacityBtn {
            id:playpause
            width:80
            height:37
            picNormal: musicState ? "/image/btmusic/pause.png" : "/image/btmusic/play.png"
            picPressed: "/image/btmusic/btn_d.png"
            onClicked: {
                console.log("[BluetoothMusicPage] playpause_btn clicked")
                bluetoothMusicPage.musicPausePlayRequest();
            }
        }

        OpacityBtn {
            id:next
            width:80
            height:37
            picNormal: "/image/btmusic/next_btn.png"
            picPressed: "/image/btmusic/btn_d.png"
            onClicked: {
                console.log("[BluetoothMusicPage] next_btn clicked")
                bluetoothMusicPage.musicNextRequest();
            }
        }
    }

    Text {
        id:currenttime
        anchors.top: musicprogress.bottom
        anchors.left: parent.left
        anchors.leftMargin: 16
        color: "white"
        font.pixelSize: 16
        text: ""
    }

    Text {
        id:totaltime
        anchors.top: musicprogress.bottom
        anchors.right: parent.right
        anchors.rightMargin: 16
        color: "white"
        font.pixelSize: 16
        text: ""
    }

    Slider {
        id: musicprogress
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 56
        anchors.horizontalCenter: parent.horizontalCenter
        style: touchStyle
        value: currentTime
        maximumValue: totalTime
    }

    Component {
        id: touchStyle
        SliderStyle {
            handle: Rectangle {
                id:next
                width:16
                height:16
                radius: height
                antialiasing: true
                Image{
                    source: "/image/btmusic/progress_point.png"
                    anchors.centerIn: parent
                }
            }

            groove: Item {
                implicitHeight: 28
                implicitWidth: 548
                Rectangle {
                    height: 4
                    width: parent.width
                    anchors.verticalCenter: parent.verticalCenter
                    color: "#BFBFBF"
                    opacity: 0.8
                    Rectangle {
                        antialiasing: true
                        radius: 1
                        color: "#468bb7"
                        height: parent.height
                        width: parent.width * control.value / control.maximumValue
                    }
                }
            }
        }
    }

    Connections {
        target: bluetoothMusicPage
        onId3InfoChanged: {
            console.log("[BluetoothMusicPage] onId3InfoChanged")
            id3_music_animation.stop();
            id3_music_animation.resume();
            id3_singer_animation.stop();
            id3_singer_animation.resume();
            id3_album_animation.stop();
            id3_album_animation.resume();
            title = m_title;
            artist = m_artist;
            album = m_album;
            checkId3InfoLength();
        }
    }

    function checkId3InfoLength() {
        console.log("[BluetoothMusicPage] checkId3InfoLength")
        console.log("[BluetoothMusicPage] id3_music_text.contentWidth = "+id3_music_text.contentWidth)
        if (id3_music_text.contentWidth > id3_music_text.width) {
            id3_music_animation.restart();
        } else {
            id3_music_animation.restart();
            id3_music_animation.stop();
        }
        console.log("[BluetoothMusicPage] id3_singer_text.contentWidth = "+id3_singer_text.contentWidth)
        if (id3_singer_text.contentWidth > id3_singer_text.width) {
            id3_singer_animation.restart();
        } else {
            id3_singer_animation.restart();
            id3_singer_animation.stop();
        }
        console.log("[BluetoothMusicPage] id3_album_text.contentWidth = "+id3_album_text.contentWidth)
        if (id3_album_text.contentWidth > id3_album_text.width) {
            id3_album_animation.restart();
        } else {
            id3_album_animation.restart();
            id3_album_animation.stop();
        }
    }

    function add_zero(temp){
        if (temp - 0.5 >= 0.0000001) {
            temp -= 0.5;
        }
        temp = temp.toFixed(0);
         if(temp<10)
             return "0"+temp;
         else
             return temp;
    }

    Connections {
        target: bluetoothMusicPage
        onTotalTimeChanged: {
            console.log("[BluetoothMusicPage] onTotalTimeChanged")
            totalTime = m_totalTime
            console.log("[BluetoothMusicPage] totalTime = "+totalTime);
            var duration = add_zero(((m_totalTime/1000)/3600))+ ":" + add_zero(((m_totalTime/1000)%3600/60)) + ":" + add_zero(((m_totalTime/1000)%60));
            console.log("[BluetoothMusicPage] duration = "+duration.toString());
            totaltime.text = duration.toString()
        }
    }

    Connections {
        target: bluetoothMusicPage
        onCurrentTimeChanged: {
            console.log("[BluetoothMusicPage] onCurrentTimeChanged")
            currentTime = m_currentTime
            console.log("[BluetoothMusicPage] currentTime = "+currentTime);
            var current = add_zero(((m_currentTime/1000)/3600))+ ":" + add_zero(((m_currentTime/1000)%3600/60)) + ":" + add_zero(((m_currentTime/1000)%60));
            console.log("[BluetoothMusicPage] current = "+current.toString());
            currenttime.text = current.toString()
        }
    }

    Connections {
        target: bluetoothMusicPage
        onA2dpStateChanged: {
            console.log("[BluetoothMusicPage] onA2dpStateChanged, m_a2dpState = "+m_a2dpState)
            a2dpState = m_a2dpState
        }
    }

    Connections {
        target: bluetoothMusicPage
        onAvrcpStateChanged: {
            console.log("[BluetoothMusicPage] onAvrcpStateChanged, m_avrcpState = "+m_avrcpState)
            avrcpState = m_avrcpState
        }
    }

    Connections {
        target: bluetoothMusicPage
        onMusicStateChanged: {
            console.log("[BluetoothMusicPage] onMusicStateChanged, m_musicState = "+m_musicState);
            musicState = m_musicState
        }
    }

    Connections {
        target: bluetoothApplication
        onLanguageChanged: {
            console.log("[BluetoothMusicPage] onLanguageChanged");
            a2dp_msg.text = a2dpState? qsTr("A2DP Connected") : qsTr("A2DP No Connect");
            avrcp_msg.text = avrcpState? qsTr("AVRCP Connected") : qsTr("AVRCP No Connect");
            bluetoothMusicPage.updatePlayStatusRequest();
        }
    }
}

